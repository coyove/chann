//unsafe
#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <unordered_set>
#include <sys/stat.h>
#include <fstream>

extern "C" {
#include "unqlite.h"
#include "mongoose.h"
#include "templates.h"
}

#if defined(_WIN32)
#include <Windows.h>
#include <Psapi.h>
#endif

#include "general.h"
#include "helper.h"
#include "cookie.h"

using namespace std;

unqlite *pDb;
int threadsPerPage = 5;			//threads per page to show
char* admin_pass;				// * administrator's password
char* site_title;				//site's title
char* md5_salt;					// * MD5 salt
time_t g_startuptime;			//server's startup time
cclong total_hit = 0;				//pages' total hits
int cd_time = 20;				//cooldown time
unordered_set<string> banlist;	//cookie ban list
unordered_set<string> ipbanlist;//ip ban list
char* ipbanlist_path;			//file to store ip ban list
bool stop_newcookie = false;	//stop delivering new cookies
map<string, cclong> iplist;		//remote ip list
char adminCookie[64];			//Admin's cookie
FILE* log_file;					//log file

void printFooter(mg_connection* conn){
	char * footer = readString(pDb, "footer");
	if(footer){
		mg_printf_data(conn, "<div id='footer'>%s</div>", footer);
		delete [] footer;
	}
	mg_printf_data(conn, "</body></html>");
}

void printMsg(mg_connection* conn, const char* msg, ...){
	mg_printf_data(conn, html_header, site_title, site_title);
	
	char tmpbuf[512]; //=w=
	va_list argptr;
	va_start(argptr, msg);
	vsprintf(tmpbuf, msg, argptr);
	va_end(argptr);

	mg_printf_data(conn, html_error, tmpbuf);
	printFooter(conn);
}

bool checkIP(mg_connection* conn, bool verbose = false){
	time_t rawtime;
	time(&rawtime);
	string sip(conn->remote_ip);
	time_t lasttime = iplist[sip];
	
	if(strstr(conn->uri, admin_pass)) return true;
	if(ipbanlist.find(sip) != ipbanlist.end()) {
		printMsg(conn, "Your IP is banned");
		return false; //banned
	}

	char ssid[128];
	mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, sizeof(ssid));
	if(strcmp(ssid, adminCookie) == 0) return true;

	if( abs(lasttime - rawtime) < cd_time && lasttime != 0){
		printMsg(conn, "Cooldown time [%s], please wait", conn->remote_ip);
		if(verbose)
			fprintf(log_file, "Rapid access [%s] at %s", conn->remote_ip, nowNow());
		return false;
	}
	iplist[sip] = rawtime;
	return true;
}

bool verifyAdmin(mg_connection* conn){
	char ssid[128]; 
	mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, sizeof(ssid));
	return (strcmp(ssid, adminCookie) == 0);
}

void doThread(mg_connection* conn, cclong tid, char STATE){
	struct Thread* t = readThread_(pDb, tid);

	if(findParent(pDb, tid) == 0){
		changeState(t, STATE, !(t->state & STATE));
	
		writeThread(pDb, tid, t, true);
		printMsg(conn, "Thread No.%d new state: %s", tid, resolveState(t->state));
		fprintf(log_file, "Thread [%d] new state: %s at %s", tid, resolveState(t->state), nowNow());
	}else
		printMsg(conn, "You can't sage or lock a reply");
}

