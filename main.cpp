#include <iostream>
#include <string>
#include "DataConverter.h"
#include "Assembler.h"
#include "Memory.h"
#include "CPU.h"

using namespace std;

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

/*

int a,b=1,c[3]={1,2,3};

int a,b=1;
int c[3]={1,2,3};

*/ 

int main(int argc, char** argv) {
	Assembler compiler;
	string source_test_1[22] = {
		"int test1(int a,int b[],int c,int d,int e,int f,int g,int h[],int i){",
		    "b[1]=a+1;",
		    "b[a]=a+1;",
			"return 1;",
		"}",
		"int main(){",
			"int a=1,b[5]={11,22,33,44,55},c,d[2]={1,2},e=5;",
			"test1(a,b,3,4,5,6,c,d,e);",
			"return 1;",
		"}"
	};
	string source_test_2[22] = {
		"int main(){",
		"int a[5]={1,2,3,4,5},sum=0;",
		"for(int i=0;i<5;i=i+1){",
			"sum=a[i]+sum;",	
		"}",
		"return 3;",
		"}"
	};
	
	
	int max_len=0;
	for(string code : source_test_1){
		if(code.length()>0){
			max_len++;
		}
	}
	cout << "number of code lines = " << max_len << endl;
	compiler.compile(source_test_1, max_len);
	
	Memory m;
	long start_addr_main_fuc = m.load_program(compiler);
	
	CPU c(m);
	c.set_reg_by_name("rip", start_addr_main_fuc);	
	c.execute_program(m);
		
	return 0;
}



