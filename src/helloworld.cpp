// HelloWorld.cpp

#include <iostream>
#include <stdlib.h>

#include "sample_client_server.cpp"
using namespace std;

int main() {
  cout << "Hello World for learning zeromq!" << endl;
  Server server;
  Client client;
  int pid=fork();
  if (pid < 0 ) { // failed to fork
    cout << "failed to fork child process!" << endl;  
    return -1;
  } else if (pid == 0) { // child process
    // change directory and start the BayesactEngine server
    // chdir("../lib/BayesactEngine");
    // system("python ./bayesactinteractive.py");
    server.Start();
  } else { // parent process
    // change back to directory
    // and connect the BayesActClient client & server
    client.Start();
  }
  return 0;
}
