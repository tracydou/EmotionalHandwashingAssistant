"""------------------------------------------------------------------------------------------
Bayesian Affect Control Theory
Assistant Subclass of Agent class
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


class Assistant(Agent):
    def __init__(self,*args,**kwargs):

        super(Assistant, self).__init__(self,*args,**kwargs)
        #x for the Assistant is a triple: 
        #the first is the turn (required)
        #the second value
        #is the planstep (0 to N-1) and the third
        # is the binary awareness of the client 0/1

        self.obs_noise = kwargs.get("onoise",0.1)

        #dictionary defining the dynamics of the state
        self.nextPsDict = kwargs.get("nextpsd",{0:([1.0],[1]),1:([1.0],[1])})
        self.num_plansteps=len(self.nextPsDict)

        self.reconPsDict = {0:2,1:3,2:3,3:4,4:5,5:7,6:7,7:7}
        #self.reconPsDict = {0:1,1:2,2:3,3:4,4:5,5:6,6:7,7:7}


        #initial awarness distribution
        self.px = [0.3,0.7]

        self.defb = [1.0,0.99,0.95,0.5,0.3,0.2,0.1,0.05,0.02,0.01]
        self.defbnp = [0.8,0.6,0.5,0.3,0.2,0.1,0.05,0.005,0.002,0.001]


        #observation is only of the planstep (and the turn - [turn,planstep])
        self.of = (NP.diag(NP.ones(self.num_plansteps)*(1.0-self.obs_noise-(self.obs_noise/(self.num_plansteps-1)))) +
                   NP.ones((self.num_plansteps,self.num_plansteps))*(self.obs_noise/(self.num_plansteps-1)))
        
        self.x_avg=[0,0,0]
        
        self.lastPrompt = 0

        #propositional (discrete) actions are 0 (do nothing) and a=1...num_plansetps-1 (prompt for planstep a)
        self.numDiscActions=self.num_plansteps   #one action (number 0) is "do nothing" or "None"


    def print_params(self):
        Agent.print_params(self)
        print "num plansteps: ",self.num_plansteps

        print "observation functin:  ",self.of
        print "ps dynamics: ",self.nextPsDict

    #The only difference between an Assistant and an Agent
    #is dynamics of x and reward
    def initialise_x(self,initx):
        if not initx:
            initpx=self.px
            initturn=State.turnnames.index("agent")  #hard-coded agent start?
        else:
            initpx=initx[1]
            initturn=State.turnnames.index(initx[0])
        #draw a sample from initx over awareness
        initx=list(NP.random.multinomial(1,initpx)).index(1)
        initps=0
        return [initturn,initps,initx]


    def is_done(self):
        return abs(self.x_avg[1]-self.num_plansteps+1)<=0.1

    #implement a default policy where we look at the client's current awareness  (average x[] value)
    #and prompt if this is less than a fixed threshold.
    #the prompt is based on the expected planstep that the client is in
    #should also look at deflection - can we access this here? 
    def get_prop_action(self,state,rollout=False,samplenum=0):

        #for POMCP use this because it will be calling this during rollouts - this is the rollout policy
        #for propositional actions
        if self.use_pomcp:
            if rollout:
                return NP.random.randint(self.numDiscActions)
            else:
                return samplenum%self.numDiscActions #loop through them one by one
            
        else:
            #heuristic policy
            awareness = self.x_avg[2]
            #zero is to do nothing at all
            propositional_action=0
            curr_planstep = int(self.x_avg[1])
            if awareness < 0.4:
                propositional_action=self.getRecommendedNextPlanStep(curr_planstep)  #used to add 1 here, but that was wrong
            return propositional_action

    #get the next planstep to take according to a plan-graph
    def getNextPlanStep(self,planstep):
        newps=self.nextPsDict[planstep][1][list(NP.random.multinomial(1,self.nextPsDict[planstep][0])).index(1)]
        return newps 

    #def getRecommendedNextPlanStep(self,planstep):
    #    return min(self.num_plansteps-1,planstep+1)   #in general, could be next step according to some plan-graph
    def getRecommendedNextPlanStep(self,planstep):
        return self.reconPsDict[planstep]

    #aab is the affective part of the action, paab is the propositional part
    def sampleXvar(self,f,tau,state,aab,paab):
        #there is no prompt, the planstep only changes based on awareness
        #and awareness only decreases
        awareness=state.x[2]
        planstep=state.x[1]
        new_awareness=awareness
        new_planstep=planstep

        if state.get_turn()=="agent":
            #agent turn - save action
            self.lastPrompt  = paab
        
        #compute deflection between f and tau
        #x moves towards goalx inversely proportionally to 
        #the deflection
        D=NP.dot(f-tau,f-tau)
        #print "D: ",D," awareness: ",awareness, "planstep: ",planstep, " lastprompt: ",self.lastPrompt
        if awareness == 1:
            #the client is aware
            if self.lastPrompt == 0:
                #without a prompt, the client will do the right thing and stay aware if deflection is low
                if NP.random.random() < self.defbnp[min(9,int(round(D)))]: #> 0.5:
                    new_planstep = self.getNextPlanStep(planstep)
                #otherwise, loses awareness
                else:
                    new_awareness = 0
                #without a prompt, the client is likely to do the right thing
                #if NP.random.random() > 0.5:
                #    new_planstep = self.getNextPlanStep(planstep)
                #but may lose awareness randomly, and this increases with deflection  (was a constant 0.5 before)
                #elif NP.random.random() > self.defb[min(9,int(round(self.deflection_avg)))]: #0.5:
                #    new_awareness = 0
            else:
                #print D,self.defb[min(9,int(round(D)))],self.lastPrompt,planstep,awareness,self.nextPsDict[planstep][1]
                #in a high deflection situation, a prompt will likely confuse the client
                if NP.random.random() > self.defb[min(9,int(round(D)))]: 
                    new_awareness = 0
                #otherwise, a prompt for the wrong thing will also mess things up
                elif not self.lastPrompt in self.nextPsDict[planstep][1]:
                    new_awareness = 0
                #otherwise, is unlikely to mess things up unless it is wrong
                elif NP.random.random() > 0.3:
                    new_planstep=self.getNextPlanStep(planstep)
                #but if it does, the client may lose awareness, and this is deflection dependent
                elif NP.random.random() > self.defb[min(9,int(round(D)))]: #0.8:
                    new_awareness = 0
        else:
            #the client is not aware
            if self.lastPrompt == 0:
                #without a prompt
                #very likely that client will do nothing, 
                #but might randomly gain awareness and move on
                if NP.random.random() > 0.99:
                    new_planstep = self.getNextPlanStep(planstep)
                    new_awareness = 1
            else:
                #with a low-deflection prompt for the next step, user will likely move on
                # and gain awareness
                if NP.random.random() < self.defb[min(9,int(round(D)))] and self.lastPrompt in self.nextPsDict[planstep][1]:
                    new_planstep = self.getNextPlanStep(planstep)   #should be more biased towards lastPrompt 
                    new_awareness = 1
                else:
                    #high deflection prompt will be unlikely to have an effect
                    if NP.random.random() > 0.99:
                        new_awareness = 1
        return [state.invert_turn(),new_planstep,new_awareness]
    
    #this must be in the class
    def evalSampleXvar(self,sample,xobs):
        #print sample.x[0],sample.x[1],xobs,self.of[sample.x[1]][xobs[1]]
        if sample.x[0]==xobs[0]:
            return self.of[sample.x[1]][xobs[1]]
        else:
            return 0.0

    #an observation sample is [turn, planstep]
    def sampleXObservation(self,sample):
        return [sample.x[0],list(NP.random.multinomial(1,self.of[sample.x[1]])).index(1)]

    def reward(self,sample,action=None):
        xreward=0.0
        if sample.x[1] == self.num_plansteps-1:
            xreward = 10.0
        elif sample.x[1] == 1:  #intermediate goal states
            xreward = 0.01
        elif sample.x[1] == 2:  #intermediate goal states
            xreward = 0.01
        elif sample.x[1] == 3:  #intermediate goal states
            xreward = 0.1
        elif sample.x[1] == 4:  #intermediate goal states
            xreward = 0.1
        elif sample.x[1] == 5:  #intermediate goal states
            xreward = 1.0
        elif sample.x[1] == 6:  #intermediate goal states
            xreward = 1.0
        #prompting carries a small cost
        if sample.get_turn()=="agent" and not action[1] == 0:
            xreward = xreward - 0.05

        #larger if out of turn
        #this should really be handled probabilistically I think 
        #so acting out of turn really messes things up 
        if sample.get_turn() == "client" and not action[1] == 0:
            xreward = xreward - 10.0
        return xreward



    #----------------------------------------------------------------------------------
    #POMCP functions
    #----------------------------------------------------------------------------------
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
        if obs1[1][0] == obs2[1][0] and obs1[1][1]==obs2[1][1]:
            return math.sqrt(raw_dist(obs1[0],obs2[0]))
        else:
            return -2

