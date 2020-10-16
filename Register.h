#ifndef REGISTER_H
#define REGISTER_H

#include <map>
#include <string>
#include "DataConverter.h"

using namespace std;

class Register
{
	public:
		Register();
		
		bool setAlias(string name, int length);
		bool searchName(string name);
		
		long get(string name);
		
		bool set(string name, long value);
		
		string get_hex(int length);
		string names();
		
		~Register();
	private:
		map<string, int> name_value_map; 
		/* 64 bits = 8 bytes  */
		string data;
		
};

#endif
