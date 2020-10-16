#ifndef DATA_H
#define DATA_H

#include <string>
using namespace std;

class Data
{
	public:
		Data(string value, int size, int start_addr, string type, string info);
		~Data();
		string value;
		int size;
		int start_addr;
		string type;
		string info; 
};

#endif
