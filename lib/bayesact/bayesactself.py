"""------------------------------------------------------------------------------------------
Bayesian Affect Control Theory
Bayesact of Self model
Author: Jesse Hoey  jhoey@cs.uwaterloo.ca   http://www.cs.uwaterloo.ca/~jhoey
June 2014
Use for research purposes only.
Please do not re-distribute without written permission from the author
Any commerical uses strictly forbidden.
Code is provided without any guarantees.
Research sponsored by the Natural Sciences and Engineering Council of Canada (NSERC).
see README for details
----------------------------------------------------------------------------------------------"""

import os
import math
import sys
import re
import copy
import numpy as NP
import itertools
import time
import random
from bayesact import *

#David Huard wrote this - found it on scipy.org
#compare two beliefs using kldivergence
def kldivergence(x, y):
    """Compute the Kullback-Leibler divergence between two multivariate samples.
    
    Parameters
    ----------
    x : 2D array (n,d)
    Samples from distribution P, which typically represents the true
    distribution.
    y : 2D array (m,d)
    Samples from distribution Q, which typically represents the approximate
    distribution.
    
    Returns
    -------
    out : float
    The estimated Kullback-Leibler divergence D(P||Q).
    
    References
    ----------
    Perez-Cruz, F. Kullback-Leibler divergence estimation of
    continuous distributions IEEE International Symposium on Information
    Theory, 2008.
    """
    from scipy.spatial import cKDTree as KDTree
    
    # Check the dimensions are consistent
    x = NP.atleast_2d(x)
    y = NP.atleast_2d(y)
    
    n,d = x.shape
    m,dy = y.shape
    
    assert(d == dy)
    
    
    # Build a KD tree representation of the samples and find the nearest neighbour
    # of each point in x.
    xtree = KDTree(x)
    ytree = KDTree(y)
    
    # Get the first two nearest neighbours for x, since the closest one is the
    # sample itself.
    r = xtree.query(x, k=2, eps=.01, p=2)[0][:,1]
    s = ytree.query(x, k=1, eps=.01, p=2)[0]
    
    print r
    print s
    # There is a mistake in the paper. In Eq. 14, the right side  misses a negative sign
    # on the first term of the right hand side.
    return -NP.log(r/s).sum() * d / n + NP.log(m / (n - 1.))
    
