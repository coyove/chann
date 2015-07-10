#define _CRT_SECURE_NO_WARNINGS

extern "C" {
#include "unqlite.h"
#include "mongoose.h"
}

#include <vector>
#include <time.h>
#include "helper.h"
#include <string>
#include <cstring>

void setCookie(mg_connection *conn, const char *ssid);
char* generateSSID(const char *user_name);
char* renewCookie(const char* username);
char* giveNewCookie(mg_connection* conn);
void destoryCookie(mg_connection *conn);
int renewCookie(mg_connection* conn, const char* username);
char* verifyCookie(mg_connection* conn);