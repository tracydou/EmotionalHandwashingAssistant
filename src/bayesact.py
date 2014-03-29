import bayesact_message_pb2
import sys
import time
import zmq

def server_stub(address_string):
	context = zmq.Context()
	socket = context.socket(zmq.REP)
	socket.bind(address_string.__str__())
	print("BayesActServer started!")

	while True:
		print("Waiting for request...\n")
		#  Wait for next request from client
		request = socket.recv()
		request_message = bayesact_message_pb2.BayesActRequest()
		request_message.ParseFromString(request)
		print "Request = \n", request_message.__str__()

		#  Do some 'work'
		respond_message = bayesact_message_pb2.BayesActRespond()
		respond_message.evaluation = 2
		respond_message.potency = 2
		respond_message.activity = 2
		respond_message.prompt = 2

		#  Send reply back to client
		respond = respond_message.SerializeToString()
		print "Respond = \n", respond_message.__str__()
		socket.send_string(respond)
		print "=============================="

if __name__ == "__main__":
	server_stub(sys.argv[-1])

