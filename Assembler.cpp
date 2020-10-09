#include "Assembler.h"

Assembler::Assembler()
{
}

/*
	Entry point for assembly translation. This function will generate a function object vector.
	Inputs: source code -> string array, the number of code line.
*/
string* Assembler::compile(string* source, int max_len){
	
	cout << "-------------------------------------------------------------"<<endl;
	cout << "-                     SOURCE CODES                          -"<<endl;
	
	for(int i=0;i<max_len;i++){
		cout << source[i] << endl;
	}
	
	cout << "-------------------------------------------------------------"<<endl;
	
	function_handler(source, 0, max_len);		
	display_function_info();
	return source;
}

/*
	input:  parse functions e.g., start"int test(int a,int b,int c){"
	return:
		    none
*/
void Assembler::function_handler(string* source, int loc, int max_len){
		
	/*
		1. create a function object, get function return type and function name
	*/
	Function f1;
	string head = source[loc];
	f1.return_type = head.substr(0, head.find(' '));    // get return type "int"
	string tempstr = head.substr(head.find(' ')+1, head.length());	// "test(int a,int b,int c){" 
	f1.function_name = tempstr.substr(0, tempstr.find('(')); // get function name "test"
	f1.assembly_instructions.push_back(f1.function_name+":");	
	f1.assembly_instructions.push_back("pushq %rbp");
	f1.assembly_instructions.push_back("movq %rsp,%rbp");
	f1.is_leaf_function = true;
	
	/*
	 	2. get parameter list and read parameter values from registers
	*/
	int addr_offset = -4;
	tempstr = tempstr.substr(tempstr.find('(')+1, tempstr.length() - f1.function_name.length() - 3); // "int a,int b,int c"
	string parameter_str = tempstr.substr(tempstr.find('(')+1, tempstr.find(')'));	
	if(parameter_str.length() > 0){
		f1.variables = variable_handler(parameter_str, 2, addr_offset);
		/*
			v1.1 update 
		*/ 
		int number_of_parameter = 0;
		for (auto & var : f1.variables) {
			number_of_parameter++;
			// first 6 parameters <- registers 
			if(number_of_parameter <= 6){ 
				if(var.type == "int"){
					// this parameter is a 32 bits variable
					f1.assembly_instructions.push_back(add_mov_insturction("%" + register_for_argument_32[number_of_parameter-1], to_string(var.addr_offset)+"(%rbp)", 32));
				}else{
					// this parameter is a 64 bits variable
					f1.assembly_instructions.push_back(add_mov_insturction("%" + register_for_argument_64[number_of_parameter-1], to_string(var.addr_offset)+"(%rbp)", 64));			
				}
			// other than the first 6, the rest need to reset their offset 
			// 16 = return address + saved RBP
			}else{
				var.addr_offset = 16 + (number_of_parameter-6 -1) * 8;
			}
		}
	}
			
	/*
	    3. go through each instruction
	*/
	loc++; 	// go to next source code 
	bool next_function = false;
	while(loc < max_len){
		if((source[loc].find("int")==0 || source[loc].find("void")==0) && source[loc].find("{")==source[loc].length()-1){   
			/*
				start with a new function
			*/ 
			next_function = true;
			break;
		}else if(source[loc] == "}"){
			loc++;
		}else{
			// send to common handler dispathcer
			common_instruction_handler_dispatcher(source, loc, max_len, f1, addr_offset); 
		}
	}
	
	// 4.  we need to modify "RSP" at the prologue if this function is not a leaf function and has local variables
	if(f1.is_leaf_function == false && f1.variables.size() > 0){
		int last_offset = 0 - f1.variables.back().addr_offset;
		// if last_offset is not divisible by 16, then do 16 bytes address alignment : multiples of 16
		if(last_offset % 16 != 0){
			last_offset = ceil((float)last_offset/16) * 16;
		} 
		f1.assembly_instructions.insert(f1.assembly_instructions.begin() + 3, "subq $" + to_string(last_offset) + ",%rsp");
	}
	
	functions.push_back(f1);
	if(next_function == true){
		function_handler(source, loc, max_len);
	}
}


