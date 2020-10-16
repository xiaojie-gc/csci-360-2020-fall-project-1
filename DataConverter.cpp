#include "DataConverter.h"

/*
	Function: binary to integer
*/
long DataConverter::binarytointeger(string data){
	string num = data; 
    long dec_value = 0; 
    int base = 1; 
    int len = num.length(); 
    for (int i=len-1;i>=0;i--) 
    { 
        if (num[i] == '1')         
            dec_value += base; 
        base = base * 2; 
    } 
    return dec_value; 	
}

/*
	Function: integer to binary, 32 bits; 
*/
string DataConverter::integertobinary(int data){
	string binary = bitset<32>(data).to_string();	
	return binary;
}

/*
	Function: split string by character
*/
void DataConverter::split(const string &txt, vector<string> &strs, char ch)
{
    size_t pos = txt.find( ch );
    size_t initialPos = 0;
    strs.clear();
    // Decompose statement
    while( pos != std::string::npos ) {
        strs.push_back( txt.substr( initialPos, pos - initialPos ) );
        initialPos = pos + 1;
        pos = txt.find( ch, initialPos );
    }
    // Add the last one
    strs.push_back( txt.substr( initialPos, std::min( pos, txt.size() ) - initialPos + 1 ) );
}


void DataConverter::createMap(unordered_map<string, char> *um) 
{ 
    (*um)["0000"] = '0'; 
    (*um)["0001"] = '1'; 
    (*um)["0010"] = '2'; 
    (*um)["0011"] = '3'; 
    (*um)["0100"] = '4'; 
    (*um)["0101"] = '5'; 
    (*um)["0110"] = '6'; 
    (*um)["0111"] = '7'; 
    (*um)["1000"] = '8'; 
    (*um)["1001"] = '9'; 
    (*um)["1010"] = 'A'; 
    (*um)["1011"] = 'B'; 
    (*um)["1100"] = 'C'; 
    (*um)["1101"] = 'D'; 
    (*um)["1110"] = 'E'; 
    (*um)["1111"] = 'F'; 
} 
  
// function to find hexadecimal  
// equivalent of binary 
string DataConverter::convertBinToHex(string bin) 
{ 
    int l = bin.size(); 
    int t = bin.find_first_of('.'); 
      
    // length of string before '.' 
    int len_left = t != -1 ? t : l; 
      
    // add min 0's in the beginning to make 
    // left substring length divisible by 4  
    for (int i = 1; i <= (4 - len_left % 4) % 4; i++) 
        bin = '0' + bin; 
      
    // if decimal point exists     
    if (t != -1)     
    { 
        // length of string after '.' 
        int len_right = l - len_left - 1; 
          
        // add min 0's in the end to make right 
        // substring length divisible by 4  
        for (int i = 1; i <= (4 - len_right % 4) % 4; i++) 
            bin = bin + '0'; 
    } 
      
    // create map between binary and its 
    // equivalent hex code 
    unordered_map<string, char> bin_hex_map; 
    createMap(&bin_hex_map); 
      
    int i = 0; 
    string hex = ""; 
      
    while (1) 
    { 
        // one by one extract from left, substring 
        // of size 4 and add its hex code 
        hex += bin_hex_map[bin.substr(i, 4)]; 
        i += 4; 
        if (i == bin.size()) 
            break; 
              
        // if '.' is encountered add it 
        // to result 
        if (bin.at(i) == '.')     
        { 
            hex += '.'; 
            i++; 
        } 
    } 
      
    // remove headling "00"
	while(hex.find("00")==0 && hex.length() > 2){
		hex=hex.substr(2, hex.length());
	}	  
	
	for(int i=2; i<hex.length();i=i+3){
		hex.insert(i, " ");	
	}

    // required hexadecimal number 
    return hex;     
} 

void DataConverter::clean(){
	#ifdef _WIN64
		system("CLS");
    #elif __unix__
		system("clear");
    #endif
}

