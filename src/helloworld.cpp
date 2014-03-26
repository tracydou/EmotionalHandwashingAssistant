// HelloWorld.cpp

#include <iostream>
#include <stdlib.h>
using namespace std;

int main() {
  cout << "Hello World for learning zeromq!" << endl;
  
  int pid=fork();
  if (pid < 0 ) { // failed to fork
    cout << "failed to fork child process!" << endl;  
    return -1;
  } else if (pid == 0) { // child process
    // change directory and start the BayesactEngine server
    chdir("../lib/BayesactEngine");
    system("python ./bayesactinteractive.py");
  } else { // parent process
    // change back to directory
    // and connect the BayesActClient client & server
    while(true) {
	  cout << "done!" << endl;
    }
    return 0;
  }
}
