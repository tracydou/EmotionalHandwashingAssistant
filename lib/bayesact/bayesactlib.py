"""------------------------------------------------------------------------------------------
Bayesian Affect Control Theory
Assistance Interactive Example
Changed from bayesactassistant.py within the same src folder
Work together with ../src/bayesact_server_stub.py
----------------------------------------------------------------------------------------------"""

#new verison is bayesactp - use just 'bayesact' to revert to the previous version
#also change in subclasses e.g. assistant.py and pwid.py
from bayesact import *
from assistant import *
from pwid import *

class BayesactAssistant:
    def __init__(self):
        #NP.set_printoptions(precision=5)
        #NP.set_printoptions(suppress=True)
        NP.set_printoptions(linewidth=10000)
         


        #-----------------------------------------------------------------------------------------------------------------------------
        #user-defined parameters 
        #-----------------------------------------------------------------------------------------------------------------------------
        #agent knowledge of client id: 
        #0 : nothing
        #1 : one of a selection of  num_confusers+1 randoms
        #2 : exactly
        #3 : same as 0 but also agent does not know its own id
        self.agent_knowledge=2

        # agent gender
        self.agent_gender="male"

        # client gender
        self.client_gender="male"

        #possibly set the agent id to be something
        #if not in database (including "") then it is a randomly drawn id
        self.agent_id="assistant"

        #can also set the client id here if agent_knowledge = 2 (knows id of client - see above)
        #if agent_knowledge is 0 then this is ignored
        self.client_id = "patient"

        #what is the client really? 
        self.true_client_id = "patient"
        #true_client_id = "psychotic"
        #client_id = ""

        #initial awareness distribution 0 = aware, 1 = unaware
        self.initial_px = [0.3,0.7]

        #set to "" to not do a fixed action
        self.fixed_agent_act=""

        #when there is a fixed agent action, this is the action taken when no prompt
        self.fixed_agent_default_act="mind"

        if len(sys.argv) > 1:
            self.true_client_id=sys.argv[1]
        if len(sys.argv) > 2:
            self.fixed_agent_act=sys.argv[2]
        if len(sys.argv) > 3:
            self.fixed_agent_default_act=sys.argv[3]


        #who goes first? 
        self.initial_learn_turn="client"
        self.initial_simul_turn="agent"



        self.num_plansteps=8  #includes the zeroth plan step and the end planstep
        self.num_behaviours=5

        #how often do we want to see the full id sets learned by the agent
        self.get_full_id_rate=-1


        #do we want to try to mimic interact?
        self.mimic_interact=False

        #if True, don't ask client just run
        self.do_automatic=False

        self.use_pomcp=False

        #parameters for POMCP
        self.numcact=5  #user defined 
        self.numdact=(self.num_plansteps-1)+1  #8 planstep changes  +  1 null action
        self.obsres=1.0
        self.actres=1.0
        self.timeout=10.0

        if not self.fixed_agent_act=="":
            self.numdact=1
            self.numcact=1   ##?? why 1 here - should stay the same at 9?

        print "do_pomcp? :",self.use_pomcp

        self.max_num_iterations=50

        #dictionary giving the state dynamics 
        self.nextPsDictOld={0:([0.8,0.2],[2,1]),
                    1:([1.0],[3]),
                    2:([1.0],[3]),
                    3:([1.0],[4]),
                    4:([0.8,0.2],[5,6]),
                    5:([1.0],[7]),
                    6:([1.0],[7]),
                    7:([1.0],[7])}

        self.nextPsDict={0:{0:([1.0],[0]),1:([1.0],[2]),2:([1.0],[1]),3:([1.0],[0]),4:([1.0],[0])},
                    1:{0:([1.0],[1]),1:([1.0],[3]),2:([1.0],[1]),3:([1.0],[1]),4:([1.0],[1])},
                    2:{0:([1.0],[2]),1:([1.0],[2]),2:([1],[3]),3:([1.0],[2]),4:([1.0],[1])},
                    3:{0:([1.0],[3]),1:([1.0],[3]),2:([0.6,0.4],[3,2]),3:([1.0],[4]),4:([1.0],[1])},
                    4:{0:([1.0],[4]),1:([1.0],[3]),2:([1],[6]),3:([1.0],[4]),4:([1.0],[5])},
                    5:{0:([1.0],[5]),1:([1.0],[3]),2:([1],[7]),3:([1.0],[4]),4:([1.0],[5])},
                    6:{0:([1.0],[6]),1:([1.0],[2]),2:([1],[6]),3:([1.0],[6]),4:([1.0],[7])},
                    7:{0:([1.0],[7]),1:([1.0],[2]),2:([0.6,0.4],[7,1]),3:([1.0],[7]),4:([1.0],[7])}}

        self.nextBehDict={0:([0.6,0.4],[2,1]),
                     1:([1.0],[1]),
                     2:([1.0],[2]),
                     3:([1.0],[3]),
                     4:([0.8,0.2],[2,4]),
                     5:([1.0],[2]),
                     6:([1.0],[4]),
                     7:([1.0],[0])}
 
        self.nextPsDictlinear={0:([1.0],[1]),
                    1:([1.0],[2]),
                    2:([1.0],[3]),
                    3:([1.0],[4]),
                    4:([1.0],[5]),
                    5:([1.0],[6]),
                    6:([1.0],[7]),
                    7:([1.0],[7])}



        #-----------------------------------------------------------------------------------------------------------------------------
        #these parameters can be tuned, but using caution
        #-----------------------------------------------------------------------------------------------------------------------------
        #agent's ability to change identities - higher means it will shape-shift more
        self.bvagent=0.0001
        #agent's belief about the client's ability to change identities - higher means it will shape-shift more
        self.bvclient=0.0001  #was 0.5

        self.simul_bvclient=0.001
        self.simul_bvagent=0.001

        #initial estimates of ids
        #learn_beta_agent_init is given by init_id, the others are overridden here
        self.agent_learn_beta_client_init=2.0     #0.0001

        #pwd has a vague identity and a rough concept of the identity of the agent
        self.simul_learn_beta_agent_init=0.0001
        self.simul_learn_beta_client_init=0.0001

        #-----------------------------------------------------------------------------------------------------------------------------
        #these parameters can be tuned, but will generally work "out of the box" for a basic simulation
        #-----------------------------------------------------------------------------------------------------------------------------

        #behaviours file
        self.fbfname="fbehaviours.dat"

        #identities file
        self.fifname="fidentities.dat"

        #get some key parameters from the command line
        #set much larger to mimic interact (>5000)
        self.num_samples=100


        # use 0.0 for mimic interact simulations
        #roughening_noise=0.0
        self.roughening_noise=self.num_samples**(-1.0/3.0)


        #the observation noise
        #set to 0.05 or less to mimic interact
        self.obs_noise=0.5

        self.xobsnoise=0.01
        self.simul_xobsnoise=0.00001

        if self.mimic_interact:
            #bvagent_init=0.000001
            self.bvclient_init=0.000001
            #bvagent=0.00001
            self.bvclient=0.00001
            self.agent_knowledge=2
            self.num_samples=100000
            self.roughening_noise=0.0
            self.obs_noise=0.01
            self.num_action_samples=10000


        #do we print out all the samples each time
        self.learn_verbose=False

        #for repeatability
        self.rseed = NP.random.randint(0,382948932)
        print "random seeed is : ",self.rseed
        NP.random.seed(self.rseed)
        
        #-----------------------------------------------------------------------------------------------------------------------------
        #code start - here there be dragons - only hackers should proceed, with caution
        #-----------------------------------------------------------------------------------------------------------------------------

        self.fbehaviours_agent=readSentiments(self.fbfname,self.agent_gender)
        self.fbehaviours_client=readSentiments(self.fbfname,self.client_gender)

        (self.agent_mean_ids,self.agent_cov_ids)=getIdentityStats(self.fifname,self.agent_gender)
        (self.client_mean_ids,self.client_cov_ids)=getIdentityStats(self.fifname,self.client_gender)

        #the mean and covariance of IDs for male agents as taken from the databases
        #should do this automatically in python based on actual genders of client/agent....as above now
        #mean_ids=NP.array([0.40760,0.40548,0.45564])
        #$cov_ids=NP.array([[2.10735,1.01121, 0.48442],[1.01121,1.22836,0.55593],[0.48442,0.55593,0.77040]])

        #the actual (true) ids drawn from the distribution over ids, if not set to something in particular
        self.aid=self.agent_id
        self.agent_id=getIdentity(self.fifname,self.agent_id,self.agent_gender)
        if self.agent_id==[]:
            self.agent_id=NP.random.multivariate_normal(self.agent_mean_ids,self.agent_cov_ids)
        self.agent_id=NP.asarray([self.agent_id]).transpose()

        self.client_id=getIdentity(self.fifname,self.client_id,self.agent_gender)
        if self.client_id==[]:
            self.client_id =  NP.random.multivariate_normal(self.client_mean_ids,self.client_cov_ids)
        self.client_id=NP.asarray([self.client_id]).transpose()


        self.client_agent_id=getIdentity(self.fifname,self.aid,self.client_gender)
        if self.client_agent_id==[]:
            self.client_agent_id=NP.random.multivariate_normal(self.agent_mean_ids,self.agent_cov_ids)
        self.client_agent_id=NP.asarray([self.client_agent_id]).transpose()



        self.true_client_id=getIdentity(self.fifname,self.true_client_id,self.client_gender)
        if self.true_client_id==[]:
            self.true_client_id =  NP.random.multivariate_normal(self.client_mean_ids,self.client_cov_ids)
        self.true_client_id=NP.asarray([self.true_client_id]).transpose()

        #get initial sets of parameters for agent
        (self.learn_tau_init,self.learn_prop_init,self.learn_beta_client_init,self.learn_beta_agent_init)=init_id(self.agent_knowledge,self.agent_id,self.client_id,self.client_mean_ids)


        (self.simul_tau_init,self.simul_prop_init,self.simul_beta_client_init,self.simul_beta_agent_init)=init_id(self.agent_knowledge,self.true_client_id,self.client_agent_id,self.agent_mean_ids)

        #overwrite these values for the interactive script only
        #do this for mimicking interact
        if self.mimic_interact:
            self.learn_beta_agent_init=self.bvagent_init
            self.learn_beta_client_init=self.bvclient_init


        if not self.fixed_agent_act=="":
            self.fbehv=self.fbehaviours_agent[self.fixed_agent_act]
            self.fixed_agent_aab=[float(self.fbehv["e"]),float(self.fbehv["p"]),float(self.fbehv["a"])]
            print "agent taking fixed action: ",self.fixed_agent_act," which is ",self.fixed_agent_aab

            self.fbehv=self.fbehaviours_agent[self.fixed_agent_default_act]
            self.fixed_agent_default_aab=[float(self.fbehv["e"]),float(self.fbehv["p"]),float(self.fbehv["a"])]
            print "agent taking fixed default action: ",self.fixed_agent_default_act," which is ",self.fixed_agent_default_aab
        else:
            print "agent taking bayesact actions"

        #set up fixed policy
        #this is an array fixed_policy[i] giving the affective action to take for 
        #each discrete action available
        self.fixed_policy=None
        if not self.fixed_agent_act=="":
            self.fixed_policy={}
            self.fixed_policy[0]=mapSentimentLabeltoEPA(self.fbehaviours_agent,self.fixed_agent_default_act)

            for a in range(self.numdact):
                self.fixed_policy[a+1]=mapSentimentLabeltoEPA(self.fbehaviours_agent,self.fixed_agent_act)


        #get the agent - can use some other subclass here if wanted 
        self.learn_agent=Assistant(N=self.num_samples,alpha_value=1.0,
                              gamma_value=self.obs_noise,beta_value_agent=self.bvagent,beta_value_client=self.bvclient,
                              beta_value_client_init=self.agent_learn_beta_client_init,beta_value_agent_init=self.learn_beta_agent_init,
                              client_gender=self.client_gender,agent_gender=self.agent_gender,use_pomcp=self.use_pomcp,
                              agent_rough=self.roughening_noise,client_rough=self.roughening_noise, nextpsd = self.nextPsDict,
                              nextbd = self.nextBehDict, onoise=self.xobsnoise, 
                              fixed_policy=self.fixed_policy,pomcp_interactive=True,
                              numcact=self.numcact,numdact=self.numdact,obsres=self.obsres,actres=self.actres,pomcp_timeout=self.timeout)


        #get the agent - can use some other subclass here if wanted 
        self.simul_agent=PwD(N=self.num_samples,alpha_value=1.0,
                        gamma_value=self.obs_noise,beta_value_agent=self.simul_bvagent,beta_value_client=self.simul_bvclient,
                        beta_value_client_init=self.simul_learn_beta_client_init,beta_value_agent_init=self.simul_learn_beta_agent_init,
                        client_gender=self.agent_gender,agent_gender=self.client_gender,
                        agent_rough=self.roughening_noise,client_rough=self.roughening_noise, nextpsd = self.nextPsDictOld, onoise=self.simul_xobsnoise,
                        numcact=self.numcact,numdact=self.numdact,obsres=self.obsres,actres=self.actres,pomcp_timeout=self.timeout)

        self.learn_initx=[self.initial_learn_turn,self.initial_px]
        self.simul_initx=[self.initial_simul_turn,self.initial_px]


        self.learn_avgs=self.learn_agent.initialise_array(self.learn_tau_init,self.learn_prop_init,self.learn_initx)
        self.simul_avgs=self.simul_agent.initialise_array(self.simul_tau_init,self.simul_prop_init,self.simul_initx)


        #cval,numaact,numpact,actres,obsres):
        #learn_agent.POMCP_initialise(c_val,numact,num_plansteps+1,actres,obsres,timeout,fixed_policy)
        #learn_agent.POMCP_initialise(c_val,numact,num_plansteps+1,obsres,timeout,fixed_policy)
        #if use_pomcp:
        #    learn_agent.initialise_pomcp(cval,

        self.learn_turn=self.initial_learn_turn
        self.simul_turn=self.initial_simul_turn    



        self.done = False
        self.iter=0
        self.ps_obs=0
        
        self.print_parameters_after_initialization()
        print "==================== END of INITIALIZATION ================"
        
    def print_parameters_after_initialization(self):
        if self.use_pomcp:
            print "pomcp obsres:  ",self.obsres
            print "pomcp actres:  ",self.actres
            print "pomcp timeout: ",self.timeout
            print "pomcp numcact: ",self.numcact
            print "pomcp numdact: ",self.numdact

        print "true_client_id: ",self.true_client_id
        print "fixed_agent_act: ",self.fixed_agent_act
        print "fixed_agent_default_act: ",self.fixed_agent_default_act
        print "fixed policy is: "
        print self.fixed_policy

        print 10*"-","learning agent parameters: "
        self.learn_agent.print_params()
        print "learner init tau: ",self.learn_tau_init
        print "learner prop init: ",self.learn_prop_init
        print "learner beta client init: ",self.learn_beta_client_init
        print "learner beta agent init: ",self.learn_beta_agent_init

        print 10*"-","simulated agent parameters: "
        self.simul_agent.print_params()
        
        print "learner (assistant) average sentiments (f) after initialisation: "
        self.learn_avgs.print_val()
        print "simulator (pwid) average sentiments (f) after initialisation: "
        self.simul_avgs.print_val()
		
    
    def calculate_helper(self, epa, action):
        print 10*"#"," current turn: ",self.learn_turn," ",10*"#"

        self.observ=[]
        print 10*"-","iter ",self.iter,80*"-"

        (self.learn_aab,self.learn_paab)=self.learn_agent.get_next_action(self.learn_avgs)
