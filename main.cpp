//unsafe
#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <iterator>
#include <algorithm>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <fstream>
#include <sys/stat.h>

#include <Windows.h>
//#include <dbghelp.h>
//#include <shellapi.h>
//#include <shlobj.h>

#include "helper.h"
#include "general.h"

extern "C" {
#include "unqlite.h"
#include "mongoose.h"
#include "templates.h"
}

using namespace std;

unqlite *pDb;

static void Fatal(const char *zMsg)
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

int threadsPerPage = 5;			//threads per page to show
char* admin_pass;				// * administrator's password
char* site_title;				//site's title
char* md5_salt;					// * MD5 salt
time_t g_startuptime;			//server's startup time
long total_hit = 0;				//pages' total hits
int cd_time = 20;				//cooldown time
vector<string> banlist;			//cookie ban list
bool stop_newcookie = false;	//stop delivering new cookies
map<string, long> iplist;		//remote ip list

void printMsg(mg_connection* conn, const char* msg, ...){
	mg_printf_data(conn, html_header, site_title, site_title);
	
	char tmpbuf[512]; //=w=
	va_list argptr;
	va_start(argptr, msg);
	vsprintf(tmpbuf, msg, argptr);
	va_end(argptr);

	mg_printf_data(conn, html_error, tmpbuf);
	mg_printf_data(conn, "</body></html>");
}

char* nowNow(){
	time_t rawtime;
	time(&rawtime);
	return asctime(localtime(&rawtime));
}

char* generateSSID(const char *user_name, const char* time) {
	char *hash = new char[33];
	mg_md5(hash, user_name, ":", time, md5_salt, NULL);

	return hash;
}

bool checkIP(mg_connection* conn, bool verbose = false){
	time_t rawtime;
	time(&rawtime);
	string sip(conn->remote_ip);
	time_t lasttime = iplist[sip];
	//printf("%d\n", lasttime);
	//lasttime = lasttime ? lasttime : rawtime; // if lasttime = 0, meaning this ip is the first time accessing the server

	if( abs(lasttime - rawtime) < cd_time && lasttime != 0){
		printMsg(conn, "Cooldown time [%s], please wait", conn->remote_ip);
		if(verbose)
			printf("Rapid access [%s] at %s", conn->remote_ip, nowNow());
		return false;
	}
	iplist[sip] = rawtime;
	return true;
}