/*
	allocate address offset to variables
*/
void Assembler::variable_offset_allocation(string* source, int &loc, Function &f1, int &addr_offset){
	vector<Variable> vars = variable_handler(source[loc].substr(0, source[loc].length()-1), 1, addr_offset); 
	for (auto & var : vars) {
		f1.variables.push_back(var);
		string source = "$" + to_string(var.initial_value);
		string dest = to_string(var.addr_offset)+"(%rbp)";
		f1.assembly_instructions.push_back(add_mov_insturction(source, dest, 32));
	}
}

/*
	give "(operand1 comparator operand2)" return a compare_statement object
*/
Compare_statment Assembler::compare_statement_parser(string source){
	Compare_statment c1;
	string comparison_str = source.substr(source.find('(')+1, source.find(')')-source.find('(')-1);
	
	if(comparison_str.find("<=") != string::npos){
		c1.operator_str = "<="; 
		c1.revsered_operator_str = ">"; 
		c1.jump_tag = "jg";
	}else if(comparison_str.find(">=") != string::npos){
		c1.operator_str = ">="; 
		c1.revsered_operator_str = "<"; 
		c1.jump_tag = "jl";
	}else if(comparison_str.find("==") != string::npos){
		c1.operator_str = "=="; 
		c1.revsered_operator_str = "!="; 
		c1.jump_tag = "jne";
	}else if(comparison_str.find(">") != string::npos){
		c1.operator_str = ">"; 
		c1.revsered_operator_str = "<="; 
		c1.jump_tag = "jle";
	}else if(comparison_str.find("<") != string::npos){
		c1.operator_str = "<"; 
		c1.revsered_operator_str = ">="; 
		c1.jump_tag = "jge";
	}
	
	c1.operand1 = comparison_str.substr(0, comparison_str.find(c1.operator_str));
	c1.operand2 = comparison_str.substr(comparison_str.find(c1.operator_str)+c1.operator_str.length(), comparison_str.length()-comparison_str.find(c1.operator_str));
	
	//cout << operand1 << operator_str << operand2 << endl;
	return c1; 
}


/*
	It translates comparison "(operand1 comparator operand2)" to aseembly instructions.  It accepts the following logic:
	
	compare(operand1, operand2){
		instruction1
    }
	instruction2
	 
	Translate Pattern£º 
	
	1.	mov operand1, eax
	2.	cmp operand2, eax
	3.	conditional jump to "instruction2"  # it creates and returns a label to indicate the starting locaiton of instruction2
	
*/
string Assembler::compare_statement_handler(string source, Function &f1){
	Compare_statment c1 = compare_statement_parser(source);
	
	string source1, source2, dest1 = "%eax", dest2 = "eax";
	// 1. mov operand1 to eax
	int operand1_offset = get_offset_by_variable_name(c1.operand1, f1.variables);
	if(operand1_offset == -1){ // operand 1 is a constant
		source1 = "$"+c1.operand1;
	}else{ 
		source1 = to_string(operand1_offset)+"(%rbp)";
	}
	f1.assembly_instructions.push_back(add_mov_insturction(source1, dest1, 32));
	
	// 2. cmpl  operand2, eax
	int operand2_offset = get_offset_by_variable_name(c1.operand2, f1.variables);
	if(operand2_offset == -1){ // operand 2 is a constant
		source2 = "$"+c1.operand2;
	}else{ 
		source2 = to_string(operand2_offset)+"(%rbp)";
	}
	f1.assembly_instructions.push_back("cmpl " + source2 + ",%" + dest2);
	
	string label_for_false = ".L" + to_string(label_index);
	label_index++;
	f1.assembly_instructions.push_back(c1.jump_tag + " " + label_for_false);
	
	return label_for_false;
}


