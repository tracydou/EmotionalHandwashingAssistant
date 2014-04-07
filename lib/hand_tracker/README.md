#Dependency packages needed to be installed to run DTreeTraining
sudo apt-get install libopencv-dev

#Before "mkdir build", "cd build", "cmake .." & "make",
#one first needs to run: 
protoc hand_tracker_message.proto --cpp_out=.