void sendThread(mg_connection* conn, struct Thread* r, bool reply = false, bool show_reply = true, bool cut_long = false){
	if (!(r->state & NORMAL_DISPLAY)) return;

	struct tm * timeinfo;
	char tmp[65536];
	timeinfo = localtime(&(r->date));
	int len;
	
	//the date and time
	char timetmp[64];
	strftime(timetmp, 64, "%Y-%m-%d %X", timeinfo);

	//a reply thread has an horizontal offset(20px) at left
	const char *width1 = reply ? "class='thread header' style='margin-left:20px'" : "class='thread' ";

	//flag to (reply to reply)
	char *reply_link;
	if (show_reply){ //display the reply link?
		//if (reply && r->childThread) // dose it have reply to reply?
		//	reply_link = "[<a class='red' href=\"/thread/%d\"><b>More</b></a>]";
		//else
		reply_link = "[<a href=\"/thread/%d\"><b>Reply</b></a>]";
	}
	else
		reply_link = "";

	char crl[128];
	int len2 = sprintf(crl, reply_link, r->threadID); crl[len2] = 0;

	//display the sage flag
	const char *sage = (r->state & SAGE_THREAD && !reply) ? "<font color='red'><b>&#128078;&nbsp;SAGE</b></font><br/>" : "";
	const char *locked = r->state & LOCKED_THREAD ? "<font color='red'><b>&#128274;&nbsp;Locked</b></font><br/>" : "";
	char *reply_count;
	if(r->childThread){
		struct Thread* c = readThread_(pDb, r->childThread);
		reply_count = new char[128];
		sprintf(reply_count, "<font color='darkcyan'>&#128265;&nbsp;<i>%d reply(s) total ...</i></font><br/>", c->childCount); 
	}else
		reply_count = "";

	//display the red name
	char display_ssid[64];
	if (strcmp(r->ssid, "Admin") == 0){
		strcpy(display_ssid, "<b><font color='red'>Admin</font></b>");
	}
	else
		strcpy(display_ssid, r->ssid);

	char display_image[128];
	if (strcmp(r->imgSrc, "") == 0 || strcmp(r->imgSrc, "x") == 0){
		strcpy(display_image, "");
	}
	else{
		int int5 = sprintf(display_image, "<div class='img'><a href='/images/%s'><img class='imgs' src='/images/%s'/></a></div>", r->imgSrc, r->imgSrc);
		display_image[int5] = 0;
	}

	//if the content is too long to display
	char c_content[1050];
	char* content = readString(pDb, r->content);
	strncpy(c_content, content, 1000);
	c_content[1000] = 0;

	if (strlen(content) > 1000){
		char *more_content = "<font color='red'><b>[more]</b></font>";
		strcat(c_content, more_content);
	}

	len = sprintf(tmp,
		"<div %s>"
		"%s"
		"<a href='/thread/%d'>No.%d</a>&nbsp;<font color='#228811'><b>%s</b></font> %s ID:<span class='uid'>%s</span> %s"
		"<div class='quote'>%s</div>"
		"%s"
		"%s"
		"%s"
		"</div>",
		width1, display_image, r->threadID, r->threadID, r->author, timetmp, display_ssid, crl,
		cut_long ? c_content : content, sage, locked, reply_count);

	mg_send_data(conn, tmp, len);
}

void showThreads(mg_connection* conn, long startID, long endID){
	struct Thread *r = readThread_(pDb, 0); // get the root thread
	long c = 0;
	clock_t startc = clock();

	long totalThreads = r->childCount;
	long totalPages;
	if (totalThreads <= threadsPerPage)
		totalPages = 1;
	else if (totalThreads % threadsPerPage == 0)
		totalPages = (long)(totalThreads / threadsPerPage);
	else
		totalPages = (long)(totalThreads / threadsPerPage) + 1;

	while (r->nextThread){
		r = readThread_(pDb, r->nextThread);
		c++;
		if (c >= startID && c <= endID){
			mg_printf_data(conn, "<hr>");
			sendThread(conn, r, false, true, true);

			struct Thread *oldr = r;

			if (r->childThread) {
				vector<struct Thread *> vt;

				r = readThread_(pDb, r->childThread); // beginning of the circle
				//vt.push_back(r);
				//mg_printf_data(conn, "<i>%d reply(s) total</i>", r->childCount);

				r = readThread_(pDb, r->prevThread);
				long rid = r->threadID; //the ID
				//sendThread(conn, r, true, false);
				vt.push_back(r);
				
				int counter = 5;

				while (r->prevThread != rid){
					if(--counter == 0) break;
					r = readThread_(pDb, r->prevThread);
					vt.push_back(r);
				}

				for (int i = vt.size() - 1; i >= 0; i--)
					sendThread(conn, vt[i], true, false);

			}

			r = oldr;
		}
		
		if (c == endID + 1) break;
	}
	mg_printf_data(conn, "<hr>");
	//mg_send_data(conn, "<table><tr>", 11);

	long current_page = endID / threadsPerPage;

	for (long i = 1; i <= totalPages; ++i){
		char tmp[256];
		int len;

		if (abs(current_page - i) <= 5){
			if (i == current_page) //current page
				len = sprintf(tmp, "<td>%d</td>", i);
			else
				if (i == 1) 
					len = sprintf(tmp, "<a class='pager' href=\"/\">1</a>");
				else
					len = sprintf(tmp, "<a class='pager' href=\"/page/%d\">%d</a>", i, i);

			mg_send_data(conn, tmp, len);
		}
	}
	//mg_send_data(conn, "</tr></table>", 13);
	mg_printf_data(conn, "<br/><br/>");
	
	clock_t endc = clock();
	mg_printf_data(conn, "Completed in %.3lf s<br/>",(float)(endc - startc) / CLOCKS_PER_SEC);
}

