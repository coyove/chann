#ifndef CHANN_HELPER_HEADER_INCLUDED
#define CHANN_HELPER_HEADER_INCLUDED
// #include "global.h"
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <time.h>
#include <sys/stat.h>

extern "C"{
#include "../lib/unqlite/unqlite.h"
#include "../lib/mongoose/mongoose.h"
}

bool is_admin(mg_connection* conn);

std::string is_assist(mg_connection* conn);

void cck_send_ssid(mg_connection *conn, const std::string c);

void cck_send_admin_ssid(mg_connection *conn, const std::string ssid, const char* type = "admin");

std::string 
cck_create_ssid(const std::string username);

void cck_destory_ssid(mg_connection *conn);

std::string 
cck_verify_ssid(mg_connection*);

std::string 
cck_verify_ssid(std::string);

std::string 
cck_extract_ssid(mg_connection* conn);

std::vector<std::string> 
cc_split(const std::string &s, const std::string &seperator);

std::string 
cc_replace(std::string, const std::string&, const std::string&);

bool cc_ends_with(std::string const &, std::string const &);
bool startsWith(std::string const &, std::string const &);

int cc_extract_uri_num(mg_connection* conn);

void cc_clean_string(std::string& str);

std::string cc_random_chars(int len);
std::string cc_random_username();

std::string cc_valid_image_ext(std::string name);

void database_fatal(const char *zMsg);
void logLog(const char* msg, ...);

void cc_serve_image_file(mg_connection* conn);
// bool is_admin(mg_connection* conn);

void cc_write_binary(const char* filename, const char* data, unsigned len);

const char * cc_get_client_ip(mg_connection* conn);

std::string cc_timestamp_to_time(time_t ts);
int cc_timestamp_diff_day(time_t t1, time_t t2);

void cc_store_set_to_file(const std::string&, std::unordered_set<std::string>&);
void cc_load_file_to_set(const std::string&, std::unordered_set<std::string>&);

#endif