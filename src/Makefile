# Makefile for building the files together
EXE = EHwA
OBJS = main.o BayesActClient.o BayesActMessage.pb.o EPACalculator.o FrameAnalyzer.o PromptSelecter.o
LIBS = -lzmq -lprotobuf
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG) $(LIBS) -I.
LFLAGS = -Wall $(DEBUG)

$(EXE) : $(OBJS)
	$(CC) -o $(EXE) $(LFLAGS) $(OBJS) $(LIBS)

main.o : *.h *.hpp EPACalculator/EPACalculator.hpp main.cpp
	$(CC) $(CFLAGS) main.cpp

BayesActMessage.pb.o : BayesActMessage.pb.h BayesActMessage.pb.cc
	$(CC) $(CFLAGS) BayesActMessage.pb.cc

BayesActClient.o : BayesActClient.hpp BayesActMessage.pb.h BayesActClient.cpp
	$(CC) $(CFLAGS) BayesActClient.cpp

EPACalculator.o : EPACalculator/EPACalculator.hpp defines.hpp EPACalculator/EPACalculator.cpp 
	$(CC) $(CFLAGS) EPACalculator/EPACalculator.cpp 

FrameAnalyzer.o : FrameAnalyzer.hpp defines.hpp FrameAnalyzer.cpp
	$(CC) $(CFLAGS) FrameAnalyzer.cpp
	
PromptSelecter.o : PromptSelecter.hpp PromptSelecter.cpp
	$(CC) $(CFLAGS) PromptSelecter.cpp
	
clean :
	\rm *.o  
