#ifndef CPU_H
#define CPU_H

#include <vector>
#include <iomanip>
#include "Register.h"
#include "Memory.h"
#include "Data.h"
 
class CPU
{
	public:
		CPU(Memory m);
		long get_reg_by_name(string name);
		bool set_reg_by_name(string name, long value);
		
		void display_stack(Memory m);
		void display_register(); 
		
		void execute_program(Memory &m); 
		string fetch(long addr, Memory &m);
		bool decode_and_exec(string ins, Memory &m);
		
		long get_operand_value(string operand, Memory m, bool addressing);
		long ALU(long operand1, long operand2, string opcode);
		void push_handler(string ins, Memory &m, string info);
		void pop_handler(string ins, Memory &m);
		void ret_handler(string ins, Memory &m);
		void mov_handler(string ins, Memory &m);
		void call_handler(string ins, Memory &m);
		void leave_handler(string ins, Memory &m);
		void jump_handler(string ins, Memory &m);
		
		
		void set_cmp_flag_register(int val1, int val2);
		void common_arithmetic_handler(string ins, Memory &m);
		
		bool is_common_arithmetic_ins(string ins);
		bool is_jump_ins(string ins);
				
		~CPU();
	private:
		vector<Register> registers;
		/*
			flag register for CMP instruction 
		*/ 
		int ZF = 0;
		int SF = 0;	
		int OF = 0;	
		string common_arithmetic_ins_prefix[5] = {"add", "sub", "mov", "lea", "cmp"}; 
		string jump_ins_prefix[7] = {"jump", "je", "jne", "jg", "jge", "jl", "jle"};
};

#endif