#        print "agent action/client observ: ",self.learn_aab        
        self.simul_observ=self.learn_aab
#        print "agent prop. action: ",self.learn_paab
        
        (self.simul_aab,self.simul_paab)=self.simul_agent.get_next_action(self.simul_avgs)
#        print "client action/agent observ: ",self.simul_aab,
        self.learn_observ=self.simul_aab
#        print "client prop. action: ",self.simul_paab


#       self.learn_aact=findNearestBehaviour(self.learn_aab,self.fbehaviours_agent)
#        print "suggested action for the agent is :",self.learn_aab,"\n  closest label is: ",self.learn_aact
#        print "agent's proposition action : ",self.learn_paab,"\n"

#        self.simul_aact=findNearestBehaviours(self.simul_aab,self.fbehaviours_agent,10)
#        print "client's proposition action : ",self.simul_paab,"\n"
#        print "agent advises the following action :",self.simul_aab,"\n  closest labels are: ", [re.sub(r"_"," ",i.strip()) for i in self.simul_aact]
        
        #initialize
        result_epa = [0,0,0]
        result_action = 0
        
        if self.learn_turn=="agent":
            #tracy#used to call "learn_aab=ask_client(fbehaviours_agent,learn_aact,learn_aab)"
            result_epa = self.learn_aab
            result_action = self.convert_prompt_number(self.learn_paab)
            print "agent will act :",self.learn_aab
            print "corresponding propositional prompt is:", self.learn_paab
            self.simul_observ=self.learn_aab
            self.learn_observ=[]  #awkward
        else:
            #now, this is where the client actually decides what to do, possibly looking at the suggested labels 
            #tracy#simul_aab=ask_client(fbehaviours_client,simul_aact[0],simul_aab)
            self.simul_aab=epa
            print "client performed action: ",self.simul_aab
            self.learn_observ=self.simul_aab
            self.simul_observ=[]  #awkward

        
        #observation of behaviour - 
        self.behav_obs = action
        self.behav_obs = int(self.behav_obs)


        if self.learn_turn=="client" and self.learn_observ==[]:
            self.done = True
        elif self.learn_turn=="agent" and self.learn_aab==[]:
            self.done = True
        elif self.iter > self.max_num_iterations:
            self.done = True
        elif self.simul_agent.is_done():
            print "all done"
            self.done = True
        else:
            self.learn_xobs=[State.turnnames.index(invert_turn(self.learn_turn)),self.behav_obs]
            self.learn_avgs=self.learn_agent.propagate_forward(self.learn_aab,self.learn_observ,xobserv=self.learn_xobs,paab=self.learn_paab,verb=self.learn_verbose)