/*
	According to the instruction type, call the corresponding handler. Return true is matched to a specific handler.
*/
bool Assembler::common_instruction_handler_dispatcher(string* source, int &loc, int max_len, Function &f1, int &addr_offset){
	if(source[loc].find("int")==0 && source[loc].find(";")==source[loc].length()-1){   
		/*
			code line start with variable declaration keyword "int"
		*/
		variable_offset_allocation(source, loc, f1, addr_offset);
		loc++;
	}else if(source[loc].find("if")==0){   // code line start with "if"
		/*
			code line start with "if"
		*/
		f1.assembly_instructions.push_back("#"+source[loc]);    // Hashtag indicates the source code
		IF_statement_handler(source, loc, max_len, f1, addr_offset);
	}else if(source[loc].find("for")==0){   // code line start with "for"
		/*
			code line start with "for"
		*/
		f1.assembly_instructions.push_back("#"+source[loc]);    // Hashtag indicates the source code
		FOR_statement_handler(source, loc, max_len, f1, addr_offset);
	}else if(source[loc].find("return")==0){
		/*
			code line start with "return"
		*/ 
		f1.assembly_instructions.push_back("#"+source[loc]);    // Hashtag indicates the source code
		return_handler(source, loc, f1); 
		loc++;
	}else if(is_function_call(source[loc])==true){
		/*
			code line start with a function
		*/ 
		f1.assembly_instructions.push_back("#"+source[loc]);    // Hashtag indicates the source code
		function_call_handler(source, loc, f1); 
		f1.is_leaf_function = false;
		loc++;	
	}else{
		// otherwise, they are arithmetic instructions
		f1.assembly_instructions.push_back("#"+source[loc]);    // Hashtag indicates the source code
		arithmetic_handler(source[loc], f1); 
		loc++;
	}
}


/*
	IF statement, IF-ELSE statement,  NESTED IF/IF-ELSE statement 
	
	
	Case 1:
	
	if(a<b){    		// if(comparison)
		instruction1
	}else{
		instruction2
	}
	instruction3
	
	control flow and loop struture
	
		comparison
		conditional jump to "label_for_false"
		instruction1
		unconditional jump to "label_for_ending_false"
	.label_for_false:
		instruction2
	.label_for_ending_false:
		instruction3
		
	Case 2:
	
	if(a<b){    		// if(comparison)
		instruction1
	}
	instruction3
	
	control flow and loop struture
	
		comparison
		conditional jump to "label_for_false"
		instruction1
	.label_for_false:
		instruction3
		
*/ 
void Assembler::IF_statement_handler(string* source, int &loc, int max_len, Function &f1, int &addr_offset){
	/*
		generate CMP assembly instruction
	*/
	string label_for_false = compare_statement_handler(source[loc], f1);		
	string label_for_ending_false = "";

	loc++;
	while(loc < max_len){
		if(source[loc] == "}"){
			if(label_for_ending_false != ""){
				f1.assembly_instructions.push_back(label_for_ending_false+":"); 
			}else{
				f1.assembly_instructions.push_back(label_for_false+":"); 	
			}			
			loc++;
			break; 
		}else if(source[loc] == "}else{"){
			label_for_ending_false = ".L" + to_string(label_index);
			label_index++;
			f1.assembly_instructions.push_back("jump " + label_for_ending_false); 
			f1.assembly_instructions.push_back(label_for_false+":"); 
			loc++;	
		}else{
			// send to common handler dispathcer
			common_instruction_handler_dispatcher(source, loc, max_len, f1, addr_offset); 
		}
	} 
}


