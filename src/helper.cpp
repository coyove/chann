#include "helper.h"

using namespace std;

extern unqlite* pDb;
extern FILE* log_file;

size_t szBuilder(char* buf, size_t buf_size, ...)
{
	va_list arg_ptr;
	char* arg;
	va_start(arg_ptr, buf_size);
	size_t len = strlen(buf);
	bool num_flag = false;
	arg = va_arg(arg_ptr, char*);

	while (arg){
		if (num_flag){
			cclong ret = va_arg(arg_ptr, cclong);
			char tmp_long[16] = { 0 };
			// logLog("%d", ret);
			sprintf(tmp_long, "%d", ret);
			arg = tmp_long;
			num_flag = false;
		}

		if (strcmp(arg, TO_STRING_PLACEHOLDER) == 0) {
			num_flag = true;
			// logLog("flag");
			continue;
		}

		len += strlen(arg);

		if (len + 1 > buf_size){
			size_t tmp = strlen(arg) - (len + 1 - buf_size);
			strncat(buf, arg, tmp);
			logLog("Buffer Overflow Detected: '%s'", buf);
			break;
		}
		else
			strcat(buf, arg);

		arg = va_arg(arg_ptr, char*);
	}
	
	va_end(arg_ptr);

	return len;
}

char* nowNow(){
	time_t rawtime;
	time(&rawtime);

	char *timetmp = new char[64];
	struct tm * timeinfo;
	timeinfo = localtime(&(rawtime));

	strftime(timetmp, 64, "%Y/%m/%d %X", timeinfo);
	return timetmp;//asctime(localtime(&rawtime));
}

void logLog(const char* msg, ...){
	fprintf(log_file, "[%s]\t", nowNow());

	va_list argptr;
	va_start(argptr, msg);
	vfprintf(log_file, msg, argptr);
	va_end(argptr);

	fprintf(log_file, "\n");
}

void Fatal(const char *zMsg)
{
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
vector<string> split(const string &s, const string &seperator){
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

string replaceAll(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

bool endsWith(std::string const &fullString, std::string const &ending) {
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

cclong extractLastNumber(mg_connection* conn){
	string url(conn->uri);
	vector<string> tmp = split(url, "/");
	string num = tmp[tmp.size() - 1];

	return atol(num.c_str());
}

void cleanString(string& str){
	str = replaceAll(str, string("<"), string("&lt;"));
	str = replaceAll(str, string(">"), string("&gt;"));
}