#include "hand_tracker.hpp"

int main(int argc, char** argv) {
	// argv[last] = server_addr
	// argv[1, 2] = projectpath
	HandTrackerServerStub server_stub(argv[argc - 1]);
	// hand_tracker_start(argc, argv, &server_stub);
	fake_hand_tracker_start(argc, argv, &server_stub);
}
