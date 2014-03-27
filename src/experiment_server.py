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
		request = socket.recv()
		print "GOT", request

		#  Do some 'work'
		time.sleep(1)
		
		response = "world"

		#  Send reply back to client
		socket.send(response)

serverStub("tcp://*:5555")