class Self(object):
    #---------------------------------------------------------------------------
    #constructor
    #---------------------------------------------------------------------------
    def __init__(self,*args,**kwargs):
        #rate at which the situational self-sentiment persists over time
        #(1-eta_value) is the learning rate
        self.eta_value=kwargs.get("eta_value",0.99)
        
        self.N_fsb = kwargs.get("n_fsb",1000)
        self.N_ssb = kwargs.get("n_ssb",1000)
        
        


    #compares two belief states using EMD measure
    def compareBeliefs(self,b1,b2):
        beliefDissimilarity = 0.0
        if len(b1)==0 or len(b2)==0:
            return

        #loop over dimensions
        biggestDiff = -1.0
        for d in range(len(b1[0])):
            b1d = sorted(map(lambda x: x[d],b1))
            b2d = sorted(map(lambda x: x[d],b2))
            j1=0
            j2=0
            for b in b1d:
                while j2 < len(b2d) and b2d[j2] < b:
                    j2 = j2 + 1
                j1 = j1 + 1 
                biggestDiff = max(biggestDiff,abs(j2-j1))
        return 1.0*biggestDiff/max(len(b1),len(b2))


        


    #simulates an interaction of Agent with a simulated client with identity fubc.
    #if the known_client flag is set, then do a simulation like in bayesactsim
    #between two agents with identites fuba and fubc
    #if the known_client flag is not set (default), then do a simulation of a single agent
    #who always gets a [] observation of fub
    def simulate_alter(self,ssb,eta,T,num_samples,alter,clientAlter,known_client=False):
        
        local_ssb = copy.deepcopy(ssb)

        alpha = 1.5
        gamma = 1.0
        env_noise=0.0
        #this roughening noise disrupts things siginficantly, but 
        #probably not for the worse
        roughening_noise=num_samples**(-1.0/3.0)
        #otherwise, to get really obvious results, use no roughening noise, but 
        #then the identities are not as flexible.
        #roughening_noise=0.0
        fifname="fidentities.dat"

        if known_client:
            agent_knowledge=2
            client_knowledge=2
            
            simul_agent=Agent(N=num_samples,alpha_value=alpha,gamma_value=gamma,beta_value_agent=0.005,beta_value_client=0.005,agent_rough=roughening_noise,client_rough=roughening_noise,identities_file=fifname)
            simul_avgs = simul_agent.set_samples(clientAlter)
        else:
            agent_knowledge=0

        learn_agent=Agent(N=num_samples,alpha_value=alpha,gamma_value=gamma,beta_value_agent=0.005,beta_value_client=0.005,agent_rough=roughening_noise,client_rough=roughening_noise,identities_file=fifname)

        learn_avgs=learn_agent.set_samples(alter)


        #have to pass this in as argument
        learn_turn="agent"
        simul_turn="client"  

        for t in range(T):
            

            print "learner turn: ",learn_turn
            print "simulator turn: ",simul_turn

            observ=[]
            print 10*"-d","iter ",t,80*"-"
            
            #like this:
            (learn_aab,learn_paab)=learn_agent.get_next_action(learn_avgs)
            print "agent action/client observ: ",learn_aab
            simul_observ=learn_aab
            
            if known_client:
                (simul_aab,simul_paab)=simul_agent.get_next_action(simul_avgs)
                print "client action/agent observ: ",simul_aab
                learn_observ=simul_aab
            else:
                learn_observ=[]
            
            #add environmental noise here if it is being used
            if env_noise>0.0:
                learn_observ=map(lambda fv: NP.random.normal(fv,env_noise), learn_observ)
                if known_client:
                    simul_observ=map(lambda fv: NP.random.normal(fv,env_noise), simul_observ)
            
            print "learn observ: ",learn_observ
            print "simul observ: ",simul_observ
            
            learn_xobserv=[State.turnnames.index(invert_turn(learn_turn))]
            simul_xobserv=[State.turnnames.index(invert_turn(simul_turn))]

            learn_avgs=learn_agent.propagate_forward(learn_aab,learn_observ,learn_xobserv)
            
            if known_client:
                simul_avgs=simul_agent.propagate_forward(simul_aab,simul_observ,simul_xobserv)


            print "learner f is: "
            learn_avgs.print_val()
            
            if known_client:
                print "simulator f is: "
                simul_avgs.print_val()
            
            #I think these should be based on fundamentals, not transients
            (aid,cid)=learn_agent.get_avg_ids(learn_avgs.f)
            print "learner agent id:",aid
            print "learner client id:",cid
            
            if known_client:
                (aid,cid)=simul_agent.get_avg_ids(simul_avgs.f)
                print "simul agent id:",aid
                print "simul client id:",cid
                    
            
            #resample and update ssb
            for samplei in range(len(local_ssb)):
                if NP.random.random()>eta:  #replace the sample with one from fuba
                    #might need a deep copy here
                    newf = random.choice(learn_agent.samples).get_copy().f[0:3]
                    local_ssb[samplei] = newf

            learn_turn = invert_turn(learn_turn)
            simul_turn = invert_turn(simul_turn)

    
        (cnt_ags,cnt_cls)=learn_agent.get_all_ids()
        print "top ten ids for agent (learner perspective):"
        print cnt_ags[0:10]
        print "top ten ids for client (learner perspective):"
        print cnt_cls[0:10]
        if known_client:
            (cnt_ags,cnt_cls)=simul_agent.get_all_ids()
            print "top ten ids for agent (simulator perspective):"
            print cnt_ags[0:10]
            print "top ten ids for client (simulator perspective):"
            print cnt_cls[0:10]
        


        return (learn_avgs,local_ssb)

                    
        
        
        
print 20*"TESTING---"
me  = Self(eta_value=0.01)
        
#really tiny example
b1=[[0.1,0.2,0.3],[0.2,0.1,0.3],[0.9,0.8,0.7]]
b2=[[-0.2,0.1,0.9],[0.7,-0.3,-0.4],[1.2,0.3,0.1]]

bdiff = me.compareBeliefs(b1,b2)
print "bdiff = ",bdiff

kldiff = kldivergence(b1,b2)
print "kldiff = ",kldiff


#set up some 3D means and covariances
cov1=[[1.0,0,0],[0.0,0.9,0.0],[0.0,0.1,0.1]]
mean1=[2.0,2.5,-0.5]

cov2=[[1.0,0,0],[0.0,0.9,0.0],[0.0,0.1,0.1]]
mean2=[-2.0,-2.5,-0.5]

cov3=[[1.0,0,0],[0.0,0.9,0.0],[0.0,0.1,0.1]]
mean3=[2.0,-2.5,-0.5]

cov4=[[1.0,0,0],[0.0,0.9,0.0],[0.0,0.1,0.1]]
mean4=[-2.0,2.5,-0.5]