#            print "agent f is: "
#            self.learn_avgs.print_val()

            #learn_paab is passed into client as the x-observation
            self.simul_xobs=[State.turnnames.index(invert_turn(self.simul_turn)),self.learn_paab]
            self.simul_avgs=self.simul_agent.propagate_forward(self.simul_aab,self.simul_observ,xobserv=self.simul_xobs,paab=None,verb=self.learn_verbose)

#            print "client f is: "
#            self.simul_avgs.print_val()


            #I think these should be based on fundamentals, not transients
#            (self.aid,self.cid)=self.learn_agent.get_avg_ids(self.learn_avgs.f)
#            print "agent thinks it is most likely a: ",self.aid
#            print "agent thinks the client is most likely a: ",self.cid

#            (self.aid,self.cid)=self.simul_agent.get_avg_ids(self.simul_avgs.f)
#            print "client thinks it is most likely a: ",self.aid
#            print "client thinks the agent is most likely a: ",self.cid
            
#            print "client state is: "
#            self.simul_agent.print_state()

#            if self.get_full_id_rate>0 and (self.iter+1)%self.get_full_id_rate==0:
#                (self.cnt_ags,self.cnt_cls)=self.learn_agent.get_all_ids()
#                print "agent thinks of itself as (full distribution): "
#                print self.cnt_ags[0:10]
#                print "agent thinks of the client as (full distribution): "
#                print self.cnt_cls[0:10]

            self.iter += 1
