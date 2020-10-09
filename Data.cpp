#include "Data.h"

Data::Data(string value, int size, int start_addr, string type, string info)
{
	this->size = size;
	this->value = value;
	this->start_addr = start_addr; 
	this->type = type;
	this->info = info;
}

Data::~Data()
{
}
