#include "tracker.hpp"

int main(int argc, char** argv) {
	int pid = fork();
	if (pid ==0) {
		main_start(argc, argv);
	} else {
		while(true) {
			printf("====================in fork!\n");
			getLeftHandPos();
			getRightHandPos();
		}
	}
	return 0;
}
	
