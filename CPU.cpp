#include "CPU.h"

CPU::CPU(Memory m)
{	
	string register_for_argument_32[7] = {"eax", "edi", "esi", "edx", "ecx", "r8d", "r9d"}; // v1.1 
	string register_for_argument_64[7] = {"rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9"};	 // v1.1
	
	for(int i=0;i<7;i++){
		Register r1;
		r1.setAlias(register_for_argument_64[i], 64);
		r1.setAlias(register_for_argument_32[i], 32);
		this->registers.push_back(r1); 
	}	
	
	Register r2;
	r2.setAlias("rip", 64);   // ins Pointer Register (RIP) 
	this->registers.push_back(r2);
		
	Register r3;
	r3.setAlias("rbp", 64);
	r3.set("rbp", m.start_addr_of_stack);
	this->registers.push_back(r3);
	
	Register r4;
	r4.setAlias("rsp", 64);
	r4.set("rsp", m.start_addr_of_stack);
	this->registers.push_back(r4);
	
}

long CPU::get_reg_by_name(string name){
	for (auto & r : this->registers) {
    	if(r.searchName(name)){
    		return r.get(name);
		}
	}
}

bool CPU::set_reg_by_name(string name, long value){
	for (auto & r : this->registers) {
    	if(r.searchName(name)){
    		return r.set(name, value);
		}
	}
	return false;
}



void CPU::execute_program(Memory &m){
	getchar();
	while(1){
		DataConverter::clean();
		long next_ins_addr = this->get_reg_by_name("rip");   // get the address of next ins to be executed
		m.display("text", next_ins_addr + 8, next_ins_addr - 16, next_ins_addr);
		this->display_register();
		this->display_stack(m);
		string next_ins = this->fetch(next_ins_addr, m);
		if(next_ins == "empty"){
			break;	
		}else{
			if(this->decode_and_exec(next_ins, m) == true){
				this->set_reg_by_name("rip", next_ins_addr -4); 
			}
			cout << "execute=[" << next_ins << "]" << endl;
			getchar();
		}
	}
}


bool CPU::is_jump_ins(string ins){
	for(string prefix : this->jump_ins_prefix){
   	 	if(ins.find(prefix)==0){
   	 		return true;
		} 
   } 
   return false;
} 


bool CPU::is_common_arithmetic_ins(string ins){
   for(string prefix : this->common_arithmetic_ins_prefix){
   	 	if(ins.find(prefix)==0){
   	 		return true;
		} 
   } 
   return false;
}

 
string CPU::fetch(long addr, Memory &m){
	return m.get_value_by_addr(addr);
}


bool CPU::decode_and_exec(string ins, Memory &m){
	if(ins.find("leave") == 0){
		this->leave_handler(ins, m);
		return true;
	}else if(ins.find("ret") == 0){
		this->ret_handler(ins, m);
		return false;
	}else if(this->is_common_arithmetic_ins(ins) == true){
		this->common_arithmetic_handler(ins, m);
		return true;
	}else if(is_jump_ins(ins) == true){
		this->jump_handler(ins, m);
		return false;
	}else if(ins.find("pushq") == 0){
		this->push_handler(ins, m, "");
		return true;
	}else if(ins.find("call") == 0){
		this->call_handler(ins, m);
		return false;
	}
	return true;
}

/*
	%reg,%reg
	offset(%reg),%reg
	%reg,offset(%reg)
	%reg,offset(%rbp,%reg,4)
	offset(%rbp,%reg,4),%reg
*/
long CPU::get_operand_value(string operand, Memory m, bool addressing){
	if(operand[0] == '$'){
		return stol(operand.substr(1, operand.length()));
	}else if(operand[0] == '%'){
		string reg_name = operand.substr(1, operand.length());
		return this->get_reg_by_name(reg_name);
		
	}else{
		long offset = 0;
		string offset_str = operand.substr(0, operand.find('('));
		if(offset_str != ""){
			offset = stol(operand.substr(0, operand.find('(')));
		}
		long base_offset;
		string reg_str = operand.substr(operand.find('(') + 1, operand.length() - operand.find('(') - 2);
		vector<string> con;
		DataConverter::split(reg_str, con, ',');
		if(con.size() > 1){
			string reg_name_1 = con[0].substr(1, con[0].length());
			string reg_name_2 = con[1].substr(1, con[1].length());
			base_offset = this->get_reg_by_name(reg_name_1) - this->get_reg_by_name(reg_name_2) * stoi(con[2]);
		}else{
			string reg_name = con[0].substr(1, con[0].length());
			base_offset = this->get_reg_by_name(reg_name);
		}
		
		if(addressing == true){
			return base_offset + offset;
		}else{
			return stol(m.get_value_by_addr(base_offset + offset));
		}	
	}
} 

