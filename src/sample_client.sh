#!/bin/sh
g++ -Wall -c -lzmq -lprotobuf -I. sample_client.cpp BayesActMessage.pb.h BayesActMessage.pb.cc
g++ -o sample_client sample_client.o BayesActMessage.pb.o -lzmq -lprotobuf
./sample_client 