/*
	For loop statement: 
	
	for(int a=3;a<=10;a++){    // for(initialization;comparison;incremental)
		instruction1
		instruction2
		instruction3
	}
	instruction4
	
	1. head_component_str = [initialization, comparison, incremental].
	
	2. control flow and loop struture
	
	.label_for_comparison:
		comparison
		conditional jump to "label_for_false"
		instruction1
		instruction2
		instruction3
		incremental
		unconditional jump to "label_for_comparison"
	.label_for_false:
		instruction4
	
*/ 
void Assembler::FOR_statement_handler(string* source, int &loc, int max_len, Function &f1, int &addr_offset){
	
	string pure_head_str = source[loc].substr(source[loc].find('(') + 1, source[loc].length() - source[loc].find('(') - 3);
	vector<string> head_component_str;
	DataConverter::split(pure_head_str, head_component_str, ';');
	
	cout << pure_head_str << endl;
	
	/*
		initialization
	*/
	vector<Variable> vars = variable_handler(head_component_str.at(0), 1, addr_offset); 
	for (auto & var : vars) {
		f1.variables.push_back(var);
		string source = "$" + to_string(var.initial_value);
		string dest = to_string(var.addr_offset)+"(%rbp)";
		f1.assembly_instructions.push_back(add_mov_insturction(source, dest, 32));
	}
					
	/*
		comparison: we need to add a label to identify the starting address of comparison
	*/
	string label_for_comparison = ".L" + to_string(label_index);
	label_index++;
	f1.assembly_instructions.push_back(label_for_comparison + ":"); 	
	
	/*
		generate CMP assembly instruction
	*/
	string label_for_false = compare_statement_handler("(" + head_component_str.at(1) + ")", f1);
	
	/*
		start to translate common instructions in "{instructions}"
	*/ 
	loc++;
	while(loc < max_len){
		if(source[loc] == "}"){
			/*
				add incremental translation
			*/
			f1.assembly_instructions.push_back("#"+head_component_str.at(2));    // Hashtag indicates the source code
			arithmetic_handler(head_component_str.at(2) + ";", f1);
			/*
				unconditional jump to "label_for_comparison"
			*/
			f1.assembly_instructions.push_back("jump " + label_for_comparison);
			/*
				add label_for_false 
			*/ 
			f1.assembly_instructions.push_back(label_for_false + ":"); 	
			loc++;
			break; 
		}else{
			// send to common handler dispathcer
			common_instruction_handler_dispatcher(source, loc, max_len, f1, addr_offset); 
		}
	} 
}


/*
	Return handler
	Format: return arg; 
*/ 
void Assembler::return_handler(string* source, int loc, Function &f1){
	string returned = source[loc].substr(7, source[loc].length()-7-1);
	if(f1.return_type == "int"){
		string source = "$"+returned;
		string dest = "%eax";
		f1.assembly_instructions.push_back(add_mov_insturction(source, dest, 32));
		/*
			add function epilogue 
		*/ 
		f1.assembly_instructions.push_back("leave");
		f1.assembly_instructions.push_back("ret");
	}	
}


