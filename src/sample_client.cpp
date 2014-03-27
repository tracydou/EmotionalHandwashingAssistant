//  Hello World client
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>

#include "BayesActMessage.pb.h"
using namespace EHwA;
using std::string;

int main (void)
{
    printf ("Connecting to hello world server…\n");
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:5555");

    while (true) {
        printf ("Sending Hello\n");
        zmq_send (requester, "Hello", 5, 0);
        
 
        BayesActRespond message;
        printf("default respondMessage = %s", message.DebugString().c_str());
        string message_str;
        char buffer[2048];
        zmq_recv (requester, buffer, 2048, 0);
        message_str.assign(buffer, 2048);
        
        if (!message.ParseFromString(message_str)) {
			printf("Parse failed\n");
		}
        printf ("Received message = %s\n", message.DebugString().c_str());
 /*
		char buffer[1024];
		zmq_recv (requester, buffer, 1024, 0);
		std::istream input(&buffer);
		
		BayesActRespond message;
		message.ParseFromIstream(&input);
*/		printf("===================================");
    }
    zmq_close (requester);
    zmq_ctx_destroy (context);
    return 0;
}

