# Makefile for building the files together
EXE_EHWA = EHwA

OBJS = main.o bayesact_client.o bayesact_message.pb.o buffer.o epa_calculator.o defines.o \
prompt_selecter.o tracker_client.o prompt_player.o hand_tracker_message.pb.o

HDRS = bayesact_client.hpp bayesact_message.pb.h buffer.hpp EPACalculator/epa_calculator.hpp \
prompt_selecter.hpp defines.hpp tracker_client.hpp ../lib/hand_tracker/hand_tracker_message.pb.h \
prompt_player.hpp

LIBS = -lzmq -lprotobuf -lvlc -lpthread
CC = g++
PBC = protoc
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG) $(LIBS) -I. -std=c++0x 
LFLAGS = -Wall $(DEBUG)

$(EXE_EHWA) : bayesact_proto handtracker_proto $(OBJS)
	$(CC) -o $(EXE_EHWA) $(LFLAGS) $(OBJS) $(LIBS)

main.o : $(HDRS) main.cpp
	$(CC) $(CFLAGS) main.cpp

bayesact_message.pb.o : bayesact_message.pb.h bayesact_message.pb.cc
	$(CC) $(CFLAGS) bayesact_message.pb.cc

bayesact_client.o : bayesact_client.hpp bayesact_message.pb.h bayesact_client.cpp defines.hpp
	$(CC) $(CFLAGS) bayesact_client.cpp defines.cpp

buffer.o : buffer.hpp EPACalculator/epa_calculator.hpp buffer.cpp EPACalculator/epa_calculator.cpp  
	$(CC) $(CFLAGS) buffer.cpp EPACalculator/epa_calculator.cpp 

epa_calculator.o : EPACalculator/epa_calculator.hpp defines.hpp EPACalculator/epa_calculator.cpp 
	$(CC) $(CFLAGS) EPACalculator/epa_calculator.cpp defines.cpp

defines.o : defines.hpp defines.cpp
	$(CC) $(CFLAGS) defines.cpp

tracker_client.o : tracker_client.hpp defines.hpp ../lib/hand_tracker/hand_tracker_message.pb.h tracker_client.cpp
	$(CC) $(CFLAGS) tracker_client.cpp defines.cpp ../lib/hand_tracker/hand_tracker_message.pb.cc
	
prompt_selecter.o : prompt_selecter.hpp ../lib/csv_parser/csv_v3.h prompt_selecter.cpp
	$(CC) $(CFLAGS) prompt_selecter.cpp

prompt_player.o : prompt_player.hpp prompt_player.cpp
	$(CC) $(CFLAGS) prompt_player.cpp
	
bayesact_proto : bayesact_message.proto
	$(PBC) bayesact_message.proto --cpp_out=. --python_out=.
	
handtracker_proto : ../lib/hand_tracker/hand_tracker_message.proto
	$(PBC) ../lib/hand_tracker/hand_tracker_message.proto --proto_path=../lib/hand_tracker/ --cpp_out=../lib/hand_tracker/

clean :
	\rm $(EXE_EHWA) $(OBJS) bayesact_message.pb.* bayesact_message_pb2.* EPACalculator/*.o
