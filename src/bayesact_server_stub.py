import sys
import time
import zmq

import bayesact_message_pb2


def server_stub(address_string):
	context = zmq.Context()
	socket = context.socket(zmq.REP)
	socket.bind(address_string.__str__())
	print("BayesActServer started!")

	while True:
		print("Waiting for request...\n")
		#  Wait for next request from client
		request_message = socket.recv()
		request = bayesact_message_pb2.BayesactRequest()
		request.ParseFromString(request_message)
		print "Request = \n", request.__str__()

		#  Do some 'work'
		response = bayesact_message_pb2.BayesactResponse()
		response.evaluation = 2
		response.potency = 2
		response.activity = 2
		response.prompt = 2

		#  Send reply back to client
		response_message = response.SerializeToString()
		print "Response = \n", response.__str__()
		socket.send_string(response_message)
		print "=============================="

if __name__ == "__main__":
	server_stub(sys.argv[-1])