b1=[]
b2=[]
b3=[]
b4=[]
b5=[]
for i in range(100):
    b1.append(NP.random.multivariate_normal(mean1,cov1))
    b3.append(NP.random.multivariate_normal(mean1,cov1))
    b4.append(NP.random.multivariate_normal(mean2,cov2))
    if NP.random.random() < 0.5:
        b2.append(NP.random.multivariate_normal(mean2,cov2))
    else:
        b2.append(NP.random.multivariate_normal(mean1,cov1))

    if NP.random.random() < 0.5:
        b5.append(NP.random.multivariate_normal(mean4,cov4))
    else:
        b5.append(NP.random.multivariate_normal(mean3,cov3))

start = time.time()
print "b1,b3: ",me.compareBeliefs(b1,b3)
print "b1,b2: ",me.compareBeliefs(b1,b2)
print "b3,b2: ",me.compareBeliefs(b3,b2)
print "b4,b2: ",me.compareBeliefs(b4,b2)
print "b1,b4: ",me.compareBeliefs(b1,b4)

#b2 and b5 should be really different, but this measure will be small
print "b2,b5: ",me.compareBeliefs(b2,b5)
print 20*"t","time : ",time.time()  - start

print 20*"-","KL divergence"
start = time.time()
print "b1,b3: ",kldivergence(b1,b3)
print "b1,b2: ",kldivergence(b1,b2)
print "b3,b2: ",kldivergence(b3,b2)
print "b4,b2: ",kldivergence(b4,b2)
print "b1,b4: ",kldivergence(b1,b4)

#b2 and b5 should be really different, and this measure is correct
print "b2,b5: ",kldivergence(b2,b5)
print 20*"t","time : ",time.time()  - start

        

fifname="fidentities.dat"              
agent_gender="male"

#thesamples = initialise_samples(100,fifname,[[0.5,"female",0.1,agent_gender],[0.5,"employer",0.1,agent_gender]],[[1.0,"employee",0.1,agent_gender]])
#for sample in thesamples:
#    sample.print_val()


#get the fundamental self sentiment
n_fsb=100
thesamples = initialise_samples(n_fsb,fifname,agent_gender,[[0.5,"female",0.01],[0.5,"employer",0.01]],[],[1.0,0.0])

fsb = map(lambda x: x.f[0:3],thesamples)
#ssb will start the same as fsb, but with a small amount of added noise
ssb = map(lambda x: x.f[0:3] + NP.random.random([3])*0.01,thesamples)


print 10*"-fsb"
print fsb
print 10*"-ssb"
print ssb


#agent fundamental self sentiment
agent_ids = [[0.5,"female",0.01],[0.5,"employer",0.01]]

#people known initially
client_ids=["husband","employee"]
client_names = ["jerry","susan"]
client_genders = ["male","female"]
client_agent_ids = ["female","employer"]
client_variances=[0.01,0.01]

thealters={}
theclientalters={}
n=1000
for (cid,cname,cgen,caid,cvar) in zip(client_ids,client_names,client_genders,client_agent_ids,client_variances):
    thealters[cname] = initialise_samples(n,fifname,agent_gender,agent_ids,[[1.0,cid,cvar]],initx=[1.0,0.0])
    theclientalters[cname] = initialise_samples(n,fifname,cgen,[[1.0,cid,cvar]],[[1.0,caid,0.01]],initx=[0.0,1.0])   #same variance for all

#one stranger
thealters["stranger"] = initialise_samples(n,fifname,agent_gender,agent_ids,[],initx=[1.0,0.0])
theclientalters["stranger"]=[]
map(lambda x: x.print_val(),thealters["jerry"])
    
eta = 0.95
T=50
known_client=True
ssb_clients={}
avgs={}
for alter in thealters:
    if alter=="stranger":
        known_client=False
    else:
        known_client=True
    print 100*"="
    print "the alter: ",alter
    (avgs[alter],ssb_clients[alter]) = me.simulate_alter(ssb,eta,T,n,thealters[alter],theclientalters[alter],known_client)


print "fsb: "
print fsb
f = open('jerrysusan.m', 'a')
suffix="4"
for alter in thealters:
    #print 50*"^"
    #print "the alter: ",alter
    #print 50*"-"
    #print ssb_clients[alter]

    f.write(alter+suffix+"= [\n")
    for x in ssb_clients[alter]:
        for y in x:
            f.write(str(y)+" ")
        f.write(";\n")
    f.write("];\n")
f.close()
    


for alter in thealters:
    print 50*"^"
    print "the alter: ",alter
    print 50*"-"
    print ssb_clients[alter]
    print fsb
    avgs[alter].print_val()

    
    print "difference in fundamental and situational sentiments: ",kldivergence(ssb_clients[alter],fsb)