/*
	Function call handler
	Format: function_name(arg1,arg2,arg3,arg4,arg5,arg6,arg7); 
*/ 
void Assembler::function_call_handler(string* source, int loc, Function &f1){
	// get function object
	Function *called_function = get_function_obj_from_code(source[loc]);
	/*
	   get argument list e.g., "a,b,c" or "100,b,c" or "a,b,c,d,e,f,g" (more than 6)
	*/
	string argument_str = source[loc].substr(source[loc].find('(')+1, source[loc].find(')')-source[loc].find('(')-1);
	// split each argument value by comma ','
	vector<string> arguments;
	DataConverter::split(argument_str, arguments, ',');		
	/*
	   The first six arguments are stored in registers.  
	   The last three arguments are pushed onto the stack in reverse order.
	*/
	// reverse argument list
	reverse(arguments.begin(), arguments.end());
	// add assembly instuction to each argument
	int argument_to_register_index = arguments.size() - 1; 
	for (auto & arg : arguments) {
		Variable var = get_variable_by_name(arg,f1.variables);
		if(arguments.size() > 6 && argument_to_register_index >= 6){
			// save this argument to stack
			if(var.type == "nullptr"){
				// this argument is a constant
				f1.assembly_instructions.push_back("pushq $"+arg);
			}else{
				/*
					this argument is a variable, please notice that in 64-bit mode, the "pushq" reqires that the operand should be 64 bits 
					1. if it is 32 bits variable, we do
						->  movl offset(%rbp), %eax 
						->  pushq rax
					2. if it is 64 bits variable(address), we do
					    -> lea offset(%rbp), rax
					    -> pushq rax
				*/ 
				if(var.type == "int"){
					f1.assembly_instructions.push_back(add_mov_insturction(to_string(var.addr_offset)+"(%rbp)", "%eax", 32));
				}else if(var.type == "int[]"){
					f1.assembly_instructions.push_back("lea " + to_string(var.addr_offset)+"(%rbp),%rax");
				}
				f1.assembly_instructions.push_back("pushq %rax");	
			}
		}else{
			// save this argument to a register
			if(var.type == "nullptr"){
				// this argument is a constant
				f1.assembly_instructions.push_back(add_mov_insturction("$"+arg, "%" + register_for_argument_32[argument_to_register_index], 32));
			}else{			
				if(var.type == "int"){
					// this argument is a 32 bits variable
					f1.assembly_instructions.push_back(add_mov_insturction(to_string(var.addr_offset)+"(%rbp)", "%" + register_for_argument_32[argument_to_register_index], 32));
				}else if(var.type == "int[]"){
					// this argument is a 64 bits variable
					f1.assembly_instructions.push_back("leaq " + to_string(var.addr_offset) + "(%rbp),%" + register_for_argument_64[argument_to_register_index]);			
				}		
			} 
		}
		argument_to_register_index--;
	} 
	// 1. add "call" function
	f1.assembly_instructions.push_back("call "+called_function->function_name);
	
	// 2. we need to free the allocated stack space if number of arguments is greate than 6
	if(arguments.size() > 6){
		f1.assembly_instructions.push_back("addq $" + to_string((arguments.size() - 6) * 8) + ",%rsp");
	}
	
}



/*
	This function helps to understand and translate array variables with the following two cases.

	case 1.  array element with constant index e.g., a[1] 
		--> translation pattern	
			1. movq base_offset(%rbp), %reg64
			2. addq $offset, %reg64
	    --> function returns (%reg64)
	
	case 2.	array element with variable index e.g., a[i] 
		--> translation pattern	
			movl offset(%rbp), %reg32      # copy offset of variable i to a 32 bits register
			movslq	%reg32, %reg64					   # convert 32 bits register to 64 bits register
		--> function returns base_offset(%rbp,%reg64,4)
		
*/
string Assembler::array_operand_handler(string array_element_str, Function &f1, string reg32, string reg64){
	
	string array_name = array_element_str.substr(0, array_element_str.find('['));
	int arrary_base_offset = get_offset_by_variable_name(array_name, f1.variables);
	int element_inx_var_offset = get_offset_by_variable_name(array_element_str.substr(array_element_str.find('[')+1
														, array_element_str.length() - array_name.length() - 2), f1.variables);
	if(element_inx_var_offset == -1){
		/*
			a[3]
		*/
		int element_inx = stoi(array_element_str.substr(array_element_str.find('[')+1, array_element_str.length() - array_name.length() - 2)); 
		f1.assembly_instructions.push_back(add_mov_insturction(to_string(arrary_base_offset) + "(%rbp)", "%" + reg64, 64));
		if(element_inx > 0){
			f1.assembly_instructions.push_back("addq $" + to_string(-4 * element_inx) + ",%" + reg64);
		}
		return "(%" + reg64 + ")";
	}else{
		/*
			a[i]
		*/
		f1.assembly_instructions.push_back(add_mov_insturction(to_string(element_inx_var_offset) + "(%rbp)", "%" + reg32, 32));
		f1.assembly_instructions.push_back("movslq %" + reg32 + "," + "%" + reg64);
		return to_string(arrary_base_offset) + "(%rbp,%" + reg64 + ",4)";
	}
}

