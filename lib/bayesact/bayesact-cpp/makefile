CC=g++
CFLAGS=-Wall
LDFLAGS=
SOURCES=main.cpp epa.cpp agent.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=bayesact

all: settings.h 

#all: $(SOURCE)
#g++ -I /usr/include/python2.7 -fpic *.cpp -lpython2.7 -lboost_python -o bayesact_cpp.o 
	
#$(EXECUTABLE): $(OBJECTS) 
#	$(CC) $(LDFLAGS) $(OBJECTS) -o $(OBJECTS)

settings.h: settingsGen settings.csv
	./$< settings.csv -o settings.h

settingsGen: settings-parser/settingsheader.cpp settings-parser/variablename.cpp settings-parser/main.cpp
	$(CC) $(CFLAGS) $^ -o $@
	chmod a+x $@
