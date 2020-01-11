#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <memory>
#include <functional>
#include <iterator>
#include <unordered_map>
#include <algorithm>

using namespace std;
ofstream fw;
typedef enum { mynumberlist, mystringlist, invalidtype } variabletype;
struct variableStruct {
	variabletype datatype;
	vector<string> contents;

	variableStruct() {}

	variableStruct(variabletype type, vector<string> con) {
		datatype = type;
		contents = con;
	}

	void append(string e) {
		contents.push_back(e);
	}
};
unordered_map<string, variableStruct> variables;

void printVect(vector<string> vec) {
	for (auto const& i : vec) {
		fw << i << " ";
	}
	fw << endl;
}

void printVariables() {
	for (auto x : variables) {
		fw << x.first << ": " << "datatype: " << x.second.datatype << " array: ";
		printVect(x.second.contents);

	}
}

string removeSpaces(string str)
{
	string temp = "";
	for (int x = 0; x < str.length(); x++) {
		if (str[x] != ' ')
			temp += str[x];
	}
	return temp;
}

string trim(string tobetrimmed)
{
	unsigned int i, j;
	for (i = 0; i < tobetrimmed.length() && isspace(tobetrimmed[i]); i++)
	{
		;
	}
	for (j = tobetrimmed.length() - 1; j > i && isspace(tobetrimmed[j]); j--)
	{
		;
	}

	return tobetrimmed.substr(i, j - i + 1);
}

void warning()
{
	fw << "#warning\n";
}


void error()
{
	fw << "#error\n";
}

variabletype checkType(string right) {		//returns its variabletype
	if (right.find("[") != -1) {	//gets rid of the brackets []
		right = right.substr(1, right.length() - 2);
	}

	if (variables.find(right) != variables.end()) { //found in map
		return variables[right].datatype;
	}
	else if (right.find("'") != -1 || right.find("\"") != -1) {
		return mystringlist;
	}
	else if (isdigit(right[0]))
		return mynumberlist;
	else
		return invalidtype;
}


//returns a vector of the item value
//ex: ['asd'] returns 'asd'
//	[312] returns 312
//most of the time it returns a vector with 1 item in it,
//		but if it is a variable that is already in the
//		map, it will return the whole vector of that variable
vector<string> returnValue(string right) {
	if (right.find("[") != -1) {	//gets rid of the brackets []
		right = right.substr(1, right.length() - 2);
	}

	vector<string> temp;
	if (variables.find(right) != variables.end()) { //found in map
		return variables[right].contents;
	}
	else {
		temp.push_back(right);
		return temp;
	}
}


//simple l2 = [123] assignments only
void simpleAssignment(string left, string right) {
	variabletype type = checkType(right);
	vector<string> values = returnValue(right);
	variableStruct temp(type, values);


	if (variables.find("left") != variables.end()) { //found in map
		if (variables[left].datatype != type) {
			warning();
		}
	}
	variables[left] = temp;		//add temp object to variables map
}

//returns a vector of the operands for addition statements
//ex: a+b+["23"]
//vector will contain (a,b,"23")
vector<string> getPlusOperands(string right) {
	vector<string> rightSide;
	right = removeSpaces(right);
	string operand = "";
	for (char const& c : right) {
		if (c != '+' && c != '[' && c != ']') {
			operand += c;
		}
		else {
			if (!operand.empty())
				rightSide.push_back(operand);
			operand = "";
		}
	}
	if (!operand.empty())
		rightSide.push_back(operand);
	return rightSide;
}

//for datatype conflicts
bool operandsSameType(vector<string> operands) {
	variabletype type = checkType(operands[0]);
	for (auto const& i : operands) {
		if (checkType(i) != type) {
			return false;
		}
	}
	return true;
}

//similar to returnValue() except this is for addition statements
vector<string> returnOperandValues(vector<string> operands) {
	vector<string> values;
	for (auto i : operands) {
		vector<string> temp = returnValue(i);
		for (int x = 0; x < temp.size(); x++) {
			values.push_back(temp[x]);
		}
	}
	return values;
}


