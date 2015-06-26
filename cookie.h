#define _CRT_SECURE_NO_WARNINGS

extern "C" {
#include "unqlite.h"
#include "mongoose.h"
}

#include <vector>
#include <time.h>
#include "helper.h"

char* generateSSID(const char *user_name);
int renewCookie(mg_connection* conn, const char* username);
char* giveNewCookie(mg_connection* conn);
void destoryCookie(mg_connection *conn);
int renewCookie(mg_connection* conn, const char* username);