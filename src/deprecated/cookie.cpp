#include "cookie.h"
// #include "config.h"
#include "helper.h"

using namespace std;

extern char md5Salt[64];
extern bool stopNewcookie;
extern unqlite *pDb;
extern FILE* log_file;
extern string admin_cookie;


bool is_admin(mg_connection* conn){
    return (cck_extract_ssid(conn) == admin_cookie);
}

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

string __generate_appender(const char *username) {
	char *hash = new char[33];
	// ConfigManager c;

	// mg_md5(hash, username, ":", c.global().get("security::salt").c_str(), NULL);
	mg_md5(hash, username, ":", md5Salt, NULL);

	string ret(hash);
	delete [] hash;

	return ret;
}

string cck_create_ssid(const char* username){
	// char newssid[33];
	// strcpy(newssid, to_ssid(username));

	string appender = __generate_appender(username);

	char finalssid[64];// = new char[64];

	int len = sprintf(finalssid, "%s|%s", username, appender.c_str());
	finalssid[len] = 0;

	std::string ret(finalssid);

	return ret;
}

string cck_create_ssid(const string username){
	return cck_create_ssid(username.c_str());
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