void showThread(mg_connection* conn, long id){
	clock_t startc = clock();

	struct Thread *r = readThread_(pDb, id); // get the root thread
	long c = 0;

	if (id == 0) return;

	sendThread(conn, r, false, false);
	mg_printf_data(conn, "<hr>");

	char zztmp[512], zztmp2[512];
	int len = sprintf(zztmp, "/post_reply/%d", id); zztmp[len] = 0;

	mg_printf_data(conn, html_form, zztmp, "[Post a Reply]");
		
	if (r->childThread) {
		r = readThread_(pDb, r->childThread); // beginning of the circle
		long rid = r->threadID; //the ID

		//mg_printf_data(conn, "<i>%d reply(s) total</i><hr>", r->childCount);
		sendThread(conn, r , true);

		while (r->nextThread != rid){
			//printf("%d\n", r->nextThread);
			r = readThread_(pDb, r->nextThread);

			sendThread(conn, r, true);
		}

	}
	/*else{
		mg_printf_data(conn, "<i>no reply yet</i><hr>", r->childCount);
	}*/

	clock_t endc = clock();
	mg_printf_data(conn, "Completed in %f s<br/>", (float)(endc - startc) / CLOCKS_PER_SEC);
}

void setCookie(mg_connection *conn, char *ssid){
	char expire[100];
	time_t t = time(NULL) + 60 * 60 * 24 * 30;
	strftime(expire, sizeof(expire), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));

	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-type: text/html\r\n"
		"Set-Cookie: ssid=%s; max-age=%d; expire=%s; path=/; http-only; HttpOnly;\r\n"
		"Content-Length: 0\r\n",
		ssid, 60 * 60 * 24 * 30, expire);
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

	time_t rawtime;
	time(&rawtime);
	char timebuf[16];
	int c = sprintf(timebuf, "%lu", rawtime); timebuf[c] = 0;

	char *newssid = generateSSID(username, timebuf);
	char finalssid[64];

	int len = sprintf(finalssid, "%s|%s|%d", username, newssid, rawtime);

	setCookie(conn, finalssid);

	printf("New cookie delivered: [%s] at %s", finalssid, nowNow());

	return username;
}

int renewCookie(mg_connection* conn, const char* username){
	time_t rawtime;
	time(&rawtime);
	char timebuf[16];
	int c = sprintf(timebuf, "%lu", rawtime); timebuf[c] = 0;

	char *newssid = generateSSID(username, timebuf);
	char finalssid[64];

	int len = sprintf(finalssid, "%s|%s|%d", username, newssid, rawtime);
	finalssid[len] = 0;

	setCookie(conn, finalssid);

	return 1;
}

void userDeleteThread(mg_connection* conn, long tid, bool admin = false){
	char ssid[128], username[10];
	mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, sizeof(ssid));
	vector<string> zztmp = split(string(ssid), string("|"));
	if (zztmp.size() != 3){
		printMsg(conn, "Your cookie is invalid");
		return;
	}

	strncpy(username, zztmp[0].c_str(), 10);
	char *finalssid = generateSSID(username, zztmp[2].c_str());
	if (strcmp(finalssid, zztmp[1].c_str()) == 0){
		struct Thread* t = readThread_(pDb, tid);

		if (strcmp(t->ssid, username) == 0 || admin){
			deleteThread(pDb, tid);
			printMsg(conn, "You have deleted thread No.%d", tid);
			printf("Thread Deleted: [%d] at %s", tid, nowNow());
		}
		else{
			printMsg(conn, "Your cookie doesn't match the thread");
			printf("Trying to Delete Thread: [%d] at %s", tid, nowNow());
		}
	}
}

