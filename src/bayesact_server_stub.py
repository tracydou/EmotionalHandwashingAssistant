import os
import sys
import time
import zmq

import bayesact_message_pb2

sys.path.append('../lib/bayesact/')
sys.path.append("../lib/bayesact/gui/")
from bayesactlib import Bayesact

os.chdir('../lib/bayesact/')


def server_stub(address_string):
	context = zmq.Context()
	socket = context.socket(zmq.REP)
	socket.bind(address_string.__str__())
	print("BayesActServer started!")

	bayesact = Bayesact()
	while True:
		print("Waiting for request...\n")
		#  Wait for next request from client
		request_message = socket.recv()
		request = bayesact_message_pb2.BayesactRequest()
		request.ParseFromString(request_message)
		print "Request = \n", request.__str__()

		#  Do some 'work'
		response = bayesact_message_pb2.BayesactResponse()
		#response.evaluation = 2
		#response.potency = 2
		#response.activity = 2
		#response.prompt = 2
		(result_epa, result_prompt) = bayesact.calculate([request.evaluation, request.potency, request.activity], request.hand_action)
		response.evaluation = float(result_epa[0])
		response.potency = float(result_epa[1])
		response.activity = float(result_epa[2])
		response.prompt = int(result_prompt)
		print "result_epa=", result_epa
		print "result_prompt=", result_prompt
		
		
		#  Send reply back to client
		print "Response = \n", response.__str__()
		socket.send_string(response.__str__())
		print "=============================="

if __name__ == "__main__":
	server_stub(sys.argv[-1])