void sendThread(mg_connection* conn, struct Thread* r, 
				bool reply = false, 
				bool show_reply = true, 
				bool cut_cclong = false, 
				bool cut_image = false,
				bool admin_view = false){
	if (!(r->state & NORMAL_DISPLAY)) return;

	struct tm * timeinfo;
	char tmp[65536];
	timeinfo = localtime(&(r->date));
	int len;
	
	//the date and time
	char timetmp[64];
	strftime(timetmp, 64, "%Y-%m-%d %X", timeinfo);

	char crl[128], sage[128], locked[128], display_ssid[64], width1[128];
	sprintf(crl, show_reply ? "[<a href=\"/thread/%d\"><b>Reply</b></a>]" : "", r->threadID); 
	//a reply thread has an horizontal offset(20px) at left
	strcpy(width1, reply ? "class='thread header' style='margin-left:20px'" : "class='thread' ");
	//display the sage flag
	strcpy(sage, (r->state & SAGE_THREAD && !reply) ? "<font color='red'><b>&#128078;&nbsp;SAGE</b></font><br/>" : "");
	//display the lock flag
	strcpy(locked, r->state & LOCKED_THREAD ? "<font color='red'><b>&#128274;&nbsp;Locked</b></font><br/>" : "");
	//display the red name
	strcpy(display_ssid, (strcmp(r->ssid, "Admin") == 0) ? "<b><font color='red'>Admin</font></b>" : r->ssid);

	char reply_count[128];
	if(r->childThread){
		struct Thread* c = readThread_(pDb, r->childThread);
		sprintf(reply_count, "<font color='darkcyan'>&#128172;&nbsp;<i>%d reply(s) total ...</i></font><br/>", c->childCount); 
	}else
		strcpy(reply_count, "");

	char display_image[128];
	if (strcmp(r->imgSrc, "") == 0 || strcmp(r->imgSrc, "x") == 0)
		strcpy(display_image, "");
	else{
		if(cut_image){
			struct stat st;
			string filename(r->imgSrc);
			stat(("images/" + filename).c_str(), &st);

			sprintf(display_image, "<div class='img'><a href='/images/%s'>[View Image (%d kb)]</a></div>", 
				r->imgSrc, st.st_size / 1024);

		}
		else
			sprintf(display_image, "<div class='img'><a href='/images/%s'><img class='imgs' src='/images/%s'/></a></div>", r->imgSrc, r->imgSrc);
	}

	char admin_ctrl[512];
	if(admin_view){
		sprintf(admin_ctrl, "<br/><span class='admin'>["
							"<a href='/list/ip/%s'>%s</a>,<a href='/ban/ip/%s'>&#10006;</a>,"
							"<a href='http://ip.chinaz.com/?IP=%s'>&#127760;</a>|"
							"<a href='/sage/%d'>&#128078;</a>,"
							"<a href='/lock/%d'>&#128274;</a>,"
							"<a href='/delete/%d'>&#10006;</a>,"
							"<a href='/rename/%s'>&#10006;IMG</a>|"
							"<a href='/list/%s'>%s</a>,"
							"<a href='/ban/%s'>&#10006;</a>"
							"]</span>",
							r->email, r->email, r->email, r->email,
							r->threadID, r->threadID, r->threadID, 
							r->imgSrc, 
							r->ssid, r->ssid, r->ssid);
	}
	else
		strcpy(admin_ctrl, "");

	//if the content is too cclong to display
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
		"<a href='/thread/%d'>No.%d</a>&nbsp;<font color='#228811'><b>%s</b></font> %s ID:<span class='uid'>%s</span> %s %s"
		"<div class='quote'>%s</div>"
		"%s"
		"%s"
		"%s"
		"</div>",
		width1, 
		display_image, 
		r->threadID, r->threadID, r->author, timetmp, display_ssid, crl, admin_ctrl,
		cut_cclong ? c_content : content, 
		sage, 
		locked, 
		reply_count);

	mg_send_data(conn, tmp, len);
}

