#include "hand_tracker.hpp"

int main(int argc, char** argv) {
	// argv[1] = server_addr
	// argv[2] = projectpath
	HandTrackerServerStub server_stub(argv[argc - 1]);
	hand_tracker_start(argc, argv, &server_stub);
}
