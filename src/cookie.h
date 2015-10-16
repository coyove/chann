#define _CRT_SECURE_NO_WARNINGS

#ifndef CCHAN_COOKIE_HEADER_INCLUDED
#define CCHAN_COOKIE_HEADER_INCLUDED

extern "C" {
#include "../lib/unqlite/unqlite.h"
#include "../lib/mongoose/mongoose.h"
}

#include <vector>
#include <time.h>
#include "helper.h"
#include <string>
#include <cstring>

void set_cookie(mg_connection *conn, const std::string c);
char* generateSSID(const char *user_name);

std::string to_ssid(const std::string username);

std::string random_9chars();
void destoryCookie(mg_connection *conn);
int renewCookie(mg_connection* conn, const char* username);

std::string verify_cookie(mg_connection* conn);

char* verifyCookieStr(char* szSSID);

#endif