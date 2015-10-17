#define _CRT_SECURE_NO_WARNINGS

#ifndef CCHAN_HELPER_HEADER_INCLUDED
#define CCHAN_HELPER_HEADER_INCLUDED

#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <time.h>
#include <sys/stat.h>

#if defined(_WIN32)
#define cclong long
#else
#define cclong int
#endif

extern "C"{
#include "../lib/unqlite/unqlite.h"
#include "../lib/mongoose/mongoose.h"
}

#include "cookie.h"

std::vector<std::string> 
cc_split(const std::string &s, const std::string &seperator);

std::string 
cc_replace(std::string, const std::string&, const std::string&);

bool cc_ends_with(std::string const &, std::string const &);
bool startsWith(std::string const &, std::string const &);

cclong cc_extract_uri_num(mg_connection* conn);

void cc_clean_string(std::string& str);

std::string cc_random_chars(int len);
std::string cc_random_username();

std::string cc_valid_image_ext(std::string name);

void database_fatal(const char *zMsg);
void logLog(const char* msg, ...);

void cc_serve_image_file(mg_connection* conn);
bool is_admin(mg_connection* conn);

void cc_write_binary(const char* filename, const char* data, unsigned len);

const char * cc_get_client_ip(mg_connection* conn);

std::string cc_timestamp_to_time(time_t ts);
int cc_timestamp_diff_day(time_t t1, time_t t2);

#endif