void plusAssignment(string left, string rRight) {
	string right = "";

	for (int x = 0; x < rRight.length(); x++) {	//removes parenthesis in the statement to clean the input
		if (rRight[x] != '(' && rRight[x] != ')')
			right += rRight[x];
	}
	right = removeSpaces(right);

	vector<string> operands = getPlusOperands(right);
	if (!operandsSameType(operands))			//datatype conflict
		error();

	vector<string> values = returnOperandValues(operands);
	variabletype type = checkType(values[0]);	//first list index is used as datatype for list
	variableStruct temp(type, values);			//variableStruct object created, stores the list and datatype
	if (variables.find("left") != variables.end()) { //found in map
		if (variables[left].datatype != checkType(values[0])) {
			warning();
		}
	}
	variables[left] = temp;
}

void simplePrint(string line) {
	line = trim(line);
	line = line.substr(6, line.length() - 7);
	vector<string> temp = returnValue(line);
	fw << ">>>[";
	for (int x = 0; x < temp.size(); x++) {
		fw << temp[x];
		if (x != temp.size() - 1)
			fw << ",";
	}
	fw << "]\n";
}

void indexPrint(string line) {
	line = trim(line);
	line = line.substr(6, line.length() - 7);
	//l3[3]
	string left = line.substr(0, line.find('['));
	string index = line.substr(line.find('[') + 1);
	index = index.substr(0, index.find(']'));

	fw << ">>>" << variables[left].contents[stoi(index)] << endl;
}

void splicePrint(string line) {
	line = trim(line);
	line = line.substr(6, line.length() - 7);
	//l3[3:]
	string left = line.substr(0, line.find('['));
	string index = line.substr(line.find('[') + 1);
	index = index.substr(0, index.find(']'));
	//left = l3		index = 3:

	int frontIndex, backIndex;
	//find where the ':' is
	if (index[index.find(':')] == index[0]) { //: is in front. ex: [:3]
		frontIndex = 0;
		backIndex = stoi(index.substr(index.find(':') + 1));
	}
	else if (index.find(':') == index.length() - 1) { //: is in back. ex: [3:]
		frontIndex = stoi(index.substr(0, index.find(':')));
		backIndex = variables[left].contents.size() - 1;
	}
	else {
		frontIndex = stoi(index.substr(0, index.find(':')));
		backIndex = stoi(index.substr(index.find(':') + 1));
	}


	fw << ">>>[";
	for (int x = frontIndex; x <= backIndex; x++) {
		fw << variables[left].contents[x];
		if (x != backIndex)
			fw << ",";
	}
	fw << "]\n";
}

bool isExpressionTrue(string line) {
	string left, right;
	int leftVal, rightVal;
	if (line.find("==") != -1) {			//a1==30
		left = line.substr(0, line.find('='));
		right = line.substr(line.find('=') + 2, line.length());
		if (checkType(left) == mystringlist && checkType(right) == mystringlist) {
			return (returnValue(left) == returnValue(right)) ? true : false;
		}
		leftVal = stoi(returnValue(left)[0]);
		rightVal = stoi(returnValue(right)[0]);
		return (leftVal == rightVal) ? true : false;
	}
	else if (line.find("!=") != -1) {		//a1!=30
		left = line.substr(0, line.find('!'));
		right = line.substr(line.find('!') + 2, line.length());
		if (checkType(left) == mystringlist && checkType(right) == mystringlist) {
			return (returnValue(left) != returnValue(right)) ? true : false;
		}
		leftVal = stoi(returnValue(left)[0]);
		rightVal = stoi(returnValue(right)[0]);
		return (leftVal != rightVal) ? true : false;
	}
	else if (line.find("<") != -1) {		//a1<30
		left = line.substr(0, line.find('<'));
		right = line.substr(line.find('<') + 1, line.length());
		if (checkType(left) == mystringlist && checkType(right) == mystringlist) {
			return (returnValue(left) < returnValue(right)) ? true : false;
		}
		leftVal = stoi(returnValue(left)[0]);
		rightVal = stoi(returnValue(right)[0]);
		return (leftVal < rightVal) ? true : false;
	}
	else if (line.find(">") != -1) {		//a1>30
		left = line.substr(0, line.find('>'));
		right = line.substr(line.find('>') + 1, line.length());
		if (checkType(left) == mystringlist && checkType(right) == mystringlist) {
			return (returnValue(left) > returnValue(right)) ? true : false;
		}
		leftVal = stoi(returnValue(left)[0]);
		rightVal = stoi(returnValue(right)[0]);
		return (leftVal > rightVal) ? true : false;
	}
	return false;
}

