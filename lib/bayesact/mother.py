"""------------------------------------------------------------------------------------------
Bayesian Affect Control Theory
Mother-Son Example from Gratch Paper
Author: Jesse Hoey  jhoey@cs.uwaterloo.ca   http://www.cs.uwaterloo.ca/~jhoey
September 2013
Use for research purposes only.
Please do not re-distribute without written permission from the author
Any commerical uses strictly forbidden.
Code is provided without any guarantees.
Research sponsored by the Natural Sciences and Engineering Council of Canada (NSERC).
use python2.6
see README for details
----------------------------------------------------------------------------------------------"""

from bayesact import *


class Mother(Agent):
    def __init__(self,*args,**kwargs):

        super(Mother, self).__init__(self,*args,**kwargs)
        #x for the Mother x is a tuple:
        #the first value is the turn (always this way for any Agent)
        #the second value
        #is the state of the son (alive+in pain, alive+not in pain, dead)
        self.xvals=["inpain","notinpain","dead"]
        #probability distribution over x
        self.px=kwargs.get("initx",[1.0,0.0,0.0])
        #distributions for action "soothe" (give morphine to) and "torment" (not give morphine to)
        self.ppx=[[[0.1,0.7,0.2],[0.05,0.75,0.2],[0.0,0.0,1.0]],
                  [[0.99,0.0,0.01],[0.85,0.14,0.01],[0.0,0.0,1.0]]]

        self.of=kwargs.get("of",[[0.98,0.01,0.01],#x[1]=inpain
                                 [0.01,0.98,0.01],#x[1]=notinpain
                                 [0.01,0.01,0.98]])#x[1]=dead


    def print_params(self):
        Agent.print_params(self)
        print "px: ",self.px
        print "ppx[0]: ",self.ppx[0]
        print "ppx[1]: ",self.ppx[1]

    #The only difference between a TutoringAgent and an Agent
    #is dynamics of x and reward
    def initialise_x(self,initx=None):
        if not initx:
            initpx=self.px
            initturn=State.turnnames.index("agent")
        else:
            initpx=initx[1]
            initturn=initx[0]
        initpl = list(NP.random.multinomial(1,initpx)).index(1)
        return [initturn,initpl]

    def get_prop_action(self,state,rollout=False,samplenum=0):
        if self.use_pomcp:
            if rollout:
                paab = NP.random.randint(self.numDiscActions) 
            else:
                paab = samplenum%self.numDiscActions
        else:
            paab = 0  #soothe
        return paab

    #was greedy_action_select
    #selects an action greedily by a
    def get_default_action(self,state,paab=None):
        return [[2.89, 1.56, -0.77],0]

    def sampleXObservation(self,state):
        if state.get_turn()=="agent":
            return [state.x[0],list(NP.random.multinomial(1,self.of[state.x[1]])).index(1)]
        else:
            return [state.x[0],0]


    #aab is the affective part of the action, paab is the propositional part
    def sampleXvar(self,f,tau,state,aab,paab):
        if state.get_turn()=="client":
            #never happens
            return [state.x[0],state.x[1]]
        else:
            #x transitions according to mother's action
            newx = list(NP.random.multinomial(1,self.ppx[paab][state.x[1]])).index(1)
            return [state.x[0],newx]  #turn never changes
    
    #this must be in the class
    def evalSampleXvar(self,sample,xobs):
        #this nasty little hack is because I'm calling update(..) to update the agent
        #in tutorgui2.py, which makes a default assumption that on agent turn, no observations
        #are received. 
        #if xobs==[] or not turn=="client":
        #    return 1.0
        #only when it was the client turn (so it is now agent turn) do we use the observation function
        if sample.get_turn()=="agent":
            if xobs[0]==sample.x[0]:
                #print sample.x[0],sample.x[1],xobs,self.of[sample.x[0]][sample.x[1]][xobs]
                return self.of[sample.x[1]][xobs[1]]
            else: 
                return 0.0
        else:
            return 1.0   #ignored x observation

    def reward(self,sample,action=None):
        # a generic deflection-based reward that can be removed eventually
        fsample=sample.f.transpose()
        freward = -1.0*NP.dot(sample.f-sample.tau,sample.f-sample.tau)
        # a state-based reward that favors states closer to the goal
        #xreward = -1.0*(sample.x[2]-self.goalx)**2
        # I wanted originally to only have the reward be based on deflection,
        # but the mother would never willingly choose to kill her son, so 
        # I'm faking it here by making the reward really negative if the son dies
        xreward = 0.0
        if sample.x==2:
            xreward=-(80.0*80.0)   #really bad

        return freward+xreward


    #----------------------------------------------------------------------------------
    #POMCP functions
    #----------------------------------------------------------------------------------

    def oracle(self,state,rollout=False,samplenum=None):
        #continuous component is just a 3D EPA vector which is the null value on client turn
        #the discrete component of the action is class dependent
        #here, if it is the client's turn, we simply use the null action for both 
        #and in any case, the propositional action is always the same (0 = null)
        (a,paab)  = self.get_null_action()

        if rollout:
            samplenum=NP.random.randint(2)

        if state.get_turn()=="agent":
            #switch between the two first sample returned is the default (ACT) action
            if samplenum%2==1:
                paab = 1            #not give morphine to
                a = [-3.2,1.4,0.94] #torment
                #kill : -4.05, 1.08, 1.35,
            else:
                paab = 0                 #give morphine to
                a = [2.89, 1.56, -0.77]  #soothe

        return (a,paab)

    #returns the closest observation match to obs in obsSet
    #this needs to be in this class, as the comparison depends on the structure of the observation vector
    def bestObservationMatch(self,obs,obsSet,obsSetData):
        firstone=True
        bestdist=-1
        besto=-1
        for oindex in range(len(obsSet)):
            o=obsSet[oindex]
            if obs[1][0]==o[1][0] and obs[1][1]==o[1][1]:
                odist = math.sqrt(raw_dist(obs[0],o[0]))
                if firstone or odist<bestdist:
                    firstone=False
                    besto=oindex
                    bestdist=odist
        return (besto,bestdist)
        
    def observationDist(self,obs1,obs2):
        if obs1[1][0] == obs2[1][0] and obs1[1][1] == obs2[1][1]:
            return math.sqrt(raw_dist(obs1[0],obs2[0]))
        else:
            return -2