/*
	Arithmetic instruction handler, now only support addition and subtraction "+/-". 
	Please notice that this is not a smart compiler as we try to make things as simple as possible.
	Format: dest=operand1 operator operand2; 
*/ 
void Assembler::arithmetic_handler(string source, Function &f1){
	
	/* 
		eax/rax, edx/rdx, ecx/rcx 
	*/ 
	
	vector<string> temp; 
	// split by equal sign
	DataConverter::split(source.substr(0, source.length()-1), temp, '=');
	
	// get dest name
	string dest_name = temp.at(0);
	string source_assembly = "", dest_assembly = "";
	// get dest offset
	int dest_offset = get_offset_by_variable_name(dest_name, f1.variables);
	if(dest_offset == -1){
		/*
			e.g., array_name[element_inx]  or array_name[variable]
		*/
		if(dest_name.find('[') != -1 && dest_name.find(']') != -1){
			dest_assembly = array_operand_handler(dest_name, f1, "eax", "rax");
		}
	}else{
		dest_assembly = to_string(dest_offset)+"(%rbp)";
	}
	
			
	string operand1, operand2, operator_str;
	char operation;
	
	if(temp.at(1).find('+') != -1){
		operation = '+';
		operator_str = "addl";
	}else if(temp.at(1).find('-') != -1){
		operation = '-';
		operator_str = "subl";
	}
	
	// split by operation sign
	vector<string> operands; 
	DataConverter::split(temp.at(1), operands, operation);	
	operand1 = operands.at(0);
	operand2 = operands.at(1);
	bool operand1_constant = false, operand2_constant = false;
	/*
		1. move operand1 to "ecx"			
	*/ 
	int operand1_offset = get_offset_by_variable_name(operand1, f1.variables);
	string source1_assembly; 
	if(operand1_offset == -1){ 
		/*
			e.g., array_name[element_inx]
		*/
		if(operand1.find('[') != -1 && operand1.find(']') != -1){
			source1_assembly = array_operand_handler(operand1, f1, "ecx", "rcx");
		}else{
			/* 
			 operand 1 is a constant
			*/ 
			operand1_constant = true;
		}
	}else{ 
		source1_assembly = to_string(operand1_offset)+"(%rbp)";
		f1.assembly_instructions.push_back(add_mov_insturction(source1_assembly, "%ecx", 32));
		source1_assembly = "%ecx";
	}
						
	/*
		2. move operand2 to "edx"				
	*/
	int operand2_offset = get_offset_by_variable_name(operand2, f1.variables);
	string source2_assembly; 
	if(operand2_offset == -1){ 
		/*
			e.g., array_name[element_inx]
		*/
		if(operand2.find('[') != -1 && operand2.find(']') != -1){
			source2_assembly = array_operand_handler(operand2, f1, "edx", "rdx");
		}else{
			/* 
			 operand 2 is a constant
			*/ 
			operand2_constant = true;
		}
	}else{
		source2_assembly = to_string(operand2_offset)+"(%rbp)";
		f1.assembly_instructions.push_back(add_mov_insturction(source2_assembly, "%edx", 32));
		source2_assembly = "%edx";
	}
	
	if(operand1_constant == true){
		/*
		3. add/sub constant to edx				
		*/
		f1.assembly_instructions.push_back(operator_str + " $" + operand1 + "," + source2_assembly);
		source_assembly = source2_assembly;
	}else if(operand2_constant == true){
		/*
		3. add/sub constant to ecx				
		*/
		f1.assembly_instructions.push_back(operator_str + " $" + operand2 + "," +source1_assembly);
		source_assembly = source1_assembly;
	}else{
		/*
		3. add/sub ecx to edx				
		*/
		f1.assembly_instructions.push_back(operator_str + " " + source1_assembly + "," +source2_assembly);
		source_assembly = source2_assembly;
	}
	
	
	/*
		4. move edx/ecx to dest				
	*/
	f1.assembly_instructions.push_back(add_mov_insturction(source_assembly, dest_assembly, 32));		
}


