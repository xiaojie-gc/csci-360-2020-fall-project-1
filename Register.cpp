#include "Register.h"

Register::Register()
{
	for(int i = 0; i<64; i++){
		this->data += "0";
	}
}

bool Register::setAlias(string name, int length){
	this->name_value_map.insert({name, length});
	return true;
}

bool Register::searchName(string name){
	for (auto const& x : this->name_value_map){
		if(name == x.first){
			return true;
		}       
	}
	return false;
}


bool Register::set(string name,  long value){
	string bin_str = DataConverter::integertobinary(value);
	
	this->data = "";
	
	for(int i = 0; i< 64 - bin_str.length(); i++){
		this->data += "0";
	}
	
	this->data += bin_str;
	
	return true;
}



long Register::get(string name){
	int length;
	if(name == ""){
		length = 64;
	}else{
		length = this->name_value_map.find(name)->second;
	}
	string str_value = this->data.substr(64 - length, 64);
	return DataConverter::binarytointeger(str_value); 
} 

string Register::get_hex(int length){
	return DataConverter::convertBinToHex(this->data.substr(64 - length, 64)); 
}


/*
	Get all names in the following format.
	eax/rax
	edx/rcx
*/
string Register::names(){
	
	string name_str = "[";
	bool first = true;
	for (auto const& x : this->name_value_map){
		if(first == false){
			name_str += "/";
		}else{
			first = false;
		}
		name_str += x.first;
	}
	name_str += "]";
	
	
	return name_str;
}

Register::~Register()
{
}
