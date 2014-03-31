# Makefile for building the files together
EXE = EHwA
OBJS = main.o bayesact_client.o bayesact_message.pb.o epa_calculator.o frame_analyzer.o prompt_selecter.o
HDRS = bayesact_client.hpp bayesact_message.pb.h EPACalculator/epa_calculator.hpp frame_analyzer.hpp prompt_selecter.hpp defines.hpp
LIBS = -lzmq -lprotobuf
CC = g++
PBC = protoc
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG) $(LIBS) -I.
LFLAGS = -Wall $(DEBUG)

$(EXE) : proto $(OBJS)
	$(CC) -o $(EXE) $(LFLAGS) $(OBJS) $(LIBS)

main.o : $(HDRS) main.cpp
	$(CC) $(CFLAGS) main.cpp

bayesact_message.pb.o : bayesact_message.pb.h bayesact_message.pb.cc
	$(CC) $(CFLAGS) bayesact_message.pb.cc

bayesact_client.o : bayesact_client.hpp bayesact_message.pb.h bayesact_client.cpp defines.hpp
	$(CC) $(CFLAGS) bayesact_client.cpp

epa_calculator.o : EPACalculator/epa_calculator.hpp defines.hpp EPACalculator/epa_calculator.cpp 
	$(CC) $(CFLAGS) EPACalculator/epa_calculator.cpp 

frame_analyzer.o : frame_analyzer.hpp defines.hpp frame_analyzer.cpp
	$(CC) $(CFLAGS) frame_analyzer.cpp
	
prompt_selecter.o : prompt_selecter.hpp prompt_selecter.cpp
	$(CC) $(CFLAGS) prompt_selecter.cpp
	
proto : bayesact_message.proto
	$(PBC) bayesact_message.proto --cpp_out=. --python_out=.

clean :
	\rm $(EXE) $(OBJS)