/*
	Seach variable list and return the address offset of target variable.
	If not found, return -1. 
*/
int Assembler::get_offset_by_variable_name(string name, vector<Variable> variables){
	for (auto & var : variables) {
		if(var.variable_name == name || (var.type == "int[]" && var.variable_name.find(name + "[") != -1) ){
			return var.addr_offset;
		}
	}	
	return -1;
}


Variable Assembler::get_variable_by_name(string name, vector<Variable> variables){
	for (auto & var : variables) {
		if(var.variable_name == name || (var.type == "int[]" && var.variable_name.find(name + "[") != -1) ){
			return var;
		}
	}
	Variable var;
	var.type = "nullptr";
	return var;
}


/*
	input: variable string with the following two formats 
		1. Variable Declaration: "int a,b=2,c=3,d[5]={1,2,3,4,5}"
		2. Parameter List: "int a,int b,int c=3,int d[5]"
		<------ TBC: array ------->
	return:
		variable vector	    
*/
vector<Variable> Assembler::variable_handler(string parameter_str, int format, int &addr_offset){
	vector<Variable> vars;
	switch(format){
		case 1:{
			string variable_type = parameter_str.substr(0, parameter_str.find(' '));
			vector<string> variable_str; 
			/*
			    parameter_str = "int a,b[3]={1,2,3},c=3,d[2]={1,2}"
			    
			    1. Extract type "int"
			    2. Here we cannot use DataConverter::split function as array initialization string "{1,2,3}" contains multpe commas. 
				3. However, we can use "std::string::find" to search the first location of "," to extract the variable at the current
				   beginning of parameter_str. e.g.,
				   
				   iteration #1
				   a,b[3]={1,2,3},c=3,d[2]={1,2}  --> a  
				   								  --> remove "a,"  (forward by 1)
				   iteration #2
				   b[3]={1,2,3},c=3,d[2]={1,2}    --> b[3]={1   
				   			#######  we know it is an incomplete array declaration
				   			#######  get the complete declaration by searching the first "}"  
				   			#######  --> b[3]={1,2,3}
				   								   --> remove "b[3]={1,2,3}," (forward by 2)
				   iteration #3
				   c=3,d[2]={1,2}  				   --> c=3
				   								   --> remove "c=3,"
				   ......
				   ......
				   terminate when empty 
				   
 			*/ 
			int forward = 1;
			parameter_str = parameter_str.substr(variable_type.length()+1, parameter_str.length());
			while(parameter_str != ""){
				string var_str = parameter_str.substr(0, parameter_str.find(','));
				if(var_str.find('[') == -1){
					Variable var;
    				var.type = variable_type;
    				if(var_str.find('=')!=-1){    // save variable name and initial value
    					var.variable_name = var_str.substr(0, var_str.find('='));
    					var.initial_value = stoi(var_str.substr(var_str.find('=')+1,var_str.length()));
					}else{						  // only save variable name
						var.variable_name = var_str;
					}
					var.addr_offset = addr_offset;
    				vars.push_back(var);
					addr_offset -= 4;
					forward = 1;   // skip ','
				}else{
					int inx_close_bracket = parameter_str.find('}');
					var_str = parameter_str.substr(0, inx_close_bracket);   // b[3]={1,2,3}
					string array_name = var_str.substr(0, var_str.find('['));
					vector<string> array_values_str;
					/*
						remove '{' and '}', then split 1,2,3 by comma
					*/
					DataConverter::split(var_str.substr(var_str.find('{')+1, var_str.find('}')-1), array_values_str, ',');  
					int number_of_elements = 0;
					for (auto & val : array_values_str) {
						Variable var;
    					var.type = "int[]";
    					var.variable_name = array_name + "[" + to_string(number_of_elements)  + "]";
    					number_of_elements++;
    					var.initial_value = stoi(val);
    					var.addr_offset = addr_offset;
    					vars.push_back(var);
						addr_offset -= 4;
					}
					forward = 2;	// skip "},"
				}	
				if(var_str.length() + forward >= parameter_str.length()){
					break; 
				}else{
					parameter_str = parameter_str.substr(var_str.length() + forward, parameter_str.length());
				}
					
			}
			break;
		}	
		case 2:{ 
			vector<string> variable_str; 
			/*
			    1. split by comma -> ["int a", "int b[]", "int c=3"]
			*/ 
			DataConverter::split(parameter_str, variable_str, ',');
			/*
			    2. iterate each variable string
			*/
			for (auto & var_str : variable_str) {
				vector<string> one_variable_info;
				/*
			    	3. split by comma -> ["int", "c=3"] or ["int", "c"] or ["int", "c[]"]
			   */ 
    			DataConverter::split(var_str, one_variable_info, ' ');
    			Variable var;
    			var.type = one_variable_info.at(0);
    			if(one_variable_info.at(1).find('=')!=-1){    // we need to save initial values
    				var.variable_name = one_variable_info.at(1).substr(0, one_variable_info.at(1).find('='));
    				var.initial_value = stoi(one_variable_info.at(1).substr(one_variable_info.at(1).find('=')+1,one_variable_info.at(1).length()));
				}else{										  // only save variable name
					var.variable_name = one_variable_info.at(1);
				}
    			var.addr_offset = addr_offset;
    			if(var.variable_name.find('[]') != -1){
    				var.variable_name = var.variable_name.substr(0, var.variable_name.length()-2);
    				addr_offset -= 8;
    				var.type = "int[]";
				}else{
					addr_offset -= 4;
				}
				vars.push_back(var);
			} 	
			break;	
		} 
	}
	return vars;
}