/*
	Arithmetic logic unit
*/
long CPU::ALU(long operand1, long operand2, string opcode){
	if(opcode.find("mov") != -1 || opcode.find("lea") != -1){
		operand2 = operand1;
	}else if(opcode.find("add") != -1){
		operand2 = operand2 + operand1;
	}else if(opcode.find("sub") != -1){
		operand2 = operand2 - operand1;
	}
	return operand2;
}

/*


*/
void CPU::set_cmp_flag_register(int val1, int val2){
	if(val1 == val2){
		this->ZF = 1;
		this->SF = 0; 
		this->OF = 0; 
	}else if(val1 != val2){
		this->ZF = 0;
		this->SF = 0; 
		this->OF = 0;
	}else if(val1 < val2){
		this->ZF = 0;
		this->SF = 1; 
		this->OF = 1; 
	}else if(val1 <= val2){
		this->ZF = 1;
		this->SF = 1;
		this->OF = 1;  
	}else if(val1 > val2){
		this->ZF = 0;
		this->SF = 1;
		this->OF = 0; 
	}else if(val1 >= val2){
		this->ZF = 1;
		this->SF = 1;
		this->OF = 0; 
	}
} 

void CPU::jump_handler(string ins, Memory &m){
	string opcode = ins.substr(0, ins.find(' '));
	string label = ins.substr(opcode.length() + 1, ins.length()) + ":";
	long addr = m.get_address_by_label(label);
	bool jump = false; 
	if(opcode == "jump"){
		jump = true;
	}else if(opcode == "je" && this->ZF == 1 && this->SF ==0 && this->OF == 0){
		jump = true;
	}else if(opcode == "jne" && ((this->ZF == 0 && this->SF ==0 && this->OF == 0)||(this->ZF == 0 && this->SF ==1 && this->OF == 1)
														||(this->ZF == 0 && this->SF ==1 && this->OF == 0))){
		jump = true;
	}else if(opcode == "jl" && this->ZF == 0 && this->SF ==1 && this->OF == 1){
		jump = true;
	}else if(opcode == "jle" && ((this->ZF == 1 && this->SF ==1 && this->OF == 1)||(this->ZF == 0 && this->SF ==1 && this->OF == 1)
														||(this->ZF == 1 && this->SF ==0 && this->OF == 0))){
		jump = true;
	}else if(opcode == "jg" && this->ZF == 0 && this->SF ==1 && this->OF == 0){
		jump = true;
	}else if(opcode == "jge" && ((this->ZF == 1 && this->SF ==1 && this->OF == 0) || (this->ZF == 0 && this->SF ==1 && this->OF == 0) 
														|| (this->ZF == 1 && this->SF ==0 && this->OF == 0))) {
		jump = true;
	}
	if(jump == true){
		this->set_reg_by_name("rip", addr);
	}else{
		long rip = this->get_reg_by_name("rip");
		this->set_reg_by_name("rip", rip - 4);
	}
}


/*
	%reg,%reg
	offset(%reg),%reg
	%reg,offset(%reg)
	%reg,offset(%rbp,%reg,4)
	offset(%rbp,%reg,4),%reg
*/
void CPU::common_arithmetic_handler(string ins, Memory &m){
	
	string opcode = ins.substr(0, ins.find(' '));
	ins = ins.substr(opcode.length() + 1, ins.length());
	string first_con = ins.substr(0, ins.find(','));
	if(first_con.find('(') != -1 && first_con.find(')') == -1){
		first_con = ins.substr(0, ins.find(')')+1);
	}
	string operand_1 = first_con, operand_2 = ins.substr(first_con.length() + 1, ins.length());
	long operand_1_val = this->get_operand_value(operand_1, m, opcode.find("lea") == 0); 
	
	if(operand_2[0] == '%'){
		string reg_name = operand_2.substr(1, operand_2.length());
		long old_value = this->get_reg_by_name(reg_name);
		if(opcode.find("cmp") == -1){
			long new_val = this->ALU(operand_1_val, old_value, opcode);
			this->set_reg_by_name(reg_name, new_val);	
		}else{
			this->set_cmp_flag_register(old_value, operand_1_val);
		}
	}else{
		long offset = 0;
		string offset_str = operand_2.substr(0, operand_2.find('('));
		if(offset_str != ""){
			offset = stol(operand_2.substr(0, operand_2.find('(')));
		}
		long base_offset;
		string reg_str = operand_2.substr(operand_2.find('(') + 1, operand_2.length() - operand_2.find('(') - 2);
		vector<string> con;
		DataConverter::split(reg_str, con, ',');
		if(con.size() > 1){
			string reg_name_1 = con[0].substr(1, con[0].length());
			string reg_name_2 = con[1].substr(1, con[1].length());
			base_offset = this->get_reg_by_name(reg_name_1) - this->get_reg_by_name(reg_name_2) * stoi(con[2]);
		}else{
			string reg_name = con[0].substr(1, con[0].length());
			base_offset = this->get_reg_by_name(reg_name);
		}
		Data old = m.get_data_by_addr(base_offset + offset);	
		if(opcode.find("cmp") == -1){
			long new_val = this->ALU(operand_1_val, stol(old.value), opcode);
			if(old.type == "EMPTY"){
				m.set(to_string(new_val), old.size, base_offset + offset, "stack", "");
			}else{
				m.update(to_string(new_val), old.size, base_offset + offset, "stack");
			}
		}else{
			this->set_cmp_flag_register(stoi(old.value), operand_1_val);
		}	
	}
}

