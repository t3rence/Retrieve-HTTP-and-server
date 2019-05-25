#!/bin/bash
g++ -std=c++11 Server.cpp -pthread -o server 
g++ -std=c++11 receiver.cpp -o receiver