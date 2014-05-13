import os
import sys
import time
import zmq
from array import *

import bayesact_message_pb2

sys.path.append('../lib/bayesact/')
sys.path.append("../lib/bayesact/gui/")
from bayesactlib import BayesactAssistant

os.chdir('../lib/bayesact/')


def server_stub(address_string):
	context = zmq.Context()
	socket = context.socket(zmq.REP)
	socket.bind(address_string.__str__())
	print("BayesActServer started!")

	bayesact = BayesactAssistant()
	finished = False
	while not finished:
		print("[log] BaysActServer: Waiting for request...\n")
		#  Wait for next request from client
		request_message = socket.recv()
		request = bayesact_message_pb2.BayesactRequest()
		request.ParseFromString(request_message)
		print "[log] BaysActServer: Request received = \n", request.__str__()

		#  Do some 'work'
		response = bayesact_message_pb2.BayesactResponse()
		(finished, result_epa, result_prompt) = bayesact.calculate([request.evaluation, request.potency, request.activity], request.hand_action)
		response.evaluation = float(result_epa[0])
		response.potency = float(result_epa[1])
		response.activity = float(result_epa[2])
		response.prompt = int(result_prompt)
		response.is_done = bool(finished)
		
		#  Send reply back to client
		response_message = response.SerializeToString()
		print "[log] BayesActServer: Response = \n", response.__str__()
		socket.send(response_message)
		print "==============================\n"
		
	print "Hoorey! You complished HandWashing Task finished!"
	print "==============================\n"
		
# call this only when to test bayesact.calculate()
def bayesact_tester():
	print("BayesActServer tester started!")
	finished = False  
	result_epa = [0,0,0]
	result_prompt = 0
	bayesact = BayesactAssistant()
	while not finished:
		#  Test and print out result
		(finished, result_epa, result_prompt) = bayesact.calculate(result_epa, result_prompt)
		print "=============================="
		print "result_epa: <",result_epa[0], ", ",result_epa[1], ", ", result_epa[2], ">\n"
		print "result_prompt: ", result_prompt 
		print "=============================="

if __name__ == "__main__":
	server_stub(sys.argv[-1])
#    bayesact_tester()

