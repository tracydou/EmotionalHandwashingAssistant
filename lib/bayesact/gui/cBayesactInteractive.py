import sys
sys.path.append("../")
from bayesact import *
import getopt
import cProfile
from cEnum import eTurn
from cBayesactInteractiveGui import cBayesactInteractiveGuiPanel

class cBayesactInteractive(object):
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
        #2 : exactly - use this to mimic interact
        #3 : same as 0 but also agent does not know its own id
        self.agent_knowledge=2

        # agent gender
        self.agent_gender="male"

        # client gender
        self.client_gender="male"

        #possibly set the agent id to be something
        self.agent_id="tutor"
        #if not in database (including "") then it is a randomly drawn id
        #agent_id=""

        #can also set the client id here if agent_knowledge = 2 (knows id of client - see above)
        #if agent_knowledge is 0 then this is ignored
        #client_id = "professor"
        self.client_id = "student"

        #who goes first?
        self.initial_turn="agent"

        #how often do we want to see the full id sets learned by the agent
        self.get_full_id_rate=10


        #do we want to try to mimic interact?
        self.mimic_interact=False

        #use pomcp for planning (default use heuristic/greedy method)
        self.use_pomcp=False

        #parameters for pomcp
        #number of continuous actions we wish to sample -
        #this is user-defined and is an important parameter
        #larger numbers mean bigger, slower, more accurate,  planning trees
        self.numcact=5
        #number of discrete (propositional) actions
        #this should be set according to the domain, and is 1 for this generic class
        #one discrete action really means no choice
        self.numdact=1
        #observation resolution when building pomcp plan tree
        self.obsres=1.0
        #action resolution when buildling pomcp plan tree
        self.actres=0.1
        #timeout used for POMCP
        self.timeout=5.0

        #-----------------------------------------------------------------------------------------------------------------------------
        #these parameters can be tuned, but using caution
        #-----------------------------------------------------------------------------------------------------------------------------
        self.agent_alpha_value = 1.0

        #agent's ability to change identities - higher means it will shape-shift more
        self.agent_beta_of_agent=0.0001
        #agent's belief about the client's ability to change identities - higher means it will shape-shift more
        self.agent_beta_of_client=0.0001


        #-----------------------------------------------------------------------------------------------------------------------------
        #these parameters can be tuned, but will generally work "out of the box" for a basic simulation
        #-----------------------------------------------------------------------------------------------------------------------------

        #behaviours file
        self.fbfname="fbehaviours.dat"

        #identities file
        self.fifname="fidentities.dat"

        #get some key parameters from the command line
        #set much larger to mimic interact (>5000)
        self.num_samples=1000


        # use 0.0 for mimic interact simulations
        #roughening_noise=0.0
        self.roughening_noise=self.num_samples**(-1.0/3.0)


        #the observation noise
        #set to 0.05 or less to mimic interact
        self.obs_noise=0.1

        self.gamma_value=self.obs_noise


        #do we print out all the samples each time
        self.learn_verbose=False

        #for repeatability
        self.rseed = NP.random.randint(0,382948932)
        #rseed=271887164

        NP.random.seed(self.rseed)



        # for init
        self.fbehaviours_agent = None
        self.fbehaviours_client = None

        self.rseed = None
        self.client_id = None
        self.agent_id = None

        self.learn_tau_init = None
        self.learn_prop_init = None
        self.learn_beta_client_init = None
        self.learn_beta_agent_init = None

        self.learn_initx = None

        self.learn_agent = None

        self.learn_avgs = None
        self.learn_turn = None

        self.learn_d = None

        self.client_action = ""

        self.total_iterations = 0

        # plotting and for gui
        self.plotter = None
        self.gui_manager = None
        self.thread_event = None
        self.waiting = True
        self.terminate_flag = False

        # for running instructions
        self.update_environment_noise           = False
        self.update_gamma_value                 = False

        self.update_agent_alpha                 = False

        self.update_agent_beta_of_client        = False
        self.update_agent_beta_of_agent         = False




    def startThread(self):
        if self.mimic_interact:
                self.mimicInteract()

        self.waiting = False
        print "agent alpha: ",self.agent_alpha_value
        print "agent beta of client: ",self.agent_beta_of_client
        print "agent beta of agent: ",self.agent_beta_of_agent
        print "agent knowledge: ",self.agent_knowledge
        print "roughening noise: ",self.roughening_noise
        print "gamma value: ",self.gamma_value
        print "client id label: ",self.client_id_label
        print "agent id label: ",self.agent_id_label
        print "agent gender: ",self.agent_gender
        print "client gender: ",self.client_gender

        self.initBayesactInteractive()
        self.mainLoop()


    def mainLoop(self):
        while(not(self.terminate_flag)):
            self.startInteraction()

        self.plotter.clearPlots()


    def runInstructions(self):
        if (isinstance(self.gui_manager, cBayesactInteractiveGuiPanel)):
            self.checkParams()


    def checkParams(self):
        recompute_learn_agent = False

        if (self.update_agent_alpha):
            self.agent_alpha_value = float(self.gui_manager.m_AgentAlphaTextBox.GetValue())
            self.learn_agent.alpha_value = self.agent_alpha_value
            recompute_learn_agent = True
            self.update_agent_alpha = False

        if (self.update_agent_beta_of_client):
            self.agent_beta_value_of_client = float(self.gui_manager.m_AgentBetaOfClientTextBox.GetValue())
            self.learn_agent.beta_value_client = self.agent_beta_value_of_client
            recompute_learn_agent = True
            self.update_agent_beta_of_client = False

        if (self.update_agent_beta_of_agent):
            self.agent_beta_value_of_agent = float(self.gui_manager.m_AgentBetaOfAgentTextBox.GetValue())
            self.learn_agent.beta_value_agent = self.agent_beta_value_of_agent
            recompute_learn_agent = True
            self.update_agent_beta_of_agent = False

        if (self.update_gamma_value):
            self.gamma_value = float(self.gui_manager.m_GammaValueTextBox.GetValue())
            self.learn_agent.gamma_value = self.gamma_value
            recompute_learn_agent = True
            self.update_gamma_value = False

        # To be used after updating alpha, beta, and gamma values
        if (recompute_learn_agent):
            self.learn_agent.init_covariances()
            recompute_learn_agent = False



    def mimicInteract(self):
        self.agent_beta_of_agent_init=0.000001
        self.agent_beta_of_client_init=0.000001
        self.agent_beta_of_agent=0.00001
        self.agent_beta_of_client=0.00001
        self.agent_knowledge=2
        self.num_samples=10000
        self.roughening_noise=0.0
        self.obs_noise=0.01


    def initBayesactInteractive(self):
        print "random seeed is : ",self.rseed

        #-----------------------------------------------------------------------------------------------------------------------------
        #code start - here there be dragons - only hackers should proceed, with caution
        #-----------------------------------------------------------------------------------------------------------------------------


        self.fbehaviours_agent=readSentiments(self.fbfname,self.agent_gender)
        self.fbehaviours_client=readSentiments(self.fbfname,self.client_gender)

        (agent_mean_ids,agent_cov_ids)=getIdentityStats(self.fifname,self.agent_gender)
        (client_mean_ids,client_cov_ids)=getIdentityStats(self.fifname,self.client_gender)

        #the mean and covariance of IDs for male agents as taken from the databases
        #should do this automatically in python based on actual genders of client/agent....as above now
        #mean_ids=NP.array([0.40760,0.40548,0.45564])
        #$cov_ids=NP.array([[2.10735,1.01121, 0.48442],[1.01121,1.22836,0.55593],[0.48442,0.55593,0.77040]])

        #the actual (true) ids drawn from the distribution over ids, if not set to something in particular
        self.agent_id=getIdentity(self.fifname,self.agent_id,self.agent_gender)
        if self.agent_id==[]:
            self.agent_id=NP.random.multivariate_normal(agent_mean_ids,agent_cov_ids)
        self.agent_id=NP.asarray([self.agent_id]).transpose()

        #here we get the identity of the client *as seen by the agent*
        self.client_id=getIdentity(self.fifname,self.client_id,self.agent_gender)
        if self.client_id==[]:
            self.client_id =  NP.random.multivariate_normal(client_mean_ids,client_cov_ids)
        self.client_id=NP.asarray([self.client_id]).transpose()

        #get initial sets of parameters for agent
        (self.learn_tau_init,self.learn_prop_init,self.learn_beta_client_init,self.learn_beta_agent_init)=init_id(self.agent_knowledge,self.agent_id,self.client_id,client_mean_ids)

        #overwrite these values for the interactive script only
        #do this for mimicking interact
        if self.mimic_interact:
            self.learn_beta_agent_init=self.agent_beta_of_agent_init
            self.learn_beta_client_init=self.agent_beta_of_client_init


        #initial x - only contains the turn for a default agent (no other x components)
        self.learn_initx=self.initial_turn


        #get the agent - can use some other subclass here if wanted
        self.learn_agent=Agent(N=self.num_samples,alpha_value=self.agent_alpha_value,
                          gamma_value=self.gamma_value,beta_value_agent=self.agent_beta_of_agent,beta_value_client=self.agent_beta_of_client,
                          beta_value_client_init=self.learn_beta_client_init,beta_value_agent_init=self.learn_beta_agent_init,
                          client_gender=self.client_gender,agent_gender=self.agent_gender,
                          agent_rough=self.roughening_noise,client_rough=self.roughening_noise,use_pomcp=self.use_pomcp,
                          init_turn=self.initial_turn,numcact=self.numcact,numdact=self.numdact,obsres=self.obsres,actres=self.actres,pomcp_timeout=self.timeout)


        print 10*"-","learning agent parameters: "
        self.learn_agent.print_params()
        print "learner init tau: ",self.learn_tau_init
        print "learner prop init: ",self.learn_prop_init
        print "learner beta client init: ",self.learn_beta_client_init
        print "learner beta agent init: ",self.learn_beta_agent_init

        #the following two initialisation calls should be inside the Agent constructor to keep things cleaner
        self.learn_avgs=self.learn_agent.initialise_array(self.learn_tau_init,self.learn_prop_init,self.learn_initx)


        #To plot initial data
        #to send the initial sentiments to the plotter
        self.learn_agent.sendSamplesToPlotter(self.learn_agent.samples,self.plotter,eTurn.learner)
        self.plotter.plot()


        print "learner average sentiments (f): "
        self.learn_avgs.print_val()



        self.terminate_flag = False
        self.total_iterations=0



    def is_array_of_floats(self,possarray):
        try:
            parr = [float(x) for x in possarray.split(',')]
        except ValueError:
            return False
        if len(parr)==3:
            return parr
        return False


    #a function to ask the client for an action to take and map it to an EPA value
    def ask_client(self,fbeh,sugg_act='',epaact=[],who="agent"):
        sst="action to take for "+who+" ('?' shows options): "

        sstnc="You can now either :\n"
        sstnc+="- pick an action (behaviour) and type in its label\n"
        sstnc+="- type 'quit' to stop the simulation\n"
        sstnc+="- type a comma separated list of three numbers to specify the action as E,P,A values\n"
        if not sugg_act=='':
            sstnc+="- hit 'enter' to take default action with label: "+sugg_act[0]+"\n"
        if not epaact=='':
            sstnc+="- type any digit (0-9) to take suggsted action: "+str(epaact)


        while True:
            #cact = raw_input(sst)

            self.gui_manager.m_CurrentTurnTextBox.SetValue(self.learn_turn)

            # When two programs access the same resource, the program will freak out and say it's leaking
            self.waiting = True
            # Pause and await instruction
            self.thread_event.clear()
            self.thread_event.wait()
            self.runInstructions()
            self.waiting = False
            #self.plotter.plot()

            cact = self.client_action


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
            elif self.is_array_of_floats(cact):
                return [float(x) for x in cact.split(',')]
            else:
                print "incorrect or not found, try again. '?' shows options"
        observ=map(lambda x: float (x), [fbeh[cact]["e"],fbeh[cact]["p"],fbeh[cact]["a"]])
        print "client said: ",cact
        return observ


    def populateSuggestedActions(self, iActions):
        if (isinstance(iActions, list)):
            self.gui_manager.m_SuggestedActionsListBox.SetItems(iActions)
        else:
            self.gui_manager.m_SuggestedActionsListBox.SetItems([iActions])


    def startInteraction(self):
        print 10*"-","iter ",self.total_iterations,80*"-"

        self.learn_avgs = self.learn_agent.getAverageState()

        print "agent state is: "
        self.learn_avgs.print_val()

        #this always works here, but is only needed to avoid asking the user too many questions
        #and to figure out the observation
        self.learn_turn=self.learn_avgs.get_turn()


        observ=[]

        
        # To get the next action and propagate_forward on it
        # ####################################################################
        
        #if use_pomcp or learn_turn=="agent":
        #get the next action for the agent - may be a null action if it is the client turn
        (learn_aab,learn_paab)=self.learn_agent.get_next_action(self.learn_avgs,exploreTree=True)
        #aact=findNearestBehaviour(learn_aab,self.fbehaviours_agent)
        aact=findNearestBehaviours(learn_aab,self.fbehaviours_agent,10)
        if self.learn_turn=="agent":
            print "suggested action for the agent is :",learn_aab,"\n  closest label is: ",aact


        if self.learn_turn=="agent":
            #we only want to ask the user for an action if it is his turn,
            #although this could be relaxed to allow the agent to barge in
            #this will be relevant if the turn is non-deterministic, in which case there
            #may be some samples for each turn value, and we may want an action to take??

            # Populate the suggested actions
            self.populateSuggestedActions(aact)

            learn_aab=self.ask_client(self.fbehaviours_agent,aact,learn_aab,self.learn_turn)
            print "agent does action :",learn_aab,"\n"
            learn_observ=[]
        else:
            #first, see what the agent would predict and suggest this to the client
            #this can be removed in a true interactive setting, so this is only here so we can see what is going on
            (client_aab,client_paab)=self.learn_agent.get_default_predicted_action(self.learn_avgs)
            aact=findNearestBehaviours(client_aab,self.fbehaviours_agent,10)
            print "agent advises the following action :",client_aab,"\n  closest labels are: ", [re.sub(r"_"," ",i.strip()) for i in aact]

            #now, this is where the client actually decides what to do, possibly looking at the suggested labels from the agent
            #we use fbehaviours_agent here (for agent gender) as it is the agent who is perceiving this

            # Populate the suggested actions -- This time, aact should be an array of strings
            self.populateSuggestedActions(aact)

            learn_observ=self.ask_client(self.fbehaviours_agent,aact[0],client_aab,self.learn_turn)
            #should be to get a default (null)  action from the agent
            #learn_aab=[0.0,0.0,0.0]
            print "client action: ",learn_observ

            
        # ####################################################################
            
            
        #we may be done if the user has killed the interaction
        if self.learn_turn=="client" and learn_observ==[]:
            self.terminate_flag = True
        elif self.learn_turn=="agent" and learn_aab==[]:
            self.terminate_flag = True
        else:
            #agent gets to observe the turn each time
            learn_xobserv=[State.turnnames.index(invert_turn(self.learn_turn))]

            #the main SMC update step
            self.learn_avgs=self.learn_agent.propagate_forward(learn_aab,learn_observ,learn_xobserv,learn_paab,verb=self.learn_verbose,plotter=self.plotter,agent=eTurn.learner)

            #To plot data
            if (None != self.plotter):
                self.plotter.plot()


            #I think these should be based on fundamentals, not transients
            (aid,cid)=self.learn_agent.get_avg_ids(self.learn_avgs.f)
            print "agent thinks it is most likely a: ",aid
            print "agent thinks the client is most likely a: ",cid

            if self.get_full_id_rate>0 and (self.total_iterations+1)%self.get_full_id_rate==0:
                (cnt_ags,cnt_cls)=self.learn_agent.get_all_ids()
                print "agent thinks of itself as (full distribution): "
                print cnt_ags[0:10]
                print "agent thinks of the client as (full distribution): "
                print cnt_cls[0:10]


            self.total_iterations += 1

        print "current deflection of averages: ",self.learn_agent.deflection_avg

        self.learn_d=self.learn_agent.compute_deflection()
        print "current deflection (agent's perspective): ",self.learn_d

def main(argv):
    a = cBayesactTools(argv)
    a.startBayesactSim()
    #a.evaluateTrials(a.num_trials, a.num_experiments, a.max_horizon)
    a.evaluateTrials(1, 1, 1)

if __name__ == "__main__":
    main(sys.argv)
