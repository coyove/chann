#include "helper.h"

using namespace std;

extern unqlite* pDb;
extern FILE* log_file;
extern string admin_cookie;
extern bool stopNewcookie;

const char * cc_get_client_ip(mg_connection* conn){
    const char * xff = mg_get_header(conn, "X-Forwarded-For");
    return (xff ? xff : conn->remote_ip);
}

void cc_write_binary(const char* filename, const char* data, unsigned len){
	FILE *fp = fopen( filename , "wb");
    fwrite(data, 1, len, fp);
    fclose(fp);
}


string cc_valid_image_ext(string name){
	string ext = "";
	if (cc_ends_with(name, ".jpg"))
        ext = ".jpg";
    else if (cc_ends_with(name, ".gif"))
        ext = ".gif";
    else if (cc_ends_with(name, ".png"))
        ext = ".png";

    return ext;
}

string cc_random_chars(int len){
	char *username = new char[len + 1];
	unqlite_util_random_string(pDb, username, len);
	username[len] = 0;

	string ret(username);

	delete [] username;

	return ret;
}

string cc_random_username(){
	if (stopNewcookie) return "";
	return cc_random_chars(9);
}

string cc_timestamp_to_time(time_t ts){
	struct tm tm_date;

    localtime_r(&(ts), &tm_date);

    char std_time[64];
    strftime(std_time, 64, "%X", &tm_date);

    string ret(std_time);
    return ret;
}

int cc_timestamp_diff_day(time_t t1, time_t t2){
	struct tm tm_t1, tm_t2;
	localtime_r(&t1, &tm_t1);
	localtime_r(&t2, &tm_t2);

	time_t diff = t1 - (tm_t1.tm_sec + tm_t1.tm_min * 60 + tm_t1.tm_hour * 3600) -
                  (t2 - (tm_t2.tm_sec + tm_t2.tm_min * 60 + tm_t2.tm_hour * 3600));
    
    return (int)(diff / 3600 / 24);
}

void logLog(const char* msg, ...){
	time_t rawtime;
	time(&rawtime);

	char timetmp[64];
	struct tm * timeinfo;
	timeinfo = localtime(&(rawtime));

	strftime(timetmp, 64, "%Y/%m/%d %X", timeinfo);

	fprintf(log_file, "[%s]\t", timetmp);

	va_list argptr;
	va_start(argptr, msg);
	vfprintf(log_file, msg, argptr);
	va_end(argptr);

	fprintf(log_file, "\n");
}

void database_fatal(const char *zMsg){
	if (pDb){
		const char *zErr;
		int iLen = 0; /* Stupid cc warning */

		/* Extract the database error log */
		unqlite_config(pDb, UNQLITE_CONFIG_ERR_LOG, &zErr, &iLen);
		if (iLen > 0){
			/* Output the DB error log */
			puts(zErr); /* Always null termniated */
		}
	}
	else{
		if (zMsg){
			puts(zMsg);
		}
	}
	/* Manually shutdown the library */
	unqlite_lib_shutdown();
	/* Exit immediately */
	exit(0);
}

//some c++ string functions to make life easier
vector<string> cc_split(const string &s, const string &seperator){
	vector<string> result;
	typedef string::size_type string_size;
	string_size i = 0;

	while (i != s.size()){
		int flag = 0;
		while (i != s.size() && flag == 0){
			flag = 1;
			for (string_size x = 0; x < seperator.size(); ++x)
				if (s[i] == seperator[x]){
					++i;
					flag = 0;
					break;
				}
		}

		flag = 0;
		string_size j = i;
		while (j != s.size() && flag == 0){
			for (string_size x = 0; x < seperator.size(); ++x)
				if (s[j] == seperator[x]){
					flag = 1;
					break;
				}
			if (flag == 0)
				++j;
		}
		if (i != j){
			result.push_back(s.substr(i, j - i));
			i = j;
		}
	}
	return result;
}

string cc_replace(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

bool cc_ends_with(std::string const &fullString, std::string const &ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else {
		return false;
	}
}

bool startsWith(std::string const &fullString, std::string const &start) {
	if (fullString.length() >= start.length()) {
		return (0 == fullString.compare(0, start.length(), start));
	}
	else {
		return false;
	}
}

cclong cc_extract_uri_num(mg_connection* conn){
	string url(conn->uri);
	vector<string> tmp = cc_split(url, "/");
	string num = tmp[tmp.size() - 1];

	return atol(num.c_str());
}

void cc_clean_string(string& str){
	str = cc_replace(str, string("<"), string("&lt;"));
	str = cc_replace(str, string(">"), string("&gt;"));
}

bool is_admin(mg_connection* conn){
    return (cck_extract_ssid(conn) == admin_cookie);
}

void cc_serve_image_file(mg_connection* conn){
	const char *ims = mg_get_header(conn, "If-Modified-Since");
    
    if (ims && (strcmp(ims, "") != 0)){
        mg_printf(conn, "HTTP/1.1 304 Not Modified\r\n\r\n");
        return;
    }

    string url(conn->uri);
    vector<string> tmp = cc_split(url, "/");
    string fname = tmp[tmp.size() - 1];
    string ctype;

    string path = "images/" + fname;

    if (cc_ends_with(fname, ".jpg"))
        ctype = "image/jpeg";
    else if (cc_ends_with(fname, ".gif"))
        ctype = "image/gif";
    else if (cc_ends_with(fname, ".png"))
        ctype = "image/png";
    else
        ctype = "application/octet-stream";

    FILE *fp; 
    struct stat st;
    char buf[1024];
    int n;

    // char expire[100];
    // time_t t = time(NULL) + 60 * 60 * 24 * 30;
    // strftime(expire, sizeof(expire), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));

    if (stat(path.c_str(), &st) == 0 && (fp = fopen(path.c_str(), "rb")) != NULL) {
        mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                        "Content-Type: %s\r\n"
                        "Last-Modified: Sat, 31 Jul 1993 00:00:00 GMT\r\n"
                        "Expires: Tue, 31 Jul 2035 00:00:00 GMT\r\n"
                        // "Cache-Control: must-revalidate\r\n"
                        "Cache-Control: Public\r\n"
                        "Content-Length: %lu\r\n\r\n", ctype.c_str(), (unsigned cclong)st.st_size);
        while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
            mg_write(conn, buf, n);
        }
        fclose(fp);
        mg_write(conn, "\r\n", 2);
    }else{
        mg_printf(conn, "HTTP/1.1 404 Not Found\r\nContent-Type: image/jpeg\r\nContent-Length: 0\r\n\r\n");
    }   //image
}