#include "tracker.hpp"

int main(int argc, char** argv) {
	// argv[1] = projectpath
	// argv[2] = server_addr
	TrackerServerStub* server_stub = new TrackerServerStub(argv[1]);
	tracker_start(argc, argv, &server_stub);
	delete server_stub;
}
