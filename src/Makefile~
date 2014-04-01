# Makefile for building the files together
EXE = EHwA
OBJS = main.o BayesActClient.o bayesact_message.pb.o EPACalculator.o FrameAnalyzer.o PromptSelecter.o
LIBS = -lzmq -lprotobuf
CC = g++
PBC = protoc
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG) $(LIBS) -I.
LFLAGS = -Wall $(DEBUG)

$(EXE) : proto $(OBJS)
	$(CC) -o $(EXE) $(LFLAGS) $(OBJS) $(LIBS)

main.o : *.h *.hpp EPACalculator/EPACalculator.hpp main.cpp
	$(CC) $(CFLAGS) main.cpp

bayesact_message.pb.o : bayesact_message.pb.h bayesact_message.pb.cc
	$(CC) $(CFLAGS) bayesact_message.pb.cc

BayesActClient.o : BayesActClient.hpp bayesact_message.pb.h BayesActClient.cpp defines.hpp
	$(CC) $(CFLAGS) BayesActClient.cpp

EPACalculator.o : EPACalculator/EPACalculator.hpp defines.hpp EPACalculator/EPACalculator.cpp 
	$(CC) $(CFLAGS) EPACalculator/EPACalculator.cpp 

FrameAnalyzer.o : FrameAnalyzer.hpp defines.hpp FrameAnalyzer.cpp
	$(CC) $(CFLAGS) FrameAnalyzer.cpp
	
PromptSelecter.o : PromptSelecter.hpp PromptSelecter.cpp
	$(CC) $(CFLAGS) PromptSelecter.cpp
	
proto : bayesact_message.proto
	$(PBC) bayesact_message.proto --cpp_out=. --python_out=.

clean :
	\rm *.o *.gch
