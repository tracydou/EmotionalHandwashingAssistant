#!/bin/sh
g++ -Wall -c -lzmq -lprotobuf -I. sample_server.cpp BayesActMessage.pb.h BayesActMessage.pb.cc
g++ -o sample_server sample_server.o BayesActMessage.pb.o -lzmq -lprotobuf
./sample_server
