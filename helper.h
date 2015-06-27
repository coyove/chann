#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <vector>
#include <algorithm>
#include <time.h>

#if defined(_WIN32)
#define cclong long
#else
#define cclong int
#endif

extern "C"{
#include "mongoose.h"
#include "unqlite.h"
}

using namespace std;

//some c++ string functions to make life easier
vector<string> split(const string &s, const string &seperator);
string replaceAll(std::string str, const std::string& from, const std::string& to);
bool endsWith(std::string const &fullString, std::string const &ending);
bool startsWith(std::string const &fullString, std::string const &start);
cclong extractLastNumber(mg_connection* conn);
void cleanString(string& str);
char* nowNow();
void Fatal(const char *zMsg);