void showThreads(mg_connection* conn, cclong startID, cclong endID){
	struct Thread *r = readThread_(pDb, 0); // get the root thread
	cclong c = 0;
	clock_t startc = clock();

	bool admin_view = verifyAdmin(conn);

	cclong totalThreads = r->childCount;
	cclong totalPages;
	if (totalThreads <= threadsPerPage)
		totalPages = 1;
	else if (totalThreads % threadsPerPage == 0)
		totalPages = (cclong)(totalThreads / threadsPerPage);
	else
		totalPages = (cclong)(totalThreads / threadsPerPage) + 1;

	char *slogan = readString(pDb, r->content);
	if(slogan) {
		mg_printf_data(conn, "<div id='slogan'>%s</div>", slogan);
		delete [] slogan;
	}

	while (r->nextThread){
		cclong tmpid = r->nextThread;
		delete r;
		r = readThread_(pDb, tmpid);
		c++;
		if (c >= startID && c <= endID){
			mg_printf_data(conn, "<hr>");
			sendThread(conn, r, false, true, true, false, admin_view);

			struct Thread *oldr = r;

			if (r->childThread) {
				vector<struct Thread *> vt;

				r = readThread_(pDb, r->childThread); // beginning of the circle
				//vt.push_back(r);
				//mg_printf_data(conn, "<i>%d reply(s) total</i>", r->childCount);

				r = readThread_(pDb, r->prevThread);
				cclong rid = r->threadID; //the ID
				//sendThread(conn, r, true, false);
				vt.push_back(r);
				
				int counter = 4;

				while (r->prevThread != rid){
					//if(--counter == 0) break;
					r = readThread_(pDb, r->prevThread);
					if(r->state & NORMAL_DISPLAY){
						vt.push_back(r);	
						if(--counter == 0) break;
					}
				}

				for (int i = vt.size() - 1; i >= 0; i--){
					sendThread(conn, vt[i], true, false, false, false, admin_view);
					delete vt[i];
				}

			}

			r = oldr;
		}
		
		if (c == endID + 1) break;
	}
	if(r) delete r;
	mg_printf_data(conn, "<hr>");

	cclong current_page = endID / threadsPerPage;

	for (cclong i = 1; i <= totalPages; ++i){
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
	mg_printf_data(conn, "Completed in %.3lfs<br/>",(float)(endc - startc) / CLOCKS_PER_SEC);
}

void showThread(mg_connection* conn, cclong id){
	clock_t startc = clock();

	struct Thread *r = readThread_(pDb, id); // get the root thread
	cclong c = 0;

	if (id == 0) return;

	bool admin_view = verifyAdmin(conn);

	sendThread(conn, r, false, false, false, false, admin_view);
	mg_printf_data(conn, "<hr>");

	char zztmp[512];
	int len = sprintf(zztmp, "/post_reply/%d", id); zztmp[len] = 0;

	mg_printf_data(conn, html_form, zztmp, "[Post a Reply]");
		
	if (r->childThread) {
		cclong r_childThread = r->childThread;
		delete r;

		r = readThread_(pDb, r_childThread); // beginning of the circle
		cclong rid = r->threadID; //the ID
		bool too_many_replies = (r->childCount > 20);

		sendThread(conn, r , true, true, false, false, admin_view);

		int di = 1;
		while (r->nextThread != rid){
			cclong r_nextThread = r->nextThread;
			delete r;

			r = readThread_(pDb, r_nextThread);
			di++;

			sendThread(conn, r, true, true, false, too_many_replies && (di > 20), admin_view);
		}

		if(r) delete r;
	}
	/*else{
		mg_printf_data(conn, "<i>no reply yet</i><hr>", r->childCount);
	}*/

	clock_t endc = clock();
	mg_printf_data(conn, "Completed in %.3lfs<br/>", (float)(endc - startc) / CLOCKS_PER_SEC);
}

void userDeleteThread(mg_connection* conn, cclong tid, bool admin = false){
	char ssid[128], username[10];
	mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, sizeof(ssid));
	vector<string> zztmp = split(string(ssid), string("|"));
	if (zztmp.size() != 2){
		printMsg(conn, "Your cookie is invalid");
		return;
	}

	strncpy(username, zztmp[0].c_str(), 10);
	char testssid[33];
	strcpy(testssid, generateSSID(username));

	if (strcmp(testssid, zztmp[1].c_str()) == 0){
		struct Thread* t = readThread_(pDb, tid);

		if (strcmp(t->ssid, username) == 0 || admin){
			deleteThread(pDb, tid);
			printMsg(conn, "You have deleted thread No.%d", tid);
			fprintf(log_file, "Thread Deleted: [%d] at %s", tid, nowNow());
		}
		else{
			printMsg(conn, "Your cookie doesn't match the thread");
			fprintf(log_file, "Trying to Delete Thread: [%d] at %s", tid, nowNow());
		}

		if(t) delete t;
	}else
		printMsg(conn, "Your cookie is invalid");
}

