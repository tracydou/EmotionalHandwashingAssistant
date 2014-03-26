#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

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
        printf ("Received Hello\n");
        sleep (1);          //  Do some 'work'
        printf("buffer[0-2] = %c, %c, %c\n", buffer[0], buffer[1], buffer[2]);
        zmq_send (responder, "World", 3, 0);
    }
    return 0;
}
