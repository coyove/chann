#include "helper.h"
// #include "cookie.h"
#include "config.h"

using namespace std;

extern unqlite* pDb;
extern FILE* log_file;
extern string admin_cookie;
extern map<string, string> assist_cookie;

bool is_admin(mg_connection* conn){
    return (cck_extract_ssid(conn) == admin_cookie);
}

string is_assist(mg_connection* conn){
    string ssid = cck_extract_ssid(conn);
    for(auto x = assist_cookie.begin(); x != assist_cookie.end(); ++x){
        // cout << x->first << "," << x->second << "," << ssid << endl;
        if(x->second == ssid) return x->first;
    }

    return string("");
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

void cck_send_admin_ssid(mg_connection *conn, const string ssid, const char* type){
    ConfigManager c;
    char expire[100];
    time_t t = time(NULL) + 60 * c.global().get<int>("security::" + string(type) + "::expire_time");
    strftime(expire, sizeof(expire), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));

    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Set-Cookie: ssid=%s; expires=%s; path=/; http-only; HttpOnly;\r\n"
        "Content-type: text/html\r\n"
        "X-UA-Compatible: IE=edge\r\n"
        "Content-Length: 0\r\n",
        ssid.c_str(), expire);
}

string __generate_appender(const char *username) {
    char *hash = new char[33];
    ConfigManager c;
    mg_md5(hash, username, ":", c.global().get("security::salt").c_str(), NULL);
    // mg_md5(hash, username, ":", md5Salt, NULL);

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

string cc_smart_shorten(string& s, int limiter){
    //s = s.substr(0, limiter * 2);
    int nl_pos = s.find_first_of("<");
    s = s.substr(0, nl_pos);
    if(s.size() <= limiter) return s;

    string ret = s.substr(0, limiter);
    if(nl_pos <= limiter) return ret;

    for(int i = limiter; i < s.size(); ++i){
        if(i < s.size() - 3){
            if(s[i] == -17 && s[i+1] == -68 && s[i+2] == -116) break;
            if(s[i] == -29 && s[i+1] == -128 && s[i+2] == -126) break;
            if(s[i] == ',' || s[i] == '.') break;
        }
        ret += s[i];
    }

    return ret;
}

string cc_htmlify(string& s, bool strict) {
    s.append("\n");
    string ret;

    int last_pos = 0;
    int i = 0, j;
    while (i < s.size()) {
        if (s[i] == '>' && s[i + 1] == '>' && s[i + 2] == 'N' && s[i + 3] == 'o' && s[i + 4] == '.') {
            ret.append(s.substr(last_pos, i - last_pos));
            for (j = i + 5; j < s.size(); ++j) if (s[j] == '\n') break;
            string label = s.substr(i, j - i);
            string thread_no = label.substr(5);
            ret.append("<div class=div-thread-");
            ret.append(thread_no);
            ret.append("><a href='javascript:ajst(");
            ret.append(thread_no);
            ret.append(")'>");
            ret.append(label);
            ret.append("</a></div>");
            last_pos = j + 1;
            i = j + 1;
        }
        else if (s[i] == 'h' && s[i + 1] == 't' && s[i + 2] == 't' && s[i + 3] == 'p' && !strict) {
            ret.append(s.substr(last_pos, i - last_pos));
            for (j = i + 4; j < s.size(); ++j) {
                if (isalnum(s[j]) || string("/:.?#=~-_!@$%^&*()+[]{}';|\\").find(s[j]) != string::npos) {}
                else
                    break;
            }
            string tmp = s.substr(i, j - i);
            ret.append("<a href='" + tmp + "' target=_blank>" + tmp + "</a>");
            last_pos = j;
            i = j;
        }
        else if (s[i] == '>' && s[i + 1] != '>' && (i == 0 || s[i - 1] == '\n')) {
            for (j = i + 1; j < s.size(); ++j) if (s[j] == '\n') break;
            ret.append("<ttt>" + s.substr(i, j - i) + "</ttt>");
            last_pos = j;
            i = j;
        }
        else if (s[i] == '#' && (i == 0 || s[i - 1] == '\n') && !strict) {
            int h_ = 1;
            if(s[i + 1] == '#') h_ = 2;
            if(s[i + 2] == '#') h_ = 3;

            for (j = i + h_; j < s.size(); ++j) if (s[j] == '\n') break;
            ret.append("<h" + to_string(h_ + 1) + ">" + s.substr(i + h_, j - i - h_ + 1) + "</h" + to_string(h_ + 1) + ">");
            last_pos = j + 1;
            i = j + 1;
        }
        else if (s[i] == '(' && s[i + 1] == '#' && s[i + 8] == ' ' && !strict) {
            string clr = s.substr(i + 2, 6);
            for (j = i + 8; j < s.size(); ++j) if (s[j] == ')') break;
            ret.append("<span style='color:#" + clr + "'>");
            ret.append(s.substr(i + 9, j - i - 9));
            ret.append("</span>");
            last_pos = j + 1;
            i = j + 1;
        }
        else if (s[i] == '\n' || s[i] == '<' || s[i] == '>') {
            string dem = s[i] == '\n' ? "<br>" : (s[i] == '<' ? "&lt;" : "&gt;");
            ret.append(s.substr(last_pos, i - last_pos) + dem);
            last_pos = i + 1;
            i++;
        }
        else
            i++;
    }

    return ret;
}

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

int cc_extract_uri_num(mg_connection* conn){
    string url(conn->uri);
    vector<string> tmp = cc_split(url, "/");
    string num = tmp[tmp.size() - 1];

    return atol(num.c_str());
}

void cc_clean_string(string& str){
    str = cc_replace(str, string("<"), string("&lt;"));
    str = cc_replace(str, string(">"), string("&gt;"));
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

    ConfigManager c;

    char expire[100];
    struct tm exp;
    time_t t = time(NULL) + 60 * 60 * 24 * c.global().get<int>("image::expire");
    gmtime_r(&t, &exp);
    strftime(expire, sizeof(expire), "%a, %d %b %Y %H:%M:%S GMT", &exp);

    if (stat(path.c_str(), &st) == 0 && (fp = fopen(path.c_str(), "rb")) != NULL) {
        mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                        "Content-Type: %s\r\n"
                        "Last-Modified: Sat, 31 Jul 1993 00:00:00 GMT\r\n"
                        // "Expires: Tue, 31 Jul 2035 00:00:00 GMT\r\n"
                        "Expires: %s\r\n"
                        // "Cache-Control: must-revalidate\r\n"
                        "Cache-Control: Public\r\n"
                        "Content-Length: %lu\r\n\r\n", ctype.c_str(), expire, (unsigned int)st.st_size);
        while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
            mg_write(conn, buf, n);
        }
        fclose(fp);
        mg_write(conn, "\r\n", 2);
    }else{
        mg_printf(conn, "HTTP/1.1 404 Not Found\r\nContent-Type: image/jpeg\r\nContent-Length: 0\r\n\r\n");
    }   //image
}


void cc_load_file_to_set(const string& p, unordered_set<string>& s){
    ifstream f(p);
    string line;
    while (f >> line) if(line != "") s.insert(line);
    f.close();
}

void cc_store_set_to_file(const string& p, unordered_set<string>& s){
    ofstream f(p);
    for(auto i = s.begin(); i != s.end(); ++i) f << *i << '\n';
    f.close();
}