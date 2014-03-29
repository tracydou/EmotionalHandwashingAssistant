import time
import zmq
import BayesActMessage_pb2

def serverStub(address_string):
	context = zmq.Context()
	socket = context.socket(zmq.REP)
	socket.bind(address_string.__str__())
	print("SimpleServer opened!")

	while True:
		print("waiting for client...")
		#  Wait for next request from client
		requestBuffer = socket.recv()
		requestMessage = BayesActMessage_pb2.BayesActRequest()
		requestMessage.ParseFromString(requestBuffer)
		print "LOG(info): GOT request = ", requestMessage.__str__()

		#  Do some 'work'		
		respondMessage = BayesActMessage_pb2.BayesActRespond()
		respondMessage.evaluation = 2
		respondMessage.potency = 2
		respondMessage.activity = 2
		respondMessage.prompt = 2

		#  Send reply back to client
		respondBuffer = respondMessage.SerializeToString()
		print "after serialization! repondBuffer = \n", respondBuffer
		socket.send_string(respondBuffer)
		print "\n=============================="

serverStub("tcp://*:5555")