void parseIf(string line) {
	line = removeSpaces(line);
	//line=m1=22ifa1==30else0

	string left = line.substr(0, line.find('='));
	//left = "m1"

	string ifTrue = line.substr(line.find('=') + 1);
	ifTrue = ifTrue.substr(0, ifTrue.find("if"));
	//ifTrue = "22"

	string expression = line.substr(line.find("if") + 2);
	expression = expression.substr(0, expression.find("else"));
	//expression = "a1==30"

	string ifFalse = line.substr(line.find("else") + 4);
	//ifFalse = "0"

	if (isExpressionTrue(expression)) {
		simpleAssignment(left, ifTrue);
	}
	else
		simpleAssignment(left, ifFalse);
}

void parse(string line) {
	string left, right;
	
	if (line.find("#") != -1) //must be a comment
	{
		fw << line << endl;
	}
	else if (line.find("=") != -1 && line.find("+") == -1 && line.find("if") == -1) {		//simple assignment statement
		left = trim(line.substr(0, line.find("=")));		// a = [23]
		right = trim(line.substr(line.find("=") + 1));
		simpleAssignment(left, right);
		fw << line << endl;
	}
	else if (line.find("+") != -1) {						//assignemt with addition
		left = trim(line.substr(0, line.find("=")));		// a = [23] + [42]
		right = trim(line.substr(line.find("=") + 1));
		plusAssignment(left, right);
		fw << line << endl;
	}
	else if (line.find("print") != -1) {
		if (line.find(':') != -1) {
			fw << line << endl;							//printing splices
			splicePrint(line);							//print(l4[3:5])
		}
		else if (line.find('[') != -1) {				//printing index
			fw << line << endl;
			indexPrint(line);							//print(l4[3])
		}
		else {
			fw << line << endl;
			simplePrint(line);							//regular print
		}												//print(l4)
	}
	else if (line.find("if") != -1) {
		parseIf(line);
		fw << line << endl;
	}
	
}



int main(int argc, char* argv[])
{

	string infile;
	if (argc <= 1)
	{
		cout << "ERROR: no input file specified" << endl;
		return -1;
	}
	string temp = argv[1];
	int pos;
	if ((pos = temp.find("=")) != -1)
	{
		infile = temp.substr(pos + 1);
	}
	else
	{
		infile = argv[1];
	}
	string outfile;
	outfile = infile.substr(0, infile.find(".")) + ".out";
	ifstream fr;
	fr.open(infile);
	fw.open(outfile);


	string line = "";
	while (getline(fr, line))
	{
		parse(line);
		//fw << "\n";
	}
	fr.close();

	//string str = "a=[3]";
	//string num = "[23]";
	//parse(str);
	//printVariables();

	//string s = "1:";
	//if (s.find(':') == s.length() - 1)
	//	cout << " asdasd";
	fw.close();
	return 0;
}



