void postSomething(mg_connection* conn, const char* uri){
	char var1[64], var2[32768], var3[64], var4[16];
	bool sage = false;
	bool fileAttached = false;
	bool user_delete = false;
	const char *data;
	int data_len, ofs = 0, mofs = 0;
	char var_name[100], file_name[100];

	//first check the ip
	if(!checkIP(conn)) return;

	//get the post form data
	//var1: the subject of the thread
	//var2: the comment
	//var3: the e-mail
	//var4: the image attached (if has)
	while ((mofs = mg_parse_multipart(conn->content + ofs, conn->content_len - ofs,
		var_name, sizeof(var_name),
		file_name, sizeof(file_name),
		&data, &data_len)) > 0) {
		
		ofs += mofs;

		if (strcmp(var_name, "input_name") == 0) {
			data_len = data_len > 63 ? 63 : data_len;
			strncpy(var1, data, data_len);
			var1[data_len] = 0;
		}

		if (strcmp(var_name, "input_email") == 0) {
			data_len = data_len > 63 ? 63 : data_len;
			strncpy(var3, data, data_len);
			var3[data_len] = 0;
		}

		if (strcmp(var_name, "input_content") == 0) {
			data_len = data_len > 32767 ? 32767 : data_len;
			strncpy(var2, data, data_len);
			var2[data_len] = 0;
		}

		strcpy(var4, "");

		if (strcmp(var_name, "input_file") == 0 && strcmp(file_name, "") != 0) {
			if (data_len > 1024 * 1024 * 2){
				printMsg(conn, "File is too big");
				return;
			}

			char fname[12];
			unqlite_util_random_string(pDb, fname, 11);
			fname[11] = 0;
			string sfname(file_name), sfnamep(fname);

			std::transform(sfname.begin(), sfname.end(), sfname.begin(), ::tolower);

			string ext;
			if (endsWith(sfname, ".jpg"))
				ext = ".jpg";
			else if (endsWith(sfname, ".gif"))
				ext = ".gif";
			else if (endsWith(sfname, ".png"))
				ext = ".png";
			else{
				printMsg(conn, "Invalid file format");
				return;
			}

			fileAttached = true;
			string fpath = "images\\" + sfnamep + ext;

			FILE *fp = fopen(fpath.c_str(), "wb");
			fwrite(data, 1, data_len, fp);
			fclose(fp);

			strcpy(var4, (sfnamep + ext).c_str());
			printf("Image uploaded: [%s] at %s", var4, nowNow());
		}
	}

	//see if there is a SPECIAL string in the text field
	if (strcmp(var1, "") == 0) strcpy(var1, "Untitled");
	//user trying to sega a thread/reply
	if (strcmp(var3, "sage") == 0) sage = true;
	//user trying to delete a thread/reply
	if (strcmp(var1, "delete") == 0){
		long id = extractLastNumber(conn);
		struct Thread * t = readThread_(pDb, id); //what he replies to is which he wants to delete
		if (strcmp(var3, admin_pass) == 0)
			userDeleteThread(conn, id, true);
		else if (strcmp(var3, t->email) == 0)
			userDeleteThread(conn, id);
		else
			printMsg(conn, "Failed");

		return;
	}
	//admin trying to update a thread/reply
	if (strcmp(var1, "update") == 0 && strcmp(var3, admin_pass) == 0){
		long id = extractLastNumber(conn);
		struct Thread * t = readThread_(pDb, id); //what admin replies to is which he wants to update
		//note the server doesn't filter the special chars such as "<" and ">"
		//so it's possible for admin to use HTML here
		writeString(pDb, t->content, var2, true); 
		printMsg(conn, "Updated successfully");
		printf("Admin edited thread no.%d at %s", t->threadID, nowNow());
		return;
	}
	//admin trying to sage a thread/reply
	if (strstr(var1, "sage") && strcmp(var3, admin_pass) == 0){
		long tid = extractLastNumber(conn);
		struct Thread* t = readThread_(pDb, tid);

		if(findParent(pDb, tid) == 0){
			//t->state = strcmp(var1, "unsage") == 0 ? 'm' : 's';
			if(strcmp(var1, "unsage") == 0)
				t->state -= SAGE_THREAD;
			else
				t->state += SAGE_THREAD;

			writeThread(pDb, tid, t, true);
			printMsg(conn, "You have %ssaged thread No.%d.", strcmp(var1, "unsage") == 0 ? "un" : "", tid);
			printf("Thread %sSaged: [%d] at %s", strcmp(var1, "unsage") == 0 ? "un" : "", tid, nowNow());
		}else
			printMsg(conn, "You can't sage a reply");

		return;
	}
	//admin trying to lock a thread/reply
	if (strstr(var1, "lock") && strcmp(var3, admin_pass) == 0){
		long tid = extractLastNumber(conn);
		struct Thread* t = readThread_(pDb, tid);

		if(findParent(pDb, tid) == 0){
			//t->state = strcmp(var1, "unlock") == 0 ? 'm' : 'l';
			if(strcmp(var1, "unlock") == 0)
				t->state -= LOCKED_THREAD;
			else
				t->state += LOCKED_THREAD;
			writeThread(pDb, tid, t, true);
			printMsg(conn, "You have %slocked thread No.%d.", strcmp(var1, "unlock") == 0 ? "un" : "", tid);
			printf("Thread %slocked: [%d] at %s", strcmp(var1, "unlock") == 0 ? "un" : "", tid, nowNow());
		}else
			printMsg(conn, "You can't lock a reply");

		return;
	}
	//image or comment or both
	if (strcmp(var2, "") == 0 && !fileAttached) {
		printMsg(conn, "Please enter something");
		return;
	}
	//verify the cookie
	char ssid[128], expire[128], expire_epoch[128], username[10];
	mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, sizeof(ssid));

	if (strcmp(ssid, "") == 0){
		//user doesn't have a cookie, we give one
		if (stop_newcookie){
			printMsg(conn, "The board has stopped delivering new cookies");
			return;
		}
		else
			strcpy(username, giveNewCookie(conn));
	}
	else{
		vector<string> tmp = split(string(ssid), string("|"));
		if (tmp.size() != 3){
			giveNewCookie(conn);
			printMsg(conn, "Your cookie is invalid");
			printf("Invalid cookie detected: [%s] at %s", ssid, nowNow());
			return;
		}

		strncpy(username, tmp[0].c_str(), 10);

		auto iter = find(banlist.begin(), banlist.end(), username);
		
		if (iter != banlist.end()){
			//this id is banned, so we destory it
			destoryCookie(conn);
			printf("Cookie Destoryed: [%s] at %s", ssid, nowNow());
			printMsg(conn, "Your ID is banned");
			return;
		}

		char *testssid = generateSSID(username, tmp[2].c_str());
		if (strcmp(testssid, tmp[1].c_str()) != 0){
			giveNewCookie(conn);
			printMsg(conn, "Your cookie is broken");
			return;
		}

		time_t nowtime;
		time(&nowtime);

		if (nowtime - atol(tmp[2].c_str()) < cd_time){
			printMsg(conn, "Cooldown time, please wait");
			return;
		}
		else{
			renewCookie(conn, username);
		}
	}

	if (strcmp(var3, admin_pass) == 0) strcpy(username, "Admin");

	//replace some important things
	string tmpcontent(var2);	cleanString(tmpcontent);
	string tmpname(var1);		cleanString(tmpname);
	string tmpemail(var3);		cleanString(tmpemail);

	strncpy(var1, tmpname.c_str(), 64);
	strncpy(var3, tmpemail.c_str(), 64);

	vector<string> imageDetector = split(tmpcontent, "\n");
	tmpcontent = "";

	for (auto i = 0; i < imageDetector.size(); ++i){
		if (startsWith(imageDetector[i], "http"))
			imageDetector[i] = "<a href='" + imageDetector[i] + "'>" + imageDetector[i] + "</a>";

		if (startsWith(imageDetector[i], "&gt;&gt;No.")){
			vector<string> gotoLink = split(imageDetector[i], string("."));
			if (gotoLink.size() == 2)
				imageDetector[i] = "<a href='/thread/" + gotoLink[1] + "'>" + imageDetector[i] + "</a>";
		}

		if (startsWith(imageDetector[i], "&gt;"))
			imageDetector[i] = "<font color='#789922'>" + imageDetector[i] + "</font>";
		
		tmpcontent += (imageDetector[i] + "<br/>");
	}

	if (strstr(conn->uri, "/post_reply/")) {
		long id = extractLastNumber(conn);
		struct Thread* t = readThread_(pDb, id);

		if(t->state & LOCKED_THREAD)
			printMsg(conn, "You can't reply to a locked thread");
		else{
			newReply(pDb, id, tmpcontent.c_str(), var1, var3, username, var4, sage);
			mg_printf_data(conn, html_header, site_title, site_title);
			mg_printf_data(conn, html_redirtothread, id, id, id);
			mg_printf_data(conn, "</body></html>");
		}
	}
	else{
		newThread(pDb, tmpcontent.c_str(), var1, var3, username, var4, sage);
		//mg_send_data(conn, html_redir, strlen(html_redir));
		printMsg(conn, "Successfully start a new thread");
	}
}

