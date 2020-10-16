# Makefile for Writing Make Files Example

# *****************************************************
# Variables to control Makefile operation

CXX = g++
CXXFLAGS = -std=c++11

# ****************************************************
# Targets needed to bring the executable up to date

main: main.o Assembler.o DataConverter.o Memory.o Data.o CPU.o Register.o
	$(CXX) $(CXXFLAGS) -o main main.o Assembler.o DataConverter.o Memory.o Data.o CPU.o Register.o

# The main.o target can be written more simply

main.o: main.cpp DataConverter.h Assembler.h Memory.h
	$(CXX) $(CXXFLAGS) -c main.cpp

DataConverter.o: DataConverter.h

Data.o: Data.h

Register.o: Register.h

Memory.o: Memory.h Data.h DataConverter.h

CPU.o: CPU.h Data.h DataConverter.h Memory.h Register.h

Assembler.o: Assembler.h DataConverter.h

clean:
	rm -f  *o  main
