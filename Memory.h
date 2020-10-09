#ifndef MEMORY_H
#define MEMORY_H

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include "Data.h"
#include "Assembler.h"

using namespace std;

/*
   Therefore, we can use string array to simulate memory storage.
   Assumptions:  1 insturction = 4 bytes
                 
*/

struct label_info{
	string name;
	long address; 
};

class Memory
{
	public:
		Memory();
		
		string get_value_by_addr(long start_addr); 	
		Data get_data_by_addr(long start_addr);
		long get_address_by_label(string name);
		bool set(string value, int size, long start_addr, string type, string info);
		bool update(string value, int size, long start_addr, string type);
		void display(string type, long start, long end, long current);
		int load_program(Assembler compiler);

		vector<Data> storage; 
		vector<label_info> labels;
		
		long start_addr_of_stack = 2048;
		long start_addr_of_heap  = 1000;
		long start_addr_of_BSS   = 800;
		long start_addr_of_Data  = 700;
		long start_addr_of_Text  = 500;
		
		~Memory();
};

#endif
