#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <iostream>

#include "BayesActMessage.pb.h"
using namespace EHwA;
using std::string;

using std::cout;
using std::endl;

int main (void)
{
    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:5555");
    assert (rc == 0);

    while (1) {
        char buffer [20];
        zmq_recv (responder, buffer, 20, 0);
        printf ("Received %s\n", buffer);
        sleep (1);          
        
        //  Do some 'work'
        BayesActRespond message;
        message.set_evaluation(-1);
        message.set_potency(-1);
        message.set_activity(-1);
        message.set_prompt(3);
        
        printf("\nmessage = %s", message.DebugString().c_str());
        string str;
        message.SerializeToString(&str);
               
        printf("strlen = %lu\n", strlen(str.c_str()));
        cout << "str = " << str << endl;
        cout << "str.c_str() = " << str.c_str() << endl;
        printf("str.length() = %lu\n", string(str.c_str()).length());
               
        BayesActRespond message2;
        string str2(str.c_str(), str.length());
        message2.ParseFromString(str2);
        printf("message2 = %s lol\n", message2.DebugString().c_str());
        printf("=======================\n");    
        
        zmq_send (responder, (void*)str.c_str(), str.length(), 0);
        
 /*       std::ostream output;
        message.SerializeToOstream(&output);
        char buffer1[1024];
        output << buffer1;
        zmq_send(responder, (void*)buffer1, 1024, 0);
  */  }
    return 0;
}