/*#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <memory>
#include <functional>
#include <iterator>
#include <unordered_map>
#include <algorithm>
#include <regex>

using namespace std;
ofstream fw;
typedef enum { mynumberlist, mystringlist, invalidtype } variabletype;
struct variableStruct {
	variabletype datatype;
	vector<string> contents;

	variableStruct() {}

	variableStruct(variabletype type, vector<string> con) {
		datatype = type;
		contents = con;
	}

	void append(string e) {
		contents.push_back(e);
	}
};
unordered_map<string, variableStruct> variables;

void printVect(vector<string> vec) {
	for (auto const& i : vec) {
		fw << i << " ";
	}
	fw << endl;
}

void printVariables() {
	for (auto x : variables) {
		fw << x.first << ": " << "datatype: " << x.second.datatype << " array: ";
		printVect(x.second.contents);

	}
}

string removeSpaces(string str)
{
	string temp = "";
	for (int x = 0; x < str.length(); x++) {
		if (str[x] != ' ')
			temp += str[x];
	}
	return temp;
}

string trim(string tobetrimmed)
{
	unsigned int i, j;
	for (i = 0; i < tobetrimmed.length() && isspace(tobetrimmed[i]); i++)
	{
		;
	}
	for (j = tobetrimmed.length() - 1; j > i && isspace(tobetrimmed[j]); j--)
	{
		;
	}

	return tobetrimmed.substr(i, j - i + 1);
}

void warning()
{
	fw << "warning\n";
}


void error()
{
	fw << "error\n";
}

variabletype checkType(string right) {		//returns its variabletype
	if (right.find("[") != -1) {	//gets rid of the brackets []
		right = right.substr(1, right.length() - 2);
	}

	if (variables.find(right) != variables.end()) { //found in map
		return variables[right].datatype;
	}
	else if (right.find("'") != -1 || right.find("\"") != -1) {
		return mystringlist;
	}
	else if (isdigit(right[0]))
		return mynumberlist;
	else
		return invalidtype;
}


//returns a vector of the item value
//ex: ['asd'] returns 'asd'
//	[312] returns 312
//most of the time it returns a vector with 1 item in it,
//		but if it is a variable that is already in the
//		map, it will return the whole vector of that variable
vector<string> returnValue(string right) {
	if (right.find("[") != -1) {	//gets rid of the brackets []
		right = right.substr(1, right.length() - 2);
	}

	vector<string> temp;
	if (variables.find(right) != variables.end()) { //found in map
		return variables[right].contents;
	}
	else {
		temp.push_back(right);
		return temp;
	}
}

string evalfuncif(string right)
{
	string conditional = right.substr(right.find(" if ")+4, right.find(" else" )-(right.find(" if ")+4));//double check substrings
	string iftrue = right.substr(0,right.find(" if "));
	string iffalse = right.substr(right.find(" else ")+6);

	//probably trim as well
	if (conditional=="")//needs to evaluate conditional to bool
	{
		return iftrue;
	}
	else
	{
		return iffalse;
	}
}


//simple l2 = [123] assignments only
void simpleAssignment(string left, string right) {
	string value = right;
	regex funcif("^.+ *if .* else .*");//0 if f>8 else -1
	if (regex_search(right, funcif))
	{
		value = evalfuncif(right);
	}
	else
	{
		value = right;
	}
	variabletype type = checkType(value);
	vector<string> values = returnValue(value);
	variableStruct temp(type, values);


	if (variables.find("left") != variables.end()) { //found in map
		if (variables[left].datatype != type) {
			warning();
		}
	}
	variables[left] = temp;		//add temp object to variables map
}

//returns a vector of the operands for addition statements
//ex: a+b+["23"]
//vector will contain (a,b,"23")
vector<string> getPlusOperands(string right) {
	vector<string> rightSide;
	right = removeSpaces(right);
	string operand = "";
	for (char const& c : right) {
		if (c != '+' && c != '[' && c != ']') {
			operand += c;
		}
		else {
			if (!operand.empty())
				rightSide.push_back(operand);
			operand = "";
		}
	}
	if (!operand.empty())
		rightSide.push_back(operand);
	return rightSide;
}

//for datatype conflicts
bool operandsSameType(vector<string> operands) {
	variabletype type = checkType(operands[0]);
	for (auto const& i : operands) {
		if (checkType(i) != type) {
			return false;
		}
	}
	return true;
}

//similar to returnValue() except this is for addition statements
vector<string> returnOperandValues(vector<string> operands) {
	vector<string> values;
	for (auto i : operands) {
		vector<string> temp = returnValue(i);
		for (int x = 0; x < temp.size(); x++) {
			values.push_back(temp[x]);
		}
	}
	return values;
}


void plusAssignment(string left, string rRight) {
	string right = "";

	for (int x = 0; x < rRight.length(); x++) {	//removes parenthesis in the statement to clean the input
		if (rRight[x] != '(' && rRight[x] != ')')
			right += rRight[x];
	}
	right = removeSpaces(right);

	vector<string> operands = getPlusOperands(right);
	if (!operandsSameType(operands))			//datatype conflict
		error();

	vector<string> values = returnOperandValues(operands);
	variabletype type = checkType(values[0]);	//first list index is used as datatype for list
	variableStruct temp(type, values);			//variableStruct object created, stores the list and datatype
	if (variables.find("left") != variables.end()) { //found in map
		if (variables[left].datatype != checkType(values[0])) {
			warning();
		}
	}
	variables[left] = temp;
}

void simplePrint(string line) {
	line = trim(line);
	line = line.substr(6, line.length() - 7);
	vector<string> temp = returnValue(line);
	fw << ">>>[";
	for (int x = 0; x < temp.size(); x++) {
		fw << temp[x];
		if (x != temp.size() - 1)
			fw << ",";
	}
	fw << "]\n";
}

void indexPrint(string line) {
	line = trim(line);
	line = line.substr(6, line.length() - 7);
	//l3[3]
	string left = line.substr(0, line.find('['));
	string index = line.substr(line.find('[') + 1);
	index = index.substr(0, index.find(']'));

	fw <<">>>"<< variables[left].contents[stoi(index)] << endl;
}

void splicePrint(string line) {
	line = trim(line);
	line = line.substr(6, line.length() - 7);
	//l3[3:]
	string left = line.substr(0, line.find('['));
	string index = line.substr(line.find('[') + 1);
	index = index.substr(0, index.find(']'));
	//left = l3		index = 3:

	int frontIndex, backIndex;
	//find where the ':' is
	if (index[index.find(':')] == index[0]) { //: is in front. ex: [:3]
		frontIndex = 0;
		backIndex = stoi(index.substr(index.find(':') + 1));
	}
	else if (index.find(':') == index.length() - 1) { //: is in back. ex: [3:]
		frontIndex = stoi(index.substr(0, index.find(':')));
		backIndex = variables[left].contents.size() - 1;
	}
	else {
		frontIndex = stoi(index.substr(0, index.find(':')));
		backIndex = stoi(index.substr(index.find(':') + 1));
	}


	fw << ">>>[";
	for (int x = frontIndex; x <= backIndex; x++) {
		fw << variables[left].contents[x];
		if (x != backIndex)
			fw << ",";
	}
	fw << "]\n";
}

void parse(string line) {
	string left, right;
	if (line.find("=") != -1 && line.find("+") == -1) {        //simple assignment statement
		left = trim(line.substr(0, line.find("=")));        // a = [23]
		right = trim(line.substr(line.find("=") + 1));
		simpleAssignment(left, right);
		fw << line << endl;
	}
	else if (line.find("+") != -1) {                        //assignemt with addition
		left = trim(line.substr(0, line.find("=")));        // a = [23] + [42]
		right = trim(line.substr(line.find("=") + 1));
		plusAssignment(left, right);
		fw << line << endl;
	}
	else if (line.find("print") != -1) {
		if (line.find(':') != -1) {
			fw << line << endl;                            //printing splices
			splicePrint(line);                            //print(l4[3:5])
		}
		else if (line.find('[') != -1) {                //printing index
			fw << line << endl;
			indexPrint(line);                            //print(l4[3])
		}
		else {
			fw << line << endl;
			simplePrint(line);                            //regular print
		}                                                //print(l4)
	}
	else                                                 //must be a comment
	{
		fw << line << endl;
	}
}



int main(int argc, char* argv[])
{

	string infile;
	if (argc <= 1)
	{
		cout << "ERROR: no input file specified"<<endl;
		return -1;
	}
	string temp = argv[1];
	int pos;
	if ((pos = temp.find("=")) != -1)
	{
		infile = temp.substr(pos + 1);
	}
	else
	{
		infile = argv[1];
	}
	string outfile;
	outfile = infile.substr(0, infile.find(".")) + ".out";
	ifstream fr;
	fr.open(infile);
	fw.open(outfile);


	string line = "";
	while (getline(fr, line))
	{
		//fw << line<<endl;
		parse(line);
		//fw << "\n";
	}
	fr.close();

	//string str = "a=[3]";
	//string num = "[23]";
	//parse(str);
	printVariables();

	//string s = "1:";
	//if (s.find(':') == s.length() - 1)
	//	cout << " asdasd";
	fw.close();
	system("PAUSE");
	return 0;
}*/