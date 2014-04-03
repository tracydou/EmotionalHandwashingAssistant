"""------------------------------------------------------------------------------------------
Bayesian Affect Control Theory
Interactive learning example (chat)
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
import getopt

#NP.set_printoptions(precision=5)
#NP.set_printoptions(suppress=True)
NP.set_printoptions(linewidth=10000)
 


#-----------------------------------------------------------------------------------------------------------------------------
#user-defined parameters 
#-----------------------------------------------------------------------------------------------------------------------------
#agent knowledge of client id: 
#0 : nothing
#1 : one of a selection of  num_confusers+1 randoms
#2 : exactly - use this to mimic interact
#3 : same as 0 but also agent does not know its own id
agent_knowledge=0

# agent gender
agent_gender="male"

# client gender
client_gender="male"

#possibly set the agent id to be something
agent_id="professor"
#if not in database (including "") then it is a randomly drawn id
#agent_id=""

#can also set the client id here if agent_knowledge = 2 (knows id of client - see above)
#if agent_knowledge is 0 then this is ignored
#client_id = "professor"
client_id = "student"

#who goes first? 
initial_turn="client"

#how often do we want to see the full id sets learned by the agent
get_full_id_rate=10


#do we want to try to mimic interact?
mimic_interact=False

#use pomcp for planning (default use heuristic/greedy method)
use_pomcp=False

#parameters for pomcp
#number of continuous actions we wish to sample - 
#this is user-defined and is an important parameter
#larger numbers mean bigger, slower, more accurate,  planning trees
numcact=5
#number of discrete (propositional) actions
#this should be set according to the domain, and is 1 for this generic class
#one discrete action really means no choice
numdact=1
#observation resolution when building pomcp plan tree
obsres=1.0
#action resolution when buildling pomcp plan tree
actres=0.1
#timeout used for POMCP
timeout=5.0

#-----------------------------------------------------------------------------------------------------------------------------
#these parameters can be tuned, but using caution
#-----------------------------------------------------------------------------------------------------------------------------
#agent's ability to change identities - higher means it will shape-shift more
bvagent=0.0001
#agent's belief about the client's ability to change identities - higher means it will shape-shift more
bvclient=0.0001


#-----------------------------------------------------------------------------------------------------------------------------
#Learning agent stuff
#-----------------------------------------------------------------------------------------------------------------------------
knownWords={}
wordCounts={}

#-----------------------------------------------------------------------------------------------------------------------------
#these parameters can be tuned, but will generally work "out of the box" for a basic simulation
#-----------------------------------------------------------------------------------------------------------------------------

#behaviours file
fbfname="fbehaviours.dat"

#identities file
fifname="fidentities.dat"

#get some key parameters from the command line
#set much larger to mimic interact (>5000)
num_samples=1000


# use 0.0 for mimic interact simulations
#roughening_noise=0.0
roughening_noise=num_samples**(-1.0/3.0)


#the observation noise
#set to 0.05 or less to mimic interact
obs_noise=0.1

if mimic_interact:
    bvagent_init=0.000001
    bvclient_init=0.000001
    bvagent=0.00001
    bvclient=0.00001
    agent_knowledge=2
    num_samples=10000
    roughening_noise=0.0
    obs_noise=0.01
    num_action_samples=10000

gamma_value=obs_noise


#do we print out all the samples each time
learn_verbose=False

#for repeatability
rseed = NP.random.randint(0,382948932)
print "random seeed is : ",rseed
#rseed=271887164

NP.random.seed(rseed)


helpstring="Bayesact interactive simulator (1 agents, 1 human) usage:\n bayesactinteractive.py\n\t -n <number of samples (default 500)>\n\t -a <agent knowledge (0,1,2) (Default 2)>\n\t -r <roughening noise: default n^(-1/3) - to use no roughening ('full' method), specify 0>\n\t -g <gamma_value (default 0.1)>\n\t -i <agent id label: default randomly chosen>\n\t -j <client id label: default randomly chosen>\n\t -k <agent gender (default: male) - only works if agent_id is specified with -i>\n\t -l (client gender (default: male) only works if client_id is specified with -j>"

try:
    opts, args = getopt.getopt(sys.argv[1:],"hun:t:x:a:c:d:r:e:g:i:j:k:l:",["help","n=","t=","x=","c=","a=","u=","d=","r=","e=","g=","i=","j=","k=","l="])
except getopt.GetoptError:
    print helpstring
    sys.exit(2)
for opt, arg in opts:
    if opt == '-h':
        print helpstring
        sys.exit()
    elif opt in ("-n", "--numsamples"):
        num_samples = int(arg)
    elif opt in ("-a", "--agentknowledge"):
        agent_knowledge = int(arg)
    elif opt in ("-r", "--roughen"):
        roughening_noise=float(arg)
    elif opt in ("-g", "--gamma"):
        gamma_value=float(arg)
    elif opt in ("-i", "--agentid"):
        agent_id=arg
    elif opt in ("-j", "--clientid"):
        client_id=arg
    elif opt in ("-k", "--agentgender"):
        agent_gender=arg
    elif opt in ("-l", "--clientgender"):
        client_gender=arg


#-----------------------------------------------------------------------------------------------------------------------------
#code start - here there be dragons - only hackers should proceed, with caution
#-----------------------------------------------------------------------------------------------------------------------------
def addWordToKnownWords(word,fbs,weight):
    if not word in knownWords:
        wordCounts[word] = 0
        knownWords[word] = []
    wordCounts[word] += 1
    knownWords[word].append((NP.array(fbs),weight))

def addChatToKnownWords(chat,fbs,weight):
    for word in chat.split():
        addWordToKnownWords(word,fbs,weight)

def getWeight(fb,sentence,h):
    sum_weight=0
    for word in sentence.split():
        sum_weight += math.log(getWeightWord(fb,word,h))
    return sum_weight

def getWeightWord(fb,word,h):
    #compute the kernel estimate of the density given the word
    weight=0.0
    for (fbi,wgt) in knownWords[word]:
        weight += wgt*math.exp(-1.0*raw_dist(fb,fbi)/h)
    return weight

def printKnownWords():
    for word in knownWords:
        print "word is :",word
        #print fsampleMean(knownWords[word])
        print knownWords[word]        

#the learning agent class - must override evalSampleFvar
class LearningAgent(Agent):
    def __init__(self,*args,**kwargs):
        
        super(LearningAgent, self).__init__(self,*args,**kwargs)
        self.h = kwargs.get("kernel_scale",1.0)
        
    def initialise_x(self,initx):
        if NP.random.random() > 0.5:
            return [State.turnnames.index(initx)]
        else:
            return [State.turnnames.index(invert_turn(initx))]

    #evaluates a sample of Fvar'=state.f
    #the observ now is a word which will need to be looked up
    
    def evalSampleFvar(self,fvars,tdyn,state,ivar,ldenom,turn,observ):
        weight=0.0
        if (not observ==[]): # and turn=="client":
            fb=getbvars(fvars,state.f)
            try:
                #super inefficient because it checks if each word in the chat is known every time this is called (for every sample)
                #this should have been already done
                #addChatToKnownWords(observ,fb)
                #assume this will not get done if the above line throws the Attribute Exception, which
                #means that observ is actually a list of floats  - this seems really awkward
                weight += getWeight(fb,observ,self.h)
            except AttributeError:  #super lame
                #standard method
                dvo=NP.array(fb)-NP.array(observ)
                weight += normpdf(dvo,0.0,ivar,ldenom)
                
        else:
            weight=0.0
        return weight

    #overload in subclass
    def sampleXvar(self,f,tau,state,aab,paab=None):
        if NP.random.random() > 0.2:
            return [state.invert_turn()]
        else:
            return state.x


    def getFbSamples(self,fvars):
        fb=[]
        for state in self.samples:
            fb.append(getbvars(fvars,state.f))
        return fb

    def drawFbSample(self,fvars):
        newsample=drawSamples(self.samples,1)
        state = newsample[0]
        (tmpfv,wgt,tmpH,tmpC)=sampleFvar(fvars,tvars,self.iH,self.iC,self.isiga,self.isigf_unconstrained_b,state.tau,state.f,state.get_turn(),[],[])
        fb = getbvars(fvars,tmpfv)
        return fb


    def addChat(self,fvars,observ):
        try:
            fb=self.drawFbSample(fvars)
            addChatToKnownWords(observ,fb,0.01)  #small weight here
        except AttributeError:  #lame
            return
        return



def is_array_of_floats(possarray):
    try:
        parr = [float(x) for x in possarray.split(',')]
    except ValueError:
        return False
    if len(parr)==3:
        return parr
    return False

        
    
#a function to ask the client for an action to take and map it to an EPA value
def ask_client(fbeh,sugg_act='',epaact=[],who="agent"):
    sst="action to take for "+who+" ('?' shows options): "

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

#a function to ask the client for a chat
def ask_client_chat(fbeh,sugg_act='',epaact=[],who="agent"):
    sst="action to take for "+who+" ('?' shows options): "

    sstnc="You can now either :\n"
    sstnc+="- say something free text"
    sstnc+="- type '\quit' to stop the simulation\n"
    #sstnc+="- type a comma separated list of three numbers to specify the action as E,P,A values\n"
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
            #its a chat - see if it has a hashtag in it
            cchat=""
            behlabel=""
            for word in cact.split():
                if word[0]=='#':
                    behlabel=word[1:]
                else:
                    #possibly do other clean up of the chat here
                    cchat = cchat+" "+word
                    
            #see if hashtag corresponds to a label in the dictionary
            if not behlabel=="":
                if re.sub(r"\s+","_",behlabel.strip()) in fbeh.keys():
                    cact=re.sub(r"\s+","_",behlabel.strip()) 
                    observ = map(lambda x: float (x), [fbeh[cact]["e"],fbeh[cact]["p"],fbeh[cact]["a"]])
                elif is_array_of_floats(behlabel):
                    observ = [float(x) for x in behlabel.split(',')]
                else:
                    #hashtag was not found, so ignore and continue as usual
                    return cchat
                #enter the chat into the dictionaries with a weight of 1  (the largest possible weight)
                addChatToKnownWords(cchat,observ,1)
                #return the observ value (EPA value)
                return observ
            #else, return the chat
            return cchat

    observ=map(lambda x: float (x), [fbeh[cact]["e"],fbeh[cact]["p"],fbeh[cact]["a"]])
    print "client said: ",cact
    return observ

fbehaviours_agent=readSentiments(fbfname,agent_gender)
fbehaviours_client=readSentiments(fbfname,client_gender)

(agent_mean_ids,agent_cov_ids)=getIdentityStats(fifname,agent_gender)
(client_mean_ids,client_cov_ids)=getIdentityStats(fifname,client_gender)

#the mean and covariance of IDs for male agents as taken from the databases
#should do this automatically in python based on actual genders of client/agent....as above now
#mean_ids=NP.array([0.40760,0.40548,0.45564])
#$cov_ids=NP.array([[2.10735,1.01121, 0.48442],[1.01121,1.22836,0.55593],[0.48442,0.55593,0.77040]])

#the actual (true) ids drawn from the distribution over ids, if not set to something in particular
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
(learn_tau_init,learn_prop_init,learn_beta_client_init,learn_beta_agent_init)=init_id(agent_knowledge,agent_id,client_id,client_mean_ids)

#overwrite these values for the interactive script only
#do this for mimicking interact
if mimic_interact:
    learn_beta_agent_init=bvagent_init
    learn_beta_client_init=bvclient_init


#initial x - only contains the turn for a default agent (no other x components)
learn_initx=initial_turn


#get the agent - can use some other subclass here if wanted 
learn_agent=LearningAgent(N=num_samples,alpha_value=1.0,
                          gamma_value=gamma_value,beta_value_agent=bvagent,beta_value_client=bvclient,
                          beta_value_client_init=learn_beta_client_init,beta_value_agent_init=learn_beta_agent_init,
                          client_gender=client_gender,agent_gender=agent_gender,
                          agent_rough=roughening_noise,client_rough=roughening_noise,use_pomcp=use_pomcp,
                          init_turn=initial_turn,numcact=numcact,numdact=numdact,obsres=obsres,actres=actres,pomcp_timeout=timeout)

print 10*"-","learning agent parameters: "
learn_agent.print_params()
print "learner init tau: ",learn_tau_init
print "learner prop init: ",learn_prop_init
print "learner beta client init: ",learn_beta_client_init
print "learner beta agent init: ",learn_beta_agent_init

#the following two initialisation calls should be inside the Agent constructor to keep things cleaner
learn_avgs=learn_agent.initialise_array(learn_tau_init,learn_prop_init,learn_initx)

print "learner average sentiments (f): "
learn_avgs.print_val()

    

done = False
iter=0
while not done:

    print 10*"-","iter ",iter,80*"-"

    learn_avgs = learn_agent.getAverageState()

    print "agent state is: "
    learn_avgs.print_val()

    #this always works here, but is only needed to avoid asking the user too many questions
    #and to figure out the observation 
    learn_turn=learn_avgs.get_turn()

    learn_turn = raw_input("whose turn (agent/client)?")
    observ=[]

    #if use_pomcp or learn_turn=="agent":
    #get the next action for the agent - may be a null action if it is the client turn
    (learn_aab,learn_paab)=learn_agent.get_next_action(learn_avgs,exploreTree=True)
    aact=findNearestBehaviour(learn_aab,fbehaviours_agent)
    if learn_turn=="agent":
        print "suggested action for the agent is :",learn_aab,"\n  closest label is: ",aact

    

    if learn_turn=="agent":
        #we only want to ask the user for an action if it is his turn, 
        #although this could be relaxed to allow the agent to barge in 
        #this will be relevant if the turn is non-deterministic, in which case there
        #may be some samples for each turn value, and we may want an action to take??
        

        #learn_aab=ask_client(fbehaviours_agent,aact,learn_aab,learn_turn)
        #print "agent does action :",learn_aab,"\n"
        #learn_observ=[]


        #here, we should set 
        learn_observ = ask_client_chat(fbehaviours_agent,aact,learn_aab,learn_turn)
        learn_aab=[]

    else:
        #first, see what the agent would predict and suggest this to the client
        #this can be removed in a true interactive setting, so this is only here so we can see what is going on
        (client_aab,client_paab)=learn_agent.get_default_predicted_action(learn_avgs)
        aact=findNearestBehaviours(client_aab,fbehaviours_agent,10)
        print "agent advises the following action :",client_aab,"\n  closest labels are: ", [re.sub(r"_"," ",i.strip()) for i in aact]

        #now, this is where the client actually decides what to do, possibly looking at the suggested labels from the agent
        #we use fbehaviours_agent here (for agent gender) as it is the agent who is perceiving this
        learn_observ=ask_client_chat(fbehaviours_agent,aact[0],client_aab,learn_turn)
        #should be to get a default (null)  action from the agent
        #learn_aab=[0.0,0.0,0.0]
        print "client action: ",learn_observ

    #we may be done if the user has killed the interaction
    if learn_turn=="client" and learn_observ==[]:
        done = True
    elif learn_turn=="agent" and learn_observ==[]:
        done = True
    else:
        #agent gets to observe the turn each time
        learn_xobserv=[State.turnnames.index(invert_turn(learn_turn))]

        #add the chat to the list of known words associated with the predicted fb
        learn_agent.addChat(fvars,learn_observ)
        
        #the main SMC update step 
        learn_avgs=learn_agent.propagate_forward(learn_aab,learn_observ,learn_xobserv,learn_paab,verb=learn_verbose)
        
        #I think these should be based on fundamentals, not transients
        (aid,cid)=learn_agent.get_avg_ids(learn_avgs.f)
        print "agent thinks it is most likely a: ",aid
        print "agent thinks the client is most likely a: ",cid

        if get_full_id_rate>0 and (iter+1)%get_full_id_rate==0:
            (cnt_ags,cnt_cls)=learn_agent.get_all_ids()
            print "agent thinks of itself as (full distribution): "
            print cnt_ags[0:10]
            print "agent thinks of the client as (full distribution): "
            print cnt_cls[0:10]
        iter += 1
    print "current deflection of averages: ",learn_agent.deflection_avg
            
    learn_d=learn_agent.compute_deflection()
    print "current deflection (agent's perspective): ",learn_d

    printKnownWords()