/*
    call func_name
*/
void CPU::call_handler(string ins, Memory &m){
	string func_name = ins.substr(5, ins.length());
	for (auto & label : m.labels){
		if(label.name.find(func_name) == 0){
			// push return address to the stack
			int return_addr = this->get_reg_by_name("rip") - 4;
			this->push_handler("pushq $" + to_string(return_addr), m, "return address");
			this->set_reg_by_name("rip", label.address);
			return;
		}
	}
}


/*
    leave = 
    movq rbp, rsp
    popq rbp
*/
void CPU::leave_handler(string ins, Memory &m){
	this->common_arithmetic_handler("movq %rbp,%rsp", m);
	this->pop_handler("popq $rbp", m);
}


/*
    ret = 
    popq %rip
*/
void CPU::ret_handler(string ins, Memory &m){
	this->pop_handler("popq $rip", m);
}

 
/*
	Given format = "pushq operand", the operand is 64 bits. The operand can be a constant or 64 bits register.
	
*/
void CPU::push_handler(string ins, Memory &m, string info){
		
	string operand = ins.substr(ins.find(' ') + 1, ins.length());
	long rsp = this->get_reg_by_name("rsp");
	rsp = rsp - 8;
	
	/*
		constant starts with '$' while register starts with '%' 
	*/
	if(operand[0] == '$'){
		m.set(operand.substr(1, operand.length()), 64, rsp, "stack", info);
	}else if(operand[0] == '%'){
		long reg_val = this->get_reg_by_name(operand.substr(1, operand.length()));
		m.set(to_string(reg_val), 64, rsp, "stack", info);
	}
	
	this->set_reg_by_name("rsp", rsp);
} 


/*
	Given format = "popq %reg", the operand is 64 bits register.
	
*/
void CPU::pop_handler(string ins, Memory &m){
	string operand = ins.substr(ins.find(' ') + 1, ins.length());
	long rsp = this->get_reg_by_name("rsp");
	long reg_val = this->get_reg_by_name("rsp");
	this->set_reg_by_name(operand.substr(1, operand.length()), stol(m.get_value_by_addr(reg_val)));	
	rsp = rsp + 8;
	this->set_reg_by_name("rsp", rsp);
} 


/*
   CPU display
*/
void CPU::display_register(){
	
	cout << "-------------------------------------------------------------"<<endl;
	cout << "-----------------------REG DISPLAY---------------------------"<<endl;
	int inx = 1;
	for (auto & reg : this->registers) {
		cout << setw(9) << setfill(' ') << reg.names() << ":" << setw(4) << setfill(' ') << reg.get("") << "(" << reg.get_hex(64) << ")";
		if(inx % 3 == 0){
			cout  << endl;
		}else{
			cout << "\t";
		}
		inx++;
	}
	cout << endl;
	cout << "-------------------------------------------------------------"<<endl;
	cout << setw(9) << setfill(' ') << "[ZF]:" << this->ZF << setw(9) << setfill(' ')  << "[SF]:" << this->SF << setw(9) << setfill(' ')  << "[OF]:" << this->OF << endl;
 	cout << "-------------------------------------------------------------"<<endl;
}

void CPU::display_stack(Memory m){
	
	cout << "-------------------------------------------------------------"<<endl;
	cout << "-----------------------STACK DISPLAY-------------------------"<<endl;
	
	long rsp = this->get_reg_by_name("rsp");
	long rbp = this->get_reg_by_name("rbp");
	
	
	for (auto & element : m.storage) {
		if(element.type == "stack"){
			cout << "(" << DataConverter::convertBinToHex(DataConverter::integertobinary(element.start_addr)) << ")-" << element.start_addr << ":\t" << element.value;
			cout << "\t";
			cout << setw(12) << setfill(' ')  << element.start_addr - rbp << "(%rbp)";
			
			string specifics = "";
			if(rsp == element.start_addr){
			   specifics += "rsp ";
			}
			
			if(rbp == element.start_addr){
				specifics += "rbp ";
			}
			
			if(element.info != ""){
				specifics += element.info;
			}
			
			cout << "\t";
			
			cout << specifics;
			
			cout << endl;
		}
	}
	cout << "-------------------------------------------------------------"<<endl;
}


CPU::~CPU()
{
}