#        print "current deflection of averages: ",self.learn_agent.deflection_avg

#        print "current deflection of averages (client): ",self.simul_agent.deflection_avg
                
#        self.learn_d=self.learn_agent.compute_deflection()
#        print "current deflection (agent's perspective): ",self.learn_d

#        self.simul_d=self.simul_agent.compute_deflection()
#        print "current deflection (client's perspective): ",self.simul_d

        if self.learn_turn=="client":
            self.learn_turn="agent"
        elif self.learn_turn=="agent":
            self.learn_turn="client"

        if self.simul_turn=="client":
            self.simul_turn="agent"
        elif self.simul_turn=="agent":
            self.simul_turn="client"
        return (self.done,result_epa,result_action)
        
    def calculate(self, epa, action):
		self.calculate_helper(epa, action)
		return self.calculate_helper(epa, action)
		
    def convert_prompt_number(self, learn_paab):
		curr_planstep = int(self.learn_agent.x_avg[1])
		if (curr_planstep==0 and learn_paab==1) or (curr_planstep==2 and learn_paab==3):
			return 1
		elif (curr_planstep==0 and learn_paab==2) or (curr_planstep==1 and learn_paab==3):
			return 2
		elif (curr_planstep==3 and learn_paab==4):
			return 3
		elif (curr_planstep==4 and learn_paab==5) or (curr_planstep==6 and learn_paab==7):
			return 5
		elif (curr_planstep==4 and learn_paab==6) or (curr_planstep==5 and learn_paab==7):
			return 4
		elif (curr_planstep==7 and learn_paab==7):
			return 6

    def __del__(self):
        print "final simul agent state: "
        self.simul_agent.print_state()

if __name__ == "__main__":
    bayesact_assistant = BayesactAssistant()
    done,epa,prompt=bayesact_assistant.calculate([1.0,2.0,3.0],2)
    print "epa=",epa
    print "prompt=",prompt
