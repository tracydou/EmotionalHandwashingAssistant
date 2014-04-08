# Makefile for building the files together
EXE_EHWA = EHwA
OBJS = main.o bayesact_client.o bayesact_message.pb.o epa_calculator.o \
prompt_selecter.o tracker_client.o ../lib/hand_tracker/hand_tracker_message.pb.o
HDRS = bayesact_client.hpp bayesact_message.pb.h EPACalculator/epa_calculator.hpp \
prompt_selecter.hpp defines.hpp tracker_client.hpp ../lib/hand_tracker/hand_tracker_message.pb.h
LIBS = -lzmq -lprotobuf
CC = g++
PBC = protoc
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG) $(LIBS) -I.
LFLAGS = -Wall $(DEBUG)

$(EXE_EHWA) : bayesact_proto handtracker_proto $(OBJS)
	$(CC) -o $(EXE_EHWA) $(LFLAGS) $(OBJS) $(LIBS)

main.o : $(HDRS) main.cpp
	$(CC) $(CFLAGS) main.cpp

bayesact_message.pb.o : bayesact_message.pb.h bayesact_message.pb.cc
	$(CC) $(CFLAGS) bayesact_message.pb.cc

bayesact_client.o : bayesact_client.hpp bayesact_message.pb.h bayesact_client.cpp defines.hpp
	$(CC) $(CFLAGS) bayesact_client.cpp

epa_calculator.o : EPACalculator/epa_calculator.hpp defines.hpp EPACalculator/epa_calculator.cpp 
	$(CC) $(CFLAGS) EPACalculator/epa_calculator.cpp 

tracker_client.o : tracker_client.hpp defines.hpp ../lib/hand_tracker/hand_tracker_message.pb.h tracker_client.cpp
	$(CC) $(CFLAGS) tracker_client.cpp ../lib/hand_tracker/hand_tracker_message.pb.cc
	
prompt_selecter.o : prompt_selecter.hpp prompt_selecter.cpp
	$(CC) $(CFLAGS) prompt_selecter.cpp
	
bayesact_proto : bayesact_message.proto
	$(PBC) bayesact_message.proto --cpp_out=. --python_out=.
	
handtracker_proto : ../lib/hand_tracker/hand_tracker_message.proto
	$(PBC) ../lib/hand_tracker/hand_tracker_message.proto --proto_path=../lib/hand_tracker/ --cpp_out=../lib/hand_tracker/

clean :
	\rm $(EXE_EHWA) $(OBJS)