void userListThread(mg_connection* conn){
	char ssid[128], username[10];
	mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, sizeof(ssid));
	vector<string> zztmp = split(string(ssid), string("|"));
	if (zztmp.size() != 2){
		printMsg(conn, "Your cookie is invalid");
		return;
	}

	strncpy(username, zztmp[0].c_str(), 10);
	char testssid[33];
	strcpy(testssid, generateSSID(username));

	if (strcmp(testssid, zztmp[1].c_str()) == 0){
		struct Thread *r = readThread_(pDb, 0); 
		struct Thread *t;

		mg_printf_data(conn, html_header, site_title, site_title);
		clock_t startc = clock();
		for(cclong i = r->childCount; i > 0; i--){
			t = readThread_(pDb, i);
			if(r->childCount - i > 500) break; // only search the most recent 500 threads/replies

			if(strcmp(username, t->ssid) == 0)
				sendThread(conn, t, true, false, true, true, false);

			if(t) delete t;
		}
		clock_t endc = clock();
		mg_printf_data(conn, "Completed in %.3lfs<br/>",(float)(endc - startc) / CLOCKS_PER_SEC);
		printFooter(conn);

		if(r) delete r;
	}else
		printMsg(conn, "Your cookie is invalid");
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
	//var3: the options
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
			string fpath = "images/" + sfnamep + ext;

			FILE *fp = fopen(fpath.c_str(), "wb");
			fwrite(data, 1, data_len, fp);
			fclose(fp);

			strcpy(var4, (sfnamep + ext).c_str());
			fprintf(log_file, "Image uploaded: [%s] at %s", var4, nowNow());
		}
	}

	//see if there is a SPECIAL string in the text field
	if (strcmp(var1, "") == 0) strcpy(var1, "Untitled");
	//user trying to sega a thread/reply
	if (strstr(var3, "sage")) sage = true;
	//user trying to delete a thread/reply
	if (strstr(var3, "delete")){
		cclong id = extractLastNumber(conn);
		struct Thread * t = readThread_(pDb, id); //what he replies to is which he wants to delete
		userDeleteThread(conn, id, verifyAdmin(conn));
		return;
	}
	if (strstr(var3, "list")){
		userListThread(conn);
		return;
	}
	if (strstr(var3, "url")){
		mg_printf_data(conn, html_header, site_title, site_title);
		mg_printf_data(conn, "Image uploaded: [http://%s:%d/images/%s].</body></html>", 
			conn->local_ip, conn->local_port, var4);
		fprintf(log_file, "Image uploaded but not as a thread\n");
		return;
	}
	//admin trying to update a thread/reply
	if (strstr(var3, "update") && verifyAdmin(conn)){
		cclong id = extractLastNumber(conn);
		struct Thread * t = readThread_(pDb, id); //what admin replies to is which he wants to update
		//note the server doesn't filter the special chars such as "<" and ">"
		//so it's possible for admin to use HTML here
		writeString(pDb, t->content, var2, true); 
		printMsg(conn, "Updated successfully");
		fprintf(log_file, "Admin edited thread no.%d at %s", t->threadID, nowNow());
		return;
	}
	//admin trying to post a slogan
	//note that the content of the slogan is represented in HTML format
	if (strstr(var3, "slogan") && verifyAdmin(conn)){
		struct Thread* t = readThread_(pDb, 0);
		
		writeString(pDb, "slogan", var2, false);
		strncpy(t->content, "slogan", 16);
		
		printMsg(conn, "You have updated the slogan");
		fprintf(log_file, "Slogan updated at %s", nowNow());
		
		writeThread(pDb, 0, t, true);
		return;
	}
	//admin trying to update the footer
	//note that the content of the footer is represented in HTML format
	if (strstr(var3, "footer") && verifyAdmin(conn)){	
		writeString(pDb, "footer", var2, true);
		
		printMsg(conn, "You have updated the footer");
		fprintf(log_file, "Footer updated at %s", nowNow());
		return;
	}
	//image or comment or both
	if (strcmp(var2, "") == 0 && !fileAttached) {
		printMsg(conn, "Please enter something");
		return;
	}
	//verify the cookie
	char ssid[128], username[10];
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
		if (tmp.size() != 2){
			if (stop_newcookie){
				printMsg(conn, "The board has stopped delivering new cookies");
				return;
			}
			else
				strcpy(username, giveNewCookie(conn));
		}else{
			strncpy(username, tmp[0].c_str(), 10);

			//auto iter = find(banlist.begin(), banlist.end(), username);
		
			if (banlist.find(tmp[0]) != banlist.end()){
				//this id is banned, so we destory it
				destoryCookie(conn);
				fprintf(log_file, "Cookie Destoryed: [%s] at %s", ssid, nowNow());
				printMsg(conn, "Your ID is banned");
				return;
			}

			char *testssid = generateSSID(username);
			if (strcmp(testssid, tmp[1].c_str()) != 0){
				giveNewCookie(conn);
				printMsg(conn, "Your cookie is broken");
				return;
			}
		}
		renewCookie(conn, username);
	}

	if (strcmp(var3, admin_pass) == 0) strcpy(username, "Admin");

	//replace some important things
	string tmpcontent(var2);	cleanString(tmpcontent);
	string tmpname(var1);		cleanString(tmpname);
	//string tmpemail(var3);		cleanString(tmpemail);

	strncpy(var1, tmpname.c_str(), 64);
	//strncpy(var3, tmpemail.c_str(), 64);

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
		cclong id = extractLastNumber(conn);
		struct Thread* t = readThread_(pDb, id);

		if(t->state & LOCKED_THREAD)
			printMsg(conn, "You can't reply to a locked thread");
		else{
			newReply(pDb, id, tmpcontent.c_str(), var1, conn->remote_ip, username, var4, sage);
			mg_printf_data(conn, html_header, site_title, site_title);
			mg_printf_data(conn, html_redirtothread, id, id, id);
			printFooter(conn);
		}

		if(t) delete t;
	}
	else{
		newThread(pDb, tmpcontent.c_str(), var1, conn->remote_ip, username, var4, sage);
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
		cclong j = extractLastNumber(conn) * threadsPerPage;
		cclong i = j - threadsPerPage + 1;
		showThreads(conn, i, j);
	}

	char ssid[10];
	mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, 9); ssid[9] = 0;
	time_t nn;
	time(&nn);
	mg_printf_data(conn, "<span class='admin'><a>%s</a><a>Last: %.3lfh</a>", ssid,
		difftime(nn, g_startuptime) / 3600);
	
	mg_printf_data(conn, "<a>Cookie: %s</a>", stop_newcookie ? "OFF" : "ON");
	mg_printf_data(conn, "<a>CD: %ds</a></span>", cd_time);
	printFooter(conn);
}

