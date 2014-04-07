#include "tracker.hpp"

int main(int argc, char** argv) {
	// argv[1] = server_addr
	// argv[2] = projectpath
	TrackerServerStub server_stub(argv[argc - 1]);
	tracker_start(argc, argv, &server_stub);
}
