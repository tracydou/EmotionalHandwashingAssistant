import numpy as np
import sys
import math
import time

# Write a function like this called 'main'
def main(job_id, params):
# print 'Anything printed here will end up in the output directory for job #:', str(job_id)
  print params
 
  from subprocess import Popen, PIPE
  import re
  executable = '/home/basement/workspace/DTreeTraining/build/TRAIN'
  workspacepath = '/home/basement/workspace/DTreeTraining'
  band = params['BANDWIDTH']
  thresh = params['PROB_THRESH']
  
  #cmd = [executable, '-projectpath', workspacepath, '-band', str(band[0]), '-prob_thresh', str(thresh[0]), '-training', 2]
  cmd = [executable, '-projectpath', workspacepath, '-band', str(band[0]), '-part', '2', '-prob_thresh', str(thresh[0]), '-spearmint', '2']
  print cmd

  p = Popen(cmd, shell=False, stdout=PIPE, stderr=PIPE)
  
  print "testing testing"
  #read buffer until program terminates
  out, err = p.communicate()
 
  print out.rstrip(), err.rstrip()
  
  m = re.search('(?<=classError: )(\d+.\d+)', out)
  result = float(m.group(0))
  print result
  return result