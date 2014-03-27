import time
import zmq
import BayesActMessage_pb2

context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://*:5555")

while True:
    #  Wait for next request from client
    message = socket.recv()

    request = BayesActMessage_pb2.BayesActRequest()
    request.ParseFromString(message)
    
    print("Received request: %s" % request.__str__())

    #  Do some 'work'
    time.sleep(1)
    
    response = BayesActMessage_pb2.BayesActResponse()
    response.evaluation = request.evaluation
    response.potency = request.potency
    response.activity = request.activity

    #  Send reply back to client
    socket.send(response.SerializeToString())
