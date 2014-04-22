import os
import sys
import time
import zmq

import bayesact_message_pb2

sys.path.append('../lib/bayesact/')
sys.path.append("../lib/bayesact/gui/")
#from bayesactlib import BayesactAssistant
from bayesactlib_unstable import BayesactAssistant

os.chdir('../lib/bayesact/')


def server_stub(address_string):
	context = zmq.Context()
	socket = context.socket(zmq.REP)
	socket.bind(address_string.__str__())
	print("BayesActServer started!")

	bayesact = BayesactAssistant()
	while True:
		print("Waiting for request...\n")
		#  Wait for next request from client
		request_message = socket.recv()
		request = bayesact_message_pb2.BayesactRequest()
		request.ParseFromString(request_message)
		print "Request = \n", request.__str__()

		#  Do some 'work'
		response = bayesact_message_pb2.BayesactResponse()
		(result_epa, result_prompt) = bayesact.calculate([request.evaluation, request.potency, request.activity], request.hand_action)
		response.evaluation = float(result_epa[0])
		response.potency = float(result_epa[1])
		response.activity = float(result_epa[2])
		response.prompt = int(result_prompt)
		
		#  Send reply back to client
		response_message = response.SerializeToString()
		print "Response = \n", response.__str__()
		socket.send(response_message)
		print "=============================="
		
def bayesact_tester():
	print("BayesActServer tester started!")

	bayesact = BayesactAssistant()
	done = False
	while not done:
		print("using faked epa<0,0,0> for testing...\n")
		#  Test and print out result
		(result_epa, result_prompt) = bayesact.calculate([0,0,0], 0)
		print "result_epa: <",result_epa[0], ", ",result_epa[1], ", ", result_epa[2], ">\n"
		print "result_prompt: ", result_prompt 
		print "=============================="
		done = True

if __name__ == "__main__":
#	server_stub(sys.argv[-1])
    bayesact_tester()

