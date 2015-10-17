
#include "cookie.h"
#include "helper.h"

extern char md5Salt[64];
extern bool stopNewcookie;
extern unqlite *pDb;
extern FILE* log_file;

using namespace std;

string cck_extract_ssid(mg_connection* conn){
	char ssid[128]; 
    mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, sizeof(ssid));
    string ret(ssid);

    return ret;
}

void cck_send_ssid(mg_connection *conn, const std::string c){
	char expire[100];
	time_t t = time(NULL) + 60 * 60 * 24 * 30;
	strftime(expire, sizeof(expire), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));

	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Set-Cookie: ssid=%s; expires=%s; max-age=%d; path=/; http-only; HttpOnly;\r\n"
		"Content-type: text/html\r\n"
		"X-UA-Compatible: IE=edge\r\n"
		"Content-Length: 0\r\n",
		c.c_str(), expire, 60 * 60 * 24 * 30);
}

char* to_ssid(const char *user_name) {
	char *hash = new char[33];
	mg_md5(hash, user_name, ":", md5Salt, NULL);

	return hash;
}

string cck_create_ssid(const char* username){
	char newssid[33];
	strcpy(newssid, to_ssid(username));
	char finalssid[64];// = new char[64];

	int len = sprintf(finalssid, "%s|%s", username, newssid);
	finalssid[len] = 0;

	std::string ret(finalssid);

	return ret;
}

string cck_create_ssid(const string username){
	char newssid[33];
	strcpy(newssid, to_ssid(username.c_str()));
	char finalssid[64];// = new char[64];

	int len = sprintf(finalssid, "%s|%s", username.c_str(), newssid);
	finalssid[len] = 0;

	std::string ret(finalssid);

	return ret;
}


void cck_destory_ssid(mg_connection *conn){
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-type: text/html\r\n"
		"Set-Cookie: ssid=\"(none)\"; expires=\"Sat, 31 Jul 1993 00:00:00 GMT\"; path=/; http-only; HttpOnly;\r\n"
		"Content-Length: 0\r\n");
}

string cck_verify_ssid(mg_connection* conn){
	string ssid = cck_extract_ssid(conn);

	return cck_verify_ssid(ssid);
}

string cck_verify_ssid(string ssid){
	string username;
	vector<string> zztmp = cc_split(ssid, string("|"));
	if (zztmp.size() != 2) return username;

	username = zztmp[0];
	string _ssid = cck_create_ssid(username);

	return (_ssid == ssid) ? username : "";
}