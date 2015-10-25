#define _CRT_SECURE_NO_WARNINGS

#ifndef CCHAN_COOKIE_HEADER_INCLUDED
#define CCHAN_COOKIE_HEADER_INCLUDED

extern "C" {
#include "../lib/unqlite/unqlite.h"
#include "../lib/mongoose/mongoose.h"
}

#include <vector>
#include <time.h>
#include <string>
#include <cstring>

bool is_admin(mg_connection* conn);

void cck_send_ssid(mg_connection *conn, const std::string c);

std::string cck_create_ssid(const std::string username);

void cck_destory_ssid(mg_connection *conn);

std::string cck_verify_ssid(mg_connection*);
std::string cck_verify_ssid(std::string);

std::string cck_extract_ssid(mg_connection* conn);

#endif