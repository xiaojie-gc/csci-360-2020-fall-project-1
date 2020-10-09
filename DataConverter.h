#ifndef DATACONVERTER_H
#define DATACONVERTER_H

#include <string>
#include <iomanip>
#include <bitset>
#include <vector>
#include <map>
#include <bits/stdc++.h> 

using namespace std;


class DataConverter
{
	public:
		static long binarytointeger(string data);
		static string integertobinary(int value);
		static void split(const string &txt, vector<string> &strs, char ch);
		static void createMap(unordered_map<string, char> *um);
		static string convertBinToHex(string bin);
		static void clean();
	protected:
};

#endif