#a function to ask the client for an action to take and map it to an EPA value
def ask_client(fbeh,sugg_act='',epaact=[]):
    sst="your action ('?' shows options): "

    sstnc="You can now either :\n"
    sstnc+="- pick an action (behaviour) and type in its label\n"
    sstnc+="- type 'quit' to stop the simulation\n"
    sstnc+="- type a comma separated list of three numbers to specify the action as E,P,A values\n"
    if not sugg_act=='':
        sstnc+="- hit 'enter' to take default action with label: "+sugg_act+"\n"
    if not epaact=='':
        sstnc+="- type any digit (0-9) to take suggsted action: "+str(epaact)

    
    while True:
        cact = raw_input(sst)
        if cact=='quit':
            return []
        elif cact=='' and not sugg_act=='':
            cact=sugg_act
            break
        elif cact.isdigit() and not epaact==[]:
            return epaact
        elif re.sub(r"\s+","_",cact.strip()) in fbeh.keys():
            cact=re.sub(r"\s+","_",cact.strip()) 
            break
        elif cact=="?":
            print sstnc
        elif is_array_of_floats(cact):
            return [float(x) for x in cact.split(',')]
        else:
            print "incorrect or not found, try again. '?' shows options"
    observ=map(lambda x: float (x), [fbeh[cact]["e"],fbeh[cact]["p"],fbeh[cact]["a"]])
    print "client said: ",cact
    return observ

