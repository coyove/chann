#define _CRT_SECURE_NO_WARNINGS

#ifndef CCHAN_HELPER_HEADER_INCLUDED
#define CCHAN_HELPER_HEADER_INCLUDED

#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <time.h>

#if defined(_WIN32)
#define cclong long
#else
#define cclong int
#endif

extern "C"{
#include "../lib/unqlite/unqlite.h"
#include "../lib/mongoose/mongoose.h"
}

using namespace std;

#define TO_STRING_PLACEHOLDER "%tsp%"
#define _N(a) TO_STRING_PLACEHOLDER, a
#define BEGIN_STRING(x) szBuilder(x, sizeof(x)
#define END_STRING NULL)
#define _(x) #x

//some c++ string functions to make life easier
vector<string> split(const string &s, const string &seperator);
string replaceAll(std::string str, const std::string& from, const std::string& to);
bool endsWith(std::string const &fullString, std::string const &ending);
bool startsWith(std::string const &fullString, std::string const &start);
cclong extractLastNumber(mg_connection* conn);
void cleanString(string& str);
char* nowNow();
void Fatal(const char *zMsg);
void logLog(const char* msg, ...);
size_t szBuilder(char* buf, size_t buf_size, ...);

#endif