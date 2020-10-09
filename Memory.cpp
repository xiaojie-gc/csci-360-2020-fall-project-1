#include "Memory.h"

Memory::Memory()
{
	
}


string Memory::get_value_by_addr(long start_addr){
	for (auto & element : this->storage) {
    	if(element.start_addr==start_addr){
    		return element.value;
		}
	}
	return "empty";
}

Data Memory::get_data_by_addr(long start_addr){
	for (auto & element : this->storage) {
    	if(element.start_addr==start_addr){
    		return element;
		}
	}
	return Data("0", 0, 0, "EMPTY", "");
} 

bool Memory::update(string value, int size, long start_addr, string type){
	for (auto & element : this->storage) {
    	if(element.start_addr==start_addr){
    		element.value = value;
    		element.size = size;
    		element.type = type;
    		return true;
		}
	}
	return false;
}


bool Memory::set(string value, int size, long start_addr, string type, string info){
	Data d(value, size, start_addr, type, info);
	this->storage.push_back(d);
	return true;
}

/*
	1. Load a program's assembly instructions into memory, return the starting address of main function.
	2. We assume that there exists and only exists one main function. 
	3. We also assume that 1 assembly instruction = 4 bytes.
*/
int Memory::load_program(Assembler compiler){
	int start_addr = this->start_addr_of_Text; 
	int start_addr_main_fuc;
	bool find_main_function = false;
	for (auto & f : compiler.get_functions()) {
		if(f.function_name == "main"){
			start_addr_main_fuc = start_addr;
		}
		for (auto & ins : f.assembly_instructions) {
			if(ins[0] != '#'){
				if(ins[ins.length()-1] == ':'){
					label_info label;
					label.address = start_addr;
					label.name = ins;
					this->labels.push_back(label);
				}
				this->set(ins, 4, start_addr, "text", "");
				start_addr = start_addr - 4;
			}
		}
	}
	return start_addr_main_fuc;
}

long Memory::get_address_by_label(string name){
	for (auto & label : this->labels){
		if(label.name == name){
			return label.address;
		}
	}
	return -1;
} 

/*
   memory display
*/
void Memory::display(string type, long start, long end, long current){
	
	cout << "-------------------------------------------------------------"<<endl;
	cout << "--------------------- MEMORY DISPLAY-------------------------"<<endl;
	
	for (auto & label : this->labels){
		cout <<  setw(8) << setfill(' ') << label.name << ":"  << label.address << "\t"; 
	}
	
	cout << endl;
	
	cout << "-------------------------------------------------------------"<<endl;
	
	for (auto & element : this->storage) {
		if(element.type == type){
			if(element.start_addr <= start && element.start_addr >= end){
				cout << "(" << DataConverter::convertBinToHex(DataConverter::integertobinary(element.start_addr)) << ")-" << element.start_addr << ":\t" << element.value;
				if(element.start_addr == current){
					cout << "\t\t" << "<------ now" << endl;
				}else{
					cout << endl;
				}
			}
		}
	}
	
	
	
	cout << "-------------------------------------------------------------"<<endl;
}


Memory::~Memory()
{
}
