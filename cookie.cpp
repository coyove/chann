
#include "cookie.h"
#include "helper.h"

extern char* md5_salt;
extern bool stop_newcookie;
extern unqlite *pDb;
extern FILE* log_file;

void setCookie(mg_connection *conn, const char *ssid){
	char expire[100];
	time_t t = time(NULL) + 60 * 60 * 24 * 30;
	strftime(expire, sizeof(expire), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));

	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Set-Cookie: ssid=%s; expires=%s; max-age=%d; path=/; http-only; HttpOnly;\r\n"
		"Content-type: text/html\r\n"
		"X-UA-Compatible: IE=edge\r\n"
		"Content-Length: 0\r\n",
		ssid, expire, 60 * 60 * 24 * 30);
}

char* generateSSID(const char *user_name) {
	char *hash = new char[33];
	mg_md5(hash, user_name, ":", md5_salt, NULL);

	return hash;
}

char* renewCookie(const char* username){
	/*time_t rawtime;
	time(&rawtime);
	char timebuf[16];
	int c = sprintf(timebuf, "%lu", rawtime); timebuf[c] = 0;*/

	char newssid[33];
	strcpy(newssid, generateSSID(username));
	char *finalssid = new char[64];

	int len = sprintf(finalssid, "%s|%s", username, newssid);
	finalssid[len] = 0;

	return finalssid;
}

void destoryCookie(mg_connection *conn){
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-type: text/html\r\n"
		"Set-Cookie: ssid=\"\"; max-age=0; expire=\"Sat, 31 Jul 1993 00:00:00 GMT\"; path=/; http-only; HttpOnly;\r\n"
		"Content-Length: 0\r\n");
}

char* giveNewCookie(mg_connection* conn){
	if (stop_newcookie) return NULL;

	char *username = new char(10);
	unqlite_util_random_string(pDb, username, 9);
	username[9] = 0;

	/*time_t rawtime;
	time(&rawtime);
	char timebuf[16];
	int c = sprintf(timebuf, "%lu", rawtime); timebuf[c] = 0;*/

	char newssid[33];
	strcpy(newssid, generateSSID(username));
	char finalssid[64];

	int len = sprintf(finalssid, "%s|%s", username, newssid);

	//setCookie(conn, finalssid);

	fprintf(log_file, "New cookie delivered: [%s] at %s", finalssid, nowNow());

	return username;
}

char* verifyCookie(mg_connection* conn){
	char ssid[128];
	char *username = new char[10];
	mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, sizeof(ssid));
	vector<string> zztmp = split(string(ssid), string("|"));
	if (zztmp.size() != 2) return NULL;

	strncpy(username, zztmp[0].c_str(), 10);
	char testssid[33];
	strcpy(testssid, generateSSID(username));

	if (strcmp(testssid, zztmp[1].c_str()) == 0)
		return username;
	else
		return NULL;
}