void returnPage(mg_connection* conn, bool indexPage){
	mg_send_header(conn, "charset", "utf-8");
	mg_printf_data(conn, html_header, site_title, site_title);
	mg_printf_data(conn, html_form, "/post_thread", "[Start a New Thread]");

	if(indexPage)
		showThreads(conn, 1, threadsPerPage);
	else{
		long j = extractLastNumber(conn) * threadsPerPage;
		long i = j - threadsPerPage + 1;
		showThreads(conn, i, j);
	}

	char ssid[128];
	mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, sizeof(ssid));
	mg_printf_data(conn, "<small><span class='state'>%s</span></small><br/>", ssid);
	time_t nn;
	time(&nn);
	mg_printf_data(conn, "<small>last restart: %.5lfd ago, ", difftime(nn, g_startuptime) / 3600 / 24);
	mg_printf_data(conn, "total hit: %d, ", total_hit);
	mg_printf_data(conn, "board state: %s, ", stop_newcookie ? "closed" : "open");
	mg_printf_data(conn, "cooldown time: %ds</small>", cd_time);
	mg_send_data(conn, "</html></body>", 14);
}

static void send_reply(struct mg_connection *conn) {
	total_hit++;

	if (strcmp(conn->uri, "/post_thread") == 0) {
		postSomething(conn, conn->uri);
	}
	else if (strstr(conn->uri, "/page/")) {
		returnPage(conn, false);
	}
	else if (strstr(conn->uri, "/thread/")) {
		mg_send_header(conn, "charset", "utf-8");
		mg_printf_data(conn, html_header, site_title, site_title);

		long id = extractLastNumber(conn);
		long pid = findParent(pDb, id);

		if (pid == -1){
			printMsg(conn, "This thread has been deleted.");
			return ;
		}
		else{
			if (pid)
				mg_printf_data(conn, "<a href='/thread/%d'>&lt;&lt; No.%d</a><hr>", pid, pid);
			else
				mg_printf_data(conn, "<a href='/'>&lt;&lt; Homepage</a><hr>");

			showThread(conn, id);
		}
		mg_send_data(conn, "</html></body>", 14);
	}
	else if (strstr(conn->uri, "/post_reply/")) {
		postSomething(conn, conn->uri);
	}
	else if (strstr(conn->uri, "/new_cookie/")) {
		/*string url(conn->uri);
		vector<string> tmp = split(url, "/");
		string ssid = tmp[tmp.size() - 1];

		mg_printf(conn,
			"HTTP/1.1 200 OK\r\n"
			"Content-type: text/html\r\n"
			"Set-Cookie: ssid=%s; max-age=%d; http-only; HttpOnly;\r\n"
			"Content-Length: 0\r\n",
			ssid.c_str(), 60 * 60 * 24 * 30);

		mg_printf_data(conn, "Transfer a new cookie: %s", ssid.c_str());*/
	}
	else if (strstr(conn->uri, "/state/")){
		if(!checkIP(conn, true)) return;

		string url(conn->uri);
		vector<string> tmp = split(url, "/");

		long newstate = atol(tmp[tmp.size() - 1].c_str());
		long tid  = atol(tmp[tmp.size() - 2].c_str());

		if (strstr(conn->uri, admin_pass)){
			struct Thread * t = readThread_(pDb, tid);
			t->state = newstate;
			writeThread(pDb, t->threadID, t, true);
			printMsg(conn, "Thread's state updated successfully");
		}else{
			printMsg(conn, "Your don't have the permission");
			checkIP(conn, true);
		}
	}
	else if (strstr(conn->uri, "/delete/")){
		long id = extractLastNumber(conn);
		struct Thread * t = readThread_(pDb, id); //what he replies to is which he wants to delete

		if (strstr(conn->uri, admin_pass))
			userDeleteThread(conn, id, true);
		else{
			printMsg(conn, "Your don't have the permission");
			checkIP(conn, true);
		}
		
	}
	else if (strstr(conn->uri, "/ban/")){
		string url(conn->uri);
		vector<string> tmp = split(url, "/");
		string id = tmp[tmp.size() - 1];

		if (strstr(conn->uri, admin_pass)){
			auto iter = find(banlist.begin(), banlist.end(), id);
			if (iter == banlist.end()){
				banlist.push_back(id);
				printMsg(conn, "You have banned ID: %s.", id.c_str());
				printf("ID Banned: [%s] at %s", id.c_str(), nowNow());
			}
			else{
				banlist.erase(iter);
				printMsg(conn, "You have unbanned ID: %s.", id.c_str());
				printf("ID Unbanned: [%s] at %s", id.c_str(), nowNow());
			}
		}
		else{
			printMsg(conn, "Your don't have the permission.");
			printf("Trying to (Un)Banned ID: [%s] at %s", id.c_str(), nowNow());
			checkIP(conn, true);
		}

	}
	else if (strstr(conn->uri, "/images/")){
		const char *ims = mg_get_header(conn, "If-Modified-Since");
		//printf("[%s]\n", ims);
		if (ims)
			if (strcmp(ims, "") != 0){
				//browser should have a cache, since we are running a semi-static imageboard, 
				//we use cache whatever the situation is.
				mg_printf(conn, "HTTP/1.1 304 Not Modified\r\n\r\n");
				return;
			}

		string url(conn->uri);
		vector<string> tmp = split(url, "/");
		char ipath[64];
		strcpy(ipath, ("images\\" + tmp[tmp.size() - 1]).c_str());

		FILE *fp; struct stat st;
		char buf[1024];
		int n;

		char expire[100];
		time_t t = time(NULL) + 60 * 60 * 24 * 30;
		strftime(expire, sizeof(expire), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));

		if (stat(ipath, &st) == 0 && (fp = fopen(ipath, "rb")) != NULL) {
			mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\n"
				"Last-Modified: Sat, 31 Jul 1993 00:00:00 GMT\r\n"
				"Expires: %s\r\n"
				"Cache-Control: max-age=2592000\r\n"
				"Content-Length: %lu\r\n\r\n", expire, (unsigned long)st.st_size);
			while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
				mg_write(conn, buf, n);
			}
			fclose(fp);
			mg_write(conn, "\r\n", 2);
		}else{
			mg_printf(conn, "HTTP/1.1 404 Not Found\r\nContent-Type: image/jpeg\r\nContent-Length: 0\r\n\r\n");
		}
	}
	else if (strstr(conn->uri, "/cookie/")){
		if (strstr(conn->uri, admin_pass)){
			stop_newcookie = strstr(conn->uri, "/close/") ? true: false;
			printMsg(conn, "Your have closed/opened new cookie delivering.");
			printf("Cookie closed/opened at %s", nowNow());
		}else{
			printMsg(conn, "Your don't have the permission.");
			checkIP(conn, true);
		}
	}
	else 
		returnPage(conn, true);
}

