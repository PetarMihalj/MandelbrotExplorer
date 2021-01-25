#!/bin/bash
g++ -o prozor -g prozor.cpp util.cpp -lGLEW -lGL -lGLU -lglut -lpthread
sleep 3
./prozor
    
