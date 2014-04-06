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
  execuatable = '/home/basement/workspace/DTreeTraining/build/TRAIN'
  workspacepath = '/home/basement/workspace/DTreeTraining/'
  gain = params['GAIN']
  offset = params['OFFSET']
  depth = params['DEPTH']
  
  cmd = [execuatable, '-projectpath', workspacepath, '-gain', str(gain[0]), '-offset', str(offset[0]), '-depth', str(depth[0]), '-spearmint', '1']

  print cmd

  p = Popen(cmd , shell=0, stdout=PIPE, stderr=PIPE)
  #read buffer until program terminates
  out, err = p.communicate()
 
  print out.rstrip(), err.rstrip()
  
  m = re.search('(?<=Priority_Recall: )(\d+.\d+)', out)
  result = float(m.group(0))
  return 1-result