static void send_reply(struct mg_connection *conn) {
	//total_hit++;

	if (strcmp(conn->uri, "/post_thread") == 0) {
		postSomething(conn, conn->uri);
	}
	else if (strstr(conn->uri, "/page/")) {
		returnPage(conn, false);
	}
	else if (strstr(conn->uri, "/thread/")) {
		mg_send_header(conn, "charset", "utf-8");
		mg_printf_data(conn, html_header, site_title, site_title);

		cclong id = extractLastNumber(conn);
		cclong pid = findParent(pDb, id);

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
		printFooter(conn);
	}
	else if (strstr(conn->uri, "/post_reply/")) {
		postSomething(conn, conn->uri);
	}
	else if (strstr(conn->uri, "/spell/")) {
		if(!checkIP(conn, true)) return;

		if(strstr(conn->uri, admin_pass)){
			char ssid[128]; 
			mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, sizeof(ssid));

			if(strcmp(ssid, "") != 0){
				strncpy(adminCookie, ssid, 64);
				printMsg(conn, "Admin's cookie has been set to %s", adminCookie);
				fprintf(log_file, "New admin cookie %s at %s", adminCookie, nowNow());
			}
			else
				printMsg(conn, "Can't set admin's cookie to null");
		}else{
			printMsg(conn, "Your don't have the permission");
			checkIP(conn, true);
		}

	}
	else if (strstr(conn->uri, "/state/")){
		if(!checkIP(conn, true)) return;

		string url(conn->uri);
		vector<string> tmp = split(url, "/");

		cclong newstate = atol(tmp[tmp.size() - 1].c_str());
		cclong tid  = atol(tmp[tmp.size() - 2].c_str());

		if (verifyAdmin(conn)){
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
		cclong id = extractLastNumber(conn);
		struct Thread * t = readThread_(pDb, id); 

		if (verifyAdmin(conn))
			userDeleteThread(conn, id, true);
		else{
			printMsg(conn, "Your don't have the permission");
			checkIP(conn, true);
		}
		
	}
	else if (strstr(conn->uri, "/sage/")){
		if (verifyAdmin(conn))
			doThread(conn, extractLastNumber(conn), SAGE_THREAD);
		else{
			printMsg(conn, "Your don't have the permission");
			checkIP(conn, true);
		}
	}

	else if (strstr(conn->uri, "/lock/")){
		if (verifyAdmin(conn))
			doThread(conn, extractLastNumber(conn), LOCKED_THREAD);
		else{
			printMsg(conn, "Your don't have the permission");
			checkIP(conn, true);
		}
	}
	else if (strstr(conn->uri, "/ban/")){
		string url(conn->uri);
		vector<string> tmp = split(url, "/");
		string id = tmp[tmp.size() - 1];

		if (verifyAdmin(conn)){
			unordered_set<string>& xlist = strstr(conn->uri, "/ip/") ? ipbanlist : banlist;

			auto iter = xlist.find(id);
			if (iter == xlist.end()){
				xlist.insert(id);
				printMsg(conn, "You have banned ID/IP: %s.", id.c_str());
				fprintf(log_file, "ID/IP Banned: [%s] at %s", id.c_str(), nowNow());
			}
			else{
				xlist.erase(iter);
				printMsg(conn, "You have UNbanned ID/IP: %s.", id.c_str());
				fprintf(log_file, "ID/IP Unbanned: [%s] at %s", id.c_str(), nowNow());
			}

			std::ofstream f(ipbanlist_path);
			for(auto i = ipbanlist.begin(); i != ipbanlist.end(); ++i)
				f << *i << '\n';
			f.close();
		}
		else{
			printMsg(conn, "Your don't have the permission");
			checkIP(conn, true);
		}

	}
	else if (strstr(conn->uri, "/list/")){
		string url(conn->uri);
		vector<string> tmp = split(url, "/");
		string id = tmp[tmp.size() - 1];

		if (verifyAdmin(conn)){
			// the following actions could be extremely slow
			struct Thread *r = readThread_(pDb, 0); 
			struct Thread *t;

			mg_printf_data(conn, html_header, site_title, site_title);
			clock_t startc = clock();

			for(cclong i = r->childCount; i > 0; i--){
				t = readThread_(pDb, i);
				if(strstr(conn->uri, "/ip/")){
					if(strcmp(id.c_str(), t->email) == 0)
						sendThread(conn, t, true, false, true, true, true);
				}else{
					if(strcmp(id.c_str(), t->ssid) == 0)
						sendThread(conn, t, true, false, true, true, true);
				}

				if(t) delete t;
			}
			clock_t endc = clock();
			mg_printf_data(conn, "Completed in %.3lfs<br/>",(float)(endc - startc) / CLOCKS_PER_SEC);
			printFooter(conn);

			if(r) delete r;
		}
		else{
			printMsg(conn, "Your don't have the permission");
			checkIP(conn, true);
		}

	}
	else if (strstr(conn->uri, "/rename/")){
		string url(conn->uri);
		vector<string> tmp = split(url, "/");
		string oldname = "images/" + tmp[tmp.size() - 1];
		string newname = oldname + ".tmp";

		if (verifyAdmin(conn)){
			rename(oldname.c_str(), newname.c_str());
			printMsg(conn, "Rename image from %s to %s", oldname.c_str(), newname.c_str());
		}
		else{
			printMsg(conn, "Your don't have the permission");
			checkIP(conn, true);
		}
	}
	else if (strstr(conn->uri, "/images/")){
		const char *ims = mg_get_header(conn, "If-Modified-Since");
		//fprintf(log_file, "[%s]\n", ims);
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
		strcpy(ipath, ("images/" + tmp[tmp.size() - 1]).c_str());

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
				"Content-Length: %lu\r\n\r\n", expire, (unsigned cclong)st.st_size);
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
		if (verifyAdmin(conn)){
			stop_newcookie = strstr(conn->uri, "/close") ? true: false;
			printMsg(conn, "Your have closed/opened new cookie delivering.");
			fprintf(log_file, "Cookie closed/opened at %s", nowNow());
		}else{
			printMsg(conn, "Your don't have the permission.");
			checkIP(conn, true);
		}
	}
	else 
		returnPage(conn, true);

	fflush(log_file);
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
	char log_file_path[256];
	log_file = stdout;

	for (int i = 0; i < argc; ++i){
		if (strcmp(argv[i], "-database") == 0){
			zPath = new char[256];
			strcpy(zPath, argv[++i]);
			continue;
		}
		if (strcmp(argv[i], "-log") == 0){
			strcpy(log_file_path, argv[++i]);
			log_file = fopen(log_file_path, "a");
			continue;
		}
	}

	int rc = unqlite_open(&pDb, zPath, UNQLITE_OPEN_CREATE);
	if (rc != UNQLITE_OK) Fatal("Out of memory");

	struct tm * timeinfo;
	time_t cur = time(NULL);
	timeinfo = localtime(&(cur));
	char timetmp[64];
	strftime(timetmp, 64, "%Y-%m-%d %X", timeinfo);

	fprintf(log_file, "===============================");
	fprintf(log_file, "Start Time: \t\t[%s]\n", timetmp);
	fprintf(log_file, "Database: \t\t[%s]\n", zPath);
	fprintf(log_file, "Log File: \t\t[%s][%d]\n", log_file_path, log_file);
	//generate a random admin password
	admin_pass = new char[11];
	ipbanlist_path = new char[32];

	unqlite_util_random_string(pDb, admin_pass, 10); admin_pass[10] = 0;
	md5_salt = "coyove";
	strcpy(adminCookie, admin_pass);
	strcpy(ipbanlist_path, "ipbanlist");

	for (int i = 0; i < argc; ++i){
		if (strcmp(argv[i], "-newprofile") == 0) {
			fprintf(log_file, "Database: \t\t[New Profile]\n");
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

		if (strcmp(argv[i], "-admincookie") == 0){
			strcpy(adminCookie, argv[++i]);
			continue;
		}

		if (strcmp(argv[i], "-ipban") == 0){
			strcpy(ipbanlist_path, argv[++i]);
			continue;
		}

		// if (strcmp(argv[i], "-NOGPFAULTERRORBOX") == 0)
		// 	SetErrorMode(SEM_NOGPFAULTERRORBOX);
	}

	fprintf(log_file, "Site Title: \t\t[%s]\nSite Admin Password: \t[%s]\n", site_title, admin_pass);
	fprintf(log_file, "Threads Per Page: \t[%d]\n", threadsPerPage);
	fprintf(log_file, "Cooldown Time: \t\t[%ds]\n", cd_time);
	fprintf(log_file, "MD5 Salt: \t\t[%s]\n", md5_salt);
	fprintf(log_file, "Admin Cookie: \t\t[%s]\n", adminCookie);
	fprintf(log_file, "IP Ban List: \t\t[%s]\n", ipbanlist_path);

	std::ifstream f(ipbanlist_path);
	string line;
	while (f >> line) if(line != "") ipbanlist.insert(line);
	f.close();

	struct mg_server *server = mg_create_server(NULL, ev_handler);

	mg_set_option(server, "listening_port", lport);

	fprintf(log_file, "Listening on Port: \t[%s]\n", mg_get_option(server, "listening_port"));
	fflush(log_file);

	time(&g_startuptime);

	for (;;) {
		mg_poll_server(server, 1000);
	}

	mg_destroy_server(&server);
	unqlite_close(pDb);

	return 0;
}