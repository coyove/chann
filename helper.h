#include <string>
#include <vector>
#include <algorithm>

extern "C"{
#include "mongoose.h"
}

using namespace std;

//some c++ string functions to make life easier
vector<string> split(const string &s, const string &seperator);
string replaceAll(std::string str, const std::string& from, const std::string& to);
bool endsWith(std::string const &fullString, std::string const &ending);
bool startsWith(std::string const &fullString, std::string const &start);
long extractLastNumber(mg_connection* conn);
void cleanString(string& str);