num_samples=200
agent_gender="female"
client_gender="male"
roughening_noise=num_samples**(-1.0/3.0)
use_pomcp=True
initial_turn=State.turnnames.index("agent")
numcact=1
numdact=2
obsres=1.0
#action resolution when buildling pomcp plan tree
actres=0.1
timeout=100.0
discount_factor=0.1

learn_verbose=False

curr_painlevel=0  #inpain,notinpain,dead


ppx=[]
#distributions for action "soothe" (give morphine to)
ppx.append([[0.1,0.8,0.1],[0.0,0.9,0.1],[0.0,0.0,1.0]])
#distributions for action "torment" (not give morphine to)
ppx.append([[0.95,0.0,0.05],[0.85,0.1,0.05],[0.0,0.0,1.0]])

print ppx
done=False
agent_id="mother"
client_id="son"
agent_knowledge=2
#behaviours file
fbfname="fbehaviours.dat"

#identities file
fifname="fidentities.dat"

fbehaviours_agent=readSentiments(fbfname,agent_gender)
fbehaviours_client=readSentiments(fbfname,client_gender)


agent_id=getIdentity(fifname,agent_id,agent_gender)
if agent_id==[]:
    agent_id=NP.random.multivariate_normal(agent_mean_ids,agent_cov_ids)
agent_id=NP.asarray([agent_id]).transpose()

#here we get the identity of the client *as seen by the agent*
client_id=getIdentity(fifname,client_id,agent_gender)
if client_id==[]:
    client_id =  NP.random.multivariate_normal(client_mean_ids,client_cov_ids)
client_id=NP.asarray([client_id]).transpose()

#get initial sets of parameters for agent
(learn_tau_init,learn_prop_init,learn_beta_client_init,learn_beta_agent_init)=init_id(agent_knowledge,agent_id,client_id)

learn_initx=[initial_turn,[1.0,0.0,0.0]]



mother=Mother(N=num_samples,alpha_value=1.0,discount_factor=discount_factor,
              client_gender=client_gender,agent_gender=agent_gender,
              agent_rough=roughening_noise,client_rough=roughening_noise,use_pomcp=use_pomcp,
              init_turn=initial_turn,numcact=numcact,numdact=numdact,obsres=obsres,actres=actres,pomcp_timeout=timeout)

print learn_tau_init
print learn_prop_init
print learn_initx

learn_avgs=mother.initialise_array(learn_tau_init,learn_prop_init,learn_initx)


while not done:

    learn_avgs = mother.getAverageState()

    print "agent state is: "
    learn_avgs.print_val()

    #this always works here, but is only needed to avoid asking the user too many questions
    #and to figure out the observation 
    learn_turn=learn_avgs.get_turn()


    (learn_aab,learn_paab)=mother.get_next_action(learn_avgs,exploreTree=True)
    aact=findNearestBehaviour(learn_aab,fbehaviours_agent)
    print "suggested action for the agent is :",learn_aab,"\n  closest label is: ",aact

    learn_aab=ask_client(fbehaviours_agent,aact,learn_aab)
    print "agent does action :",learn_aab,"\n"
    learn_observ=[]

    print "old pain level: ",curr_painlevel
    curr_painlevel = int(raw_input("enter new pain level: 0,1,2 for inpain,notinpain,dead: "))
    print "new pain level: ",curr_painlevel


    #we may be done if the user has killed the interaction
    if learn_aab==[]:
        done = True

    else:

        #agent gets to observe the turn each time, and the pain level
        learn_xobserv=[State.turnnames.index(learn_turn),curr_painlevel]
        
        #the main SMC update step 
        learn_avgs=mother.propagate_forward(learn_aab,learn_observ,learn_xobserv,learn_paab,verb=learn_verbose)
    
        #I think these should be based on fundamentals, not transients
        (aid,cid)=mother.get_avg_ids(learn_avgs.f)
        print "agent thinks it is most likely a: ",aid
        print "agent thinks the client is most likely a: ",cid