static int ev_handler(struct mg_connection *conn, enum mg_event ev) {
	if (ev == MG_REQUEST) {
		send_reply(conn);
		return MG_TRUE;
	}
	else if (ev == MG_AUTH) {
		return MG_TRUE;
	}
	else {
		return MG_FALSE;
	}
}

int main(int argc, char *argv[])
{
	//SetErrorMode(SEM_NOGPFAULTERRORBOX);

	site_title = ""; //&#21311;&#21517;&#29256;
	threadsPerPage = 5;
	char *lport = "8080";
	char *zPath = "default.db"; 
	
	for (int i = 0; i < argc; ++i){
		if (strcmp(argv[i], "-database") == 0){
			zPath = new char[256];
			strcpy(zPath, argv[++i]);
			break;
		}
	}

	int rc = unqlite_open(&pDb, zPath, UNQLITE_OPEN_CREATE);
	if (rc != UNQLITE_OK) Fatal("Out of memory");

	printf("Database: \t\t[%s]\n", zPath);

	//generate a random admin password
	admin_pass = new char[11];
	unqlite_util_random_string(pDb, admin_pass, 10);
	admin_pass[10] = 0;
	md5_salt = "coyove";

	for (int i = 0; i < argc; ++i){
		if (strcmp(argv[i], "-newprofile") == 0) {
			printf("Database: \t\t[New Profile]\n");
			resetDatabase(pDb);
		}

		if (strcmp(argv[i], "-title") == 0){
			//delete site_title;
			site_title = new char[64];
			strcpy(site_title, argv[++i]);
			continue;
		}

		if (strcmp(argv[i], "-spell") == 0){
			strcpy(admin_pass, argv[++i]);
			continue;
		}

		if (strcmp(argv[i], "-tpp") == 0){
			threadsPerPage = atoi(argv[++i]);
			continue;
		}

		if (strcmp(argv[i], "-cd") == 0){
			cd_time = atoi(argv[++i]);
			continue;
		}

		if (strcmp(argv[i], "-port") == 0){
			lport = new char[6];
			strcpy(lport, argv[++i]);
			continue;
		}

		if (strcmp(argv[i], "-salt") == 0){
			md5_salt = new char[9];
			strcpy(md5_salt, argv[++i]);
			continue;
		}

		if (strcmp(argv[i], "-opencookie") == 0)
			stop_newcookie = false;

		if (strcmp(argv[i], "-closecookie") == 0)
			stop_newcookie = true;

		if (strcmp(argv[i], "-NOGPFAULTERRORBOX") == 0)
			SetErrorMode(SEM_NOGPFAULTERRORBOX);
	}

	printf("Site Title: \t\t[%s]\nSite Admin Password: \t[%s]\n", site_title, admin_pass);
	printf("Threads Per Page: \t[%d]\n", threadsPerPage);
	printf("Cooldown Time: \t\t[%ds]\n", cd_time);
	printf("MD5 Salt: \t\t[%s]\n", md5_salt);

	struct mg_server *server = mg_create_server(NULL, ev_handler);

	mg_set_option(server, "listening_port", lport);

	printf("Listening on Port: \t[%s]\n", mg_get_option(server, "listening_port"));

	time(&g_startuptime);

	for (;;) {
		mg_poll_server(server, 1000);
	}

	mg_destroy_server(&server);
	unqlite_close(pDb);

	return 0;
}