/*
	Based on operand size, return a specific type of "mov" instruction e.g., movl source,dest. 
	The dest cannot be constants. Both operands cannot be memory. 
	Inputs:  source, dest, size
*/ 
string Assembler::add_mov_insturction(string source, string dest, int size){
    string opcode;
	if(size == 64){
		opcode = "movq"; 
	}else if(size == 32){
		opcode = "movl";
	}
	return opcode + " " + source + ","+ dest;
} 


/*
	Print function information on the screen.
*/ 
void Assembler::display_function_info(){
	for (auto & f : functions) {
		cout << "-------------------------------------------------------------"<<endl;
		cout << "-                     FUNCTION INFO                         -"<<endl;
		cout << "function name = " << f.function_name << ", return type = " << f.return_type << endl;
		cout << "<-- parameter & variable list -->" << endl;
		for (auto & var : f.variables) {
			cout << "name = " << var.variable_name << ", type = " << var.type << ", addr_offset = " << var.addr_offset << ", value = " << var.initial_value << endl;
		} 		
		cout << "-                     ASSEMBLY INFO                         -"<<endl;
		bool first = true; 
		for (auto & ins : f.assembly_instructions) {
			if(first == true){
				cout << ins << endl;
				first = false;
			}else if(ins[0]=='.'){
				cout << ins << endl;
			}else{
				cout << "\t" << ins << endl;
			}
		} 	
		cout << "-------------------------------------------------------------"<<endl;
	}
}

vector<Function> Assembler::get_functions(){
	return  functions;
} 

/*
   if -> function call: return true, -> otherwise return false 
*/
bool Assembler::is_function_call(string source){
	for (auto & f : functions) {
		string function_name = source.substr(0, source.find('('));
		if(function_name == f.function_name){
			return true;
		}
	} 
	return false;
}

Function *Assembler::get_function_obj_from_code(string source){
	for (auto & f : functions) {
		string function_name = source.substr(0, source.find('('));
		if(function_name == f.function_name){
			return &f;
		}
	} 
	return nullptr;
} 

Assembler::~Assembler()
{
}
