//unsafe
#define _CRT_SECURE_NO_WARNINGS
// #define NS_ENABLE_SSL
// #define NS_ENABLE_DEBUG

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
time_t  gStartupTime;               //server's startup time
char*   adminPassword;              // * administrator's password
char    siteTitle[64];              //site's title
char    md5Salt[64]     = "coyove"; // * MD5 salt
char    adminCookie[64];            //Admin's cookie
int     threadsPerPage  = 5;        //threads per page to show
int     postCDTime      = 20;       //cooldown time
int     maxFileSize     = 2;        //megabytes
bool    stopNewcookie   = false;    //stop delivering new cookies
bool    stopCheckIP     = false;    //stop checking ip
bool    useXFF          = false;
FILE*   log_file;                   //log file

char                    banListPath[64];//file to store ip ban list
unordered_set<string>   IDBanList;      //cookie ban list
unordered_set<string>   IPBanList;      //ip ban list
map<string, cclong>     IPAccessList;   //remote ip list

#define TEST_ARG(b1, b2) (strcmp(argv[i], b1) == 0 || strcmp(argv[i], b2) == 0)

const char * getClientIP(mg_connection* conn){
    const char * xff = mg_get_header(conn, "X-Forwarded-For");
    return useXFF ? (xff ? xff : conn->remote_ip) : conn->remote_ip;
}

void printFooter(mg_connection* conn){
    char * footer = readString(pDb, "footer");
    if(footer){
        mg_printf_data(conn, "<div id='footer'>%s</div>", footer);
        delete [] footer;
    }
    mg_printf_data(conn, "</body></html>");
}

void printHeader(mg_connection* conn){
    char * header = readString(pDb, "header");
    if(header){
        mg_printf_data(conn, html_header, siteTitle, header);
        delete [] header;
    }else
        mg_printf_data(conn, html_header, siteTitle, siteTitle);
}

void printMsg(mg_connection* conn, const char* msg, ...){
    printHeader(conn);
    
    char tmpbuf[512]; //=w=
    va_list argptr;
    va_start(argptr, msg);
    vsprintf(tmpbuf, msg, argptr);
    va_end(argptr);

    mg_printf_data(conn, html_error, "/ ", tmpbuf, "/ ", STRING_HOMEPAGE);
    printFooter(conn);
}

bool checkIP(mg_connection* conn, bool verbose = false){
    if(stopCheckIP) return true;

    time_t rawtime;
    time(&rawtime);
    const char * cip = getClientIP(conn);
    string sip(cip);
    time_t lasttime = IPAccessList[sip];
    
    if(strstr(conn->uri, adminPassword)) return true;
    char ssid[128];
    mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, sizeof(ssid));
    if(strcmp(ssid, adminCookie) == 0) return true;

    if(IPBanList.find(sip) != IPBanList.end()) {
        printMsg(conn, STRING_YOUR_IP_IS_BANNED);
        logLog("Banned IP %s Access", cip);
        return false; //banned
    }

    if( abs(lasttime - rawtime) < postCDTime && lasttime != 0){
        printMsg(conn, STRING_COOLDOWN_TIME, cip);
        if(verbose)
            logLog("Rapid Access %s", cip);
        return false;
    }
    IPAccessList[sip] = rawtime;
    return true;
}

bool verifyAdmin(mg_connection* conn){
    char ssid[128]; 
    mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, sizeof(ssid));
    return (strcmp(ssid, adminCookie) == 0);
}

// Linux only
unsigned long readMemusage(){
    unsigned long dummy;
    unsigned long res;

    FILE *f = fopen("/proc/self/statm", "r");
    if(!f) return 0;
    fscanf(f,"%ld %ld %ld %ld %ld %ld %ld", &dummy, &res, &dummy, &dummy, &dummy, &dummy, &dummy);
    fclose(f);
    return res;
}

void doThread(mg_connection* conn, cclong tid, char STATE){
    struct Thread* t = readThread_(pDb, tid);

    if(findParent(pDb, tid) == 0){
        changeState(t, STATE, !(t->state & STATE));
    
        writeThread(pDb, tid, t, true);
        printMsg(conn, "Thread No.%d new state: %s", tid, resolveState(t->state));
        logLog("Thread No.%d New State: '%s'", tid, resolveState(t->state));
    }else
        printMsg(conn, "You can't sage or lock a reply");
}

#define SEND_IS_REPLY           1
#define SEND_SHOW_REPLY_LINK    2
#define SEND_CUT_LONG_COMMENT   4
#define SEND_CUT_IMAGE          8
#define SEND_CUT_REPLY_COUNT    16

void sendThread(mg_connection* conn, struct Thread* r, char display_state, bool admin_view = false, char* iid = ""){
    if (!(r->state & NORMAL_DISPLAY) && !admin_view) return;

    struct tm * timeinfo;
    char tmp[65536] = {'\0'};
    timeinfo = localtime(&(r->date));
    int len;

    bool reply 		= display_state & SEND_IS_REPLY;
    bool show_reply = display_state & SEND_SHOW_REPLY_LINK;
    bool cut_cclong = display_state & SEND_CUT_LONG_COMMENT;
    bool cut_image 	= display_state & SEND_CUT_IMAGE;
    bool cut_count 	= display_state & SEND_CUT_REPLY_COUNT;
    
    //the date and time
    char timetmp[64];
    strftime(timetmp, 64, "%Y-%m-%d %X", timeinfo);

    char crl[128] = {'\0'}; //, display_ssid[100] = {'\0'}; //, width1[64], width2[32];
    snprintf(crl, 127, show_reply ? "[<a href=\"/thread/%d\">"STRING_REPLY"</a>]" : "", r->threadID); 
    //a reply thread has an horizontal offset(20px) at left
    // strcpy(width1, reply ? "<div class='holder'>&gt;&gt;</div>" : "");
    // strcpy(width2, reply ? "class='thread header' " : "class='thread' ");
    //display the sage flag
    // strcpy(sage, (r->state & SAGE_THREAD && !reply) ? "<font color='red'><b>&#128078;&nbsp;"STRING_THREAD_SAGED"</b></font><br/>" : "");
    //display the lock flag
    // strcpy(locked, r->state & LOCKED_THREAD ? "<font color='red'><b>&#128274;&nbsp;"STRING_THREAD_LOCKED"</b></font><br/>" : "");
    //display the red name
    // strcpy(display_ssid, (strcmp(r->ssid, "Admin") == 0) ? "<font color='red'>"STRING_ADMIN"</font>" : r->ssid);
    // if(strcmp(r->ssid, iid) == 0) strcat(display_ssid, "<pox>"STRING_POSTER"</pox>");
    //display the deleted flag
    // strcpy(deleted, !(r->state & NORMAL_DISPLAY) ? "<font color='red'><b>&#10006;&nbsp;Deleted</b></font><br/>" : "");

    char reply_count[256] = {'\0'};
    if(r->childThread){
        struct Thread* c = readThread_(pDb, r->childThread);
        if(c->childCount > 5 && show_reply && !reply)
            snprintf(reply_count, 255,
                "<font color='darkcyan'>&#128172;&nbsp;<i>"SRTING_THREAD_REPLIES_HIDE"</i></font><br/>", 
                c->childCount - 5, crl); 
        else if(!cut_count){
            snprintf(reply_count, 255,
                "<font color='darkcyan'>&#128172;&nbsp;<i>"SRTING_THREAD_REPLIES"</i></font><br/>", 
                c->childCount); 
        }
        delete c;
    }

    char ref_or_link[64] = {'\0'};
    if(show_reply)
        snprintf(ref_or_link, 63, "<a href='javascript:qref(%d)'>No.%d</a>", r->threadID, r->threadID);
    else
        snprintf(ref_or_link, 63, "<a href='/thread/%d'>No.%d</a>", r->threadID, r->threadID);

    char display_image[256] = {'\0'};
    if (strlen(r->imgSrc) >= 4){
        if(cut_image){
            struct stat st;
            string filename(r->imgSrc);
            stat(("images/" + filename).c_str(), &st);

            snprintf(display_image, 255, "<div class='img'><a href='/images/%s'>["STRING_VIEW_IMAGE" (%d kb)]</a></div>", 
                r->imgSrc, st.st_size / 1024);

        }
        else{
            // if(reply)
            snprintf(display_image, 255, 
                	"<div class='img'>"
                		"<a id='img-%d' href='javascript:void(0)' onclick='enim(\"img-%d\")'>"
                			"<img class='%s' src='/images/%s'/>"
                		"</a>"
                	"</div>", r->threadID, r->threadID, reply ? "img-s" : "img-n", r->imgSrc);
            // else
            //     snprintf(display_image, 255, 
            //     	"<div class='img'><a href='/images/%s'><img class='imgn' src='/images/%s'/></a></div>", r->imgSrc, r->imgSrc);
        }
    }

    char admin_ctrl[128] = {'\0'};
    if(admin_view)
        snprintf(admin_ctrl, 127, "&nbsp;<span id='admin-%d'><a class='wp-btn' onclick='javascript:admc(%d)'>%s</a></span>", r->threadID, r->threadID, r->email);

    //if the content is too cclong to display
    char c_content[1050] = {'\0'};
    char* content = readString(pDb, r->content);
    strncpy(c_content, content, 1000);
    c_content[1000] = 0;

    if (strlen(content) > 1000) strcat(c_content, "<font color='red'><b>[more]</b></font>");

    len = snprintf(tmp, 65535,
        "<div>%s"
        	"<div %s>"
        		/*image*/
        		"%s"
        		/*thread header*/
        		"%s&nbsp;<font color='#228811'>%s</font>&nbsp;"
        		"<span class='tms hiding'>%d</span><span class='tmsc'>%s</span>&nbsp;"
        		"ID:<ssid>%s%s</ssid> %s %s"
        		/*thread comment*/
        		"<div class='quote'>%s</div>"
		        "%s"
		        "%s"
		        "%s"
		        "%s"
        	"</div>"
        "</div>",
        /*place holder*/
        reply 								? "<div class='holder'>&gt;&gt;</div>" : "", 
        reply 								? "class='thread header'" : "class='thread'",
        /*image*/
        display_image, 
        ref_or_link, r->author, 
        r->date, timetmp, 
        (strcmp(r->ssid, "Admin") == 0) 	? "<font color='red'>"STRING_ADMIN"</font>" : r->ssid, 
        (strcmp(r->ssid, iid) == 0)			? "<pox>"STRING_POSTER"</pox>" : "", 
        crl, admin_ctrl,
        /*do we cut long comment?*/
        cut_cclong 							? c_content : content, 
        /*thread state*/
        (r->state & SAGE_THREAD && !reply) 	? "<font color='red'><b>&#128078;&nbsp;"STRING_THREAD_SAGED"</b></font><br/>" : "", 
        (r->state & LOCKED_THREAD) 			? "<font color='red'><b>&#128274;&nbsp;"STRING_THREAD_LOCKED"</b></font><br/>" : "", 
        !(r->state & NORMAL_DISPLAY) 		? "<font color='red'><b>&#10006;&nbsp;"STRING_THREAD_DELETED"</b></font><br/>" : "",
        /*reply count*/
        reply_count);

    mg_send_data(conn, tmp, len);

    if(content) delete [] content;
}

void requestAdminConsole(mg_connection* conn, struct Thread* r){
    char admin_ctrl[1024];
    int len = sprintf(admin_ctrl, "<br><span>"
                        "<a class='wp-btn' href='/sage/%d' title='Sage Thread'>&#128078;</a>"
                        "<a class='wp-btn' href='/lock/%d' title='Lock Thread'>&#128274;</a>"
                        "<a class='wp-btn' href='/delete/%d' title='Del Thread'>&#128640;</a>"
                        "<a class='wp-btn' href='/rename/%s'>"STRING_ADMIN_DEL_IMG"</a>"
                        "<a class='wp-btn' href='/list/ip/%s'>"STRING_ADMIN_LIST_IP"</a>"
                        "<a class='wp-btn' href='/ban/ip/%s'>"STRING_ADMIN_BAN_IP"</a>"
                        "<a class='wp-btn' href='http://ip.chinaz.com/?IP=%s'>"STRING_ADMIN_WHERE_IP"</a>"
                        "<a class='wp-btn' href='/list/%s'>"STRING_ADMIN_LIST_ID"</a>"
                        "<a class='wp-btn' href='/ban/%s'>"STRING_ADMIN_BAN_ID"</a>"
                        "<a class='wp-btn' href='/state/%d/%d'>"STRING_ADMIN_RESTORE"</a>"
                        "</span>",
                        r->threadID, r->threadID, r->threadID, r->imgSrc, 
                        r->email, r->email, r->email,
                        r->ssid, r->ssid,
                        r->threadID, r->state & MAIN_THREAD ? 5 : 3);

    mg_send_data(conn, admin_ctrl, len);
}

void showGallery(mg_connection* conn, cclong startID, cclong endID){
    struct Thread *r = readThread_(pDb, 0); // get the root thread
    cclong c = 0;
    clock_t startc = clock();

    bool admin_view = verifyAdmin(conn);

    cclong totalThreads = r->childCount;
    cclong totalPages = 100;

    char *slogan = readString(pDb, "slogan");
    if(slogan) {
        mg_printf_data(conn, "<div id='slogan'>%s</div>", slogan);
        if(admin_view)
            mg_printf_data(conn, "<script type=\"text/javascript\">"
                "var elem = document.getElementById('slogan');"
                "elem.innerText = elem.innerHTML;"
                "</script>");
        delete [] slogan;
    }

    for(cclong i = totalThreads; i > 0; --i){
        delete r;
        r = readThread_(pDb, i);
        if(strlen(r->imgSrc) == 15 && r->state & NORMAL_DISPLAY) {
            c++;
            if (c >= startID && c <= endID){
                mg_printf_data(conn, "<hr>");
                //sendThread(conn, r, false, true, true, false, admin_view);
                sendThread(conn, r, SEND_SHOW_REPLY_LINK + SEND_CUT_LONG_COMMENT, admin_view);
            }
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
                len = sprintf(tmp, "<a class='pager-inv'>%d</a>", i);
            else
                len = sprintf(tmp, "<a class='pager' href=\"/gallery/%d\">%d</a>", i, i);

            mg_send_data(conn, tmp, len);
        }
    }
    mg_printf_data(conn, "<a class='pager' href=\"/gallery/%d\">"STRING_NEXT_PAGE"</a>", ++current_page);
    mg_printf_data(conn, "|<a class='pager' href='/'>&#128193;&nbsp;"STRING_TIMELINE_PAGE"</a>");
    mg_printf_data(conn, "<br/><br/>");
    
    clock_t endc = clock();
    mg_printf_data(conn, "Completed in %.3lfs<br/>",(float)(endc - startc) / CLOCKS_PER_SEC);
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

    char *slogan = readString(pDb, "slogan");
    if(slogan) {
        mg_printf_data(conn, "<div id='slogan'>%s</div>", slogan);
        if(admin_view)
            mg_printf_data(conn, "<script type=\"text/javascript\">"
                "var elem = document.getElementById('slogan');"
                "elem.innerText = elem.innerHTML;"
                "</script>");
        delete [] slogan;
    }

    while (r->nextThread){
        cclong tmpid = r->nextThread;
        delete r;
        r = readThread_(pDb, tmpid);
        c++;
        if (c >= startID && c <= endID){
            mg_printf_data(conn, "<hr>");
            //sendThread(conn, r, false, true, true, false, admin_view);
            sendThread(conn, r, SEND_SHOW_REPLY_LINK + SEND_CUT_LONG_COMMENT + SEND_CUT_REPLY_COUNT, admin_view);
            char *iid = new char[10];
            strcpy(iid, r->ssid);

            struct Thread *oldr = r;

            if (r->childThread) {
                vector<struct Thread *> vt;

                r = readThread_(pDb, r->childThread); // beginning of the circle
                //vt.push_back(r);
                //mg_printf_data(conn, "<i>%d reply(s) total</i>", r->childCount);

                r = readThread_(pDb, r->prevThread);
                cclong rid = r->threadID; //the ID
                //sendThread(conn, r, true, false);
                if(r->state & NORMAL_DISPLAY) vt.push_back(r);
                
                int counter = 5;

                while (r->prevThread != rid){
                    //if(--counter == 0) break;
                    r = readThread_(pDb, r->prevThread);
                    if(r->state & NORMAL_DISPLAY){
                        if(--counter == 0) break;
                        vt.push_back(r);    
                    }
                }

                for (int i = vt.size() - 1; i >= 0; i--){
                    //sendThread(conn, vt[i], true, false, false, false, admin_view, iid);
                    sendThread(conn, vt[i], SEND_IS_REPLY + SEND_CUT_LONG_COMMENT, admin_view, iid);
                    delete vt[i];
                }

            }

            r = oldr;
            delete [] iid;
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
                len = sprintf(tmp, "<a class='pager-inv'>%d</a>", i);
            else
                if (i == 1) 
                    len = sprintf(tmp, "<a class='pager' href=\"/\">1</a>");
                else
                    len = sprintf(tmp, "<a class='pager' href=\"/page/%d\">%d</a>", i, i);

            mg_send_data(conn, tmp, len);
        }
    }
    mg_printf_data(conn, "<a class='pager' href=\"/page/%d\">"STRING_NEXT_PAGE"</a>", ++current_page);
    mg_printf_data(conn, "|<a class='pager' href='/gallery/1'>&#128193;&nbsp;"STRING_GALLERY_PAGE"</a>");
    mg_printf_data(conn, "<br/><br/>");
    
    clock_t endc = clock();
    mg_printf_data(conn, "Completed in %.3lfs<br/>",(float)(endc - startc) / CLOCKS_PER_SEC);
}

void showThread(mg_connection* conn, cclong id){
    clock_t startc = clock();

    if (id <= 0) {
        printMsg(conn, STRING_INVALID_ID);
        return;
    }
    struct Thread *r = readThread_(pDb, id); // get the root thread
    cclong c = 0;

    bool admin_view = verifyAdmin(conn);

    //sendThread(conn, r, false, false, false, false, admin_view);
    sendThread(conn, r, 0, admin_view);
    mg_printf_data(conn, "<hr>");

    char iid[10];
    strcpy(iid, r->ssid);

    char zztmp[512];
    int len = sprintf(zztmp, "/post_reply/%d", id); zztmp[len] = 0;

    mg_printf_data(conn, html_form, zztmp, "postform", STRING_REPLY_TO_THREAD);
        
    if (r->childThread) {
        cclong r_childThread = r->childThread;
        delete r;

        r = readThread_(pDb, r_childThread); // beginning of the circle
        cclong rid = r->threadID; //the ID
        bool too_many_replies = (r->childCount > 20);

        //sendThread(conn, r , true, true, false, false, admin_view, iid);
        sendThread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK, admin_view, iid);

        int di = 1;
        while (r->nextThread != rid){
            cclong r_nextThread = r->nextThread;
            delete r;

            r = readThread_(pDb, r_nextThread);
            di++;

            //sendThread(conn, r, true, true, false, too_many_replies && (di > 20), admin_view, iid);
            if(too_many_replies && (di > 20))
                sendThread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK + SEND_CUT_IMAGE, admin_view, iid);
            else
                sendThread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK, admin_view, iid);
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
    char *username = verifyCookie(conn);

    if (username){
        struct Thread* t = readThread_(pDb, tid);

        if (strcmp(t->ssid, username) == 0 || admin){
            deleteThread(pDb, tid);
            printMsg(conn, STRING_DELETE_SUCCESS, tid);
            logLog("Thread No.%d Deleted", tid);
        }
        else{
            printMsg(conn, STRING_NO_PERMISSION);
        }

        if(t) delete t;
    }else
        printMsg(conn, STRING_NO_PERMISSION);

    delete [] username;
}

void userListThread(mg_connection* conn, bool admin_view = false){
    char *username = verifyCookie(conn);

    printHeader(conn);
    mg_printf_data(conn, "<button class='wp-btn' onclick='cvtm(true);'>&#128198;&nbsp;"STRING_TIMESTAMP_DISPLAY"</button><br>"
            "<small>Example:<span class='tms hiding'>0</span><span class='tmsc'></span></small><hr>");

    if (username){
        struct Thread *r = readThread_(pDb, 0); 
        struct Thread *t;

        clock_t startc = clock();
        for(cclong i = r->childCount; i > 0; i--){
            t = readThread_(pDb, i);
            if(r->childCount - i > 500 && !admin_view) break; // only search the most recent 500 threads/replies

            if(strcmp(username, t->ssid) == 0 || admin_view)
                //sendThread(conn, t, true, false, true, true, false);
                sendThread(conn, t, SEND_IS_REPLY + SEND_CUT_LONG_COMMENT + SEND_CUT_IMAGE, false);

            if(t) delete t;
        }
        clock_t endc = clock();
        mg_printf_data(conn, "Completed in %.3lfs<br/>",(float)(endc - startc) / CLOCKS_PER_SEC);
        printFooter(conn);

        if(r) delete r;
    }else{
        printFooter(conn);
    }

    delete [] username;
}

void postSomething(mg_connection* conn, const char* uri){
    char var1[64] = {'\0'}, var2[32768] = {'\0'}, var3[64] = {'\0'}, var4[16] = {'\0'};
    bool sage = false;
    bool fileAttached = false;
    bool user_delete = false;
    const char *data;
    int data_len, ofs = 0, mofs = 0;
    char var_name[100], file_name[100];

    //first check the ip
    if(!checkIP(conn)) return;
    bool admin_ctrl = verifyAdmin(conn);
    //get the post form data
    char dump_name[12];
    unqlite_util_random_string(pDb, dump_name, 11);
    dump_name[11] = 0;

    FILE *dump = fopen( ("dump/" + string(dump_name)).c_str() , "wb");
    fwrite(conn->content, 1, conn->content_len, dump);
    fclose(dump);

    if(conn->content_len <= 0){
        printMsg(conn, STRING_UNKNOWN_ERROR);
        return;
    }

    while ((mofs = mg_parse_multipart(conn->content + ofs, conn->content_len - ofs,
        var_name, sizeof(var_name),
        file_name, sizeof(file_name),
        &data, &data_len)) > 0) {
        
        ofs += mofs;

        if (strcmp(var_name, "input_name") == 0) {  //var1: the subject of the thread
            data_len = data_len > 63 ? 63 : data_len;
            strncpy(var1, data, data_len);
            var1[data_len] = 0;
        }

        if (strcmp(var_name, "input_email") == 0) { //var3: the options
            data_len = data_len > 63 ? 63 : data_len;
            strncpy(var3, data, data_len);
            var3[data_len] = 0;
        }

        if (strcmp(var_name, "input_content") == 0) { //var2: the comment
            data_len = data_len > 32767 ? 32767 : data_len;
            strncpy(var2, data, data_len);
            var2[data_len] = 0;
        }

        strcpy(var4, "");

        if (strcmp(var_name, "input_file") == 0 && strcmp(file_name, "") != 0) {    //var4: the image attached (if has)
            if (data_len > 1024 * 1024 * maxFileSize){
                printMsg(conn, STRING_FILE_TOO_BIG);
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
            else if (endsWith(sfname, ".torrent"))
                ext = ".xxx";
            else{
                printMsg(conn, STRING_INVALID_FILE);
                return;
            }

            fileAttached = true;
            string fpath = "images/" + sfnamep + ext;

            FILE *fp = fopen(fpath.c_str(), "wb");
            fwrite(data, 1, data_len, fp);
            fclose(fp);

            strcpy(var4, (sfnamep + ext).c_str());
            logLog("Image Uploaded: '%s'", var4);
        }
    }

    //logLog("New Thread Before: '%s', '%s', '%s', '%s'", var1, var2, var3, var4);

    //see if there is a SPECIAL string in the text field
    if (strcmp(var1, "") == 0) strcpy(var1, STRING_UNTITLED);
    //user trying to sega a thread/reply
    if (strstr(var3, "sage")) sage = true;
    //user trying to delete a thread/reply
    if (strstr(var3, "delete")){
        cclong id = extractLastNumber(conn);
        struct Thread * t = readThread_(pDb, id); //what he replies to is which he wants to delete
        userDeleteThread(conn, id, admin_ctrl);
        return;
    }
    if (strstr(var3, "list")){
        userListThread(conn);
        return;
    }
    if (strstr(var3, "url")){
        printHeader(conn);
        mg_printf_data(conn, "Image uploaded: <div style='background-color:white; padding: 1em;border: dashed 1px'>"
            "http://%s:%d/images/%s</div></body></html>", 
            conn->local_ip, conn->local_port, var4);
        logLog("Image Uploaded But Not As New Thread");
        return;
    }
    //admin trying to update a thread/reply
    if (strstr(var3, "update") && admin_ctrl){
        cclong id = extractLastNumber(conn);
        struct Thread * t = readThread_(pDb, id); //what admin replies to is which he wants to update
        //note the server doesn't filter the special chars such as "<" and ">"
        //so it's possible for admin to use HTML here
        string up_str = replaceAll(string(var2), string("\n"), string("<br>"));
        writeString(pDb, t->content, up_str.c_str(), true); 
        printMsg(conn, "Thread No.%d updated successfully", t->threadID);
        logLog("Admin Edited Thread No.%d", t->threadID);
        return;
    }
    if (strstr(var3, "mod-") && admin_ctrl){    
        string ent = split(string(var3), "-")[1];
        writeString(pDb, (char*)ent.c_str(), var2, true);
        
        printMsg(conn, "You have updated the %s", ent.c_str());
        logLog("Var '%s' Updated", ent.c_str());
        return;
    }
    
    char ipath[64]; FILE *fp; struct stat st;
    sprintf(ipath, "images/%s", var3);
    
    if (stat(ipath, &st) == 0 && (fp = fopen(ipath, "rb")) != NULL && strcmp(var3, "") != 0)
        strncpy(var4, var3, 16);
    else{
        //image or comment or both
        if (strcmp(var2, "") == 0 && !fileAttached) {
            printMsg(conn, STRING_EMPTY_COMMENT);
            return;
        }else if (strcmp(var2, "") == 0 && fileAttached){
            strcpy(var2, STRING_UNCOMMENTED);
        }
    }
    //logLog("addr: %d %d %d %d %d", var1, var2, var3, var4, ipath);
    //verify the cookie
    char ssid[100];//, username[10];
    char *username = verifyCookie(conn);
    // if (strstr(var3, "|") && strlen(var3) == 42)
    //  strcpy(ssid, var3);
    // else
    mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, sizeof(ssid));

    if (username){  //passed
        string findID(username);
        if(strcmp(ssid, adminCookie) != 0)
            if (IDBanList.find(findID) != IDBanList.end()){
                //this id is banned, so we destory it
                destoryCookie(conn);
                logLog("Cookie Destoryed: '%s'", ssid);
                printMsg(conn, STRING_YOUR_ID_IS_BANNED);
                return;
            }
    }else{
        if (stopNewcookie){
            printMsg(conn, STRING_STOP_COOKIE);
            return;
        }
        else{
            username = new char[10];
            strcpy(username, giveNewCookie(conn));
            strcpy(ssid, renewCookie(username));
        }
    }
    // if (strcmp(ssid, "") == 0){
    //  //user doesn't have a cookie, we give one
    //  if (stopNewcookie){
    //      printMsg(conn, STRING_STOP_COOKIE);
    //      return;
    //  }
    //  else{
    //      strcpy(username, giveNewCookie(conn));
    //      strcpy(ssid, renewCookie(username));
    //  }
    // }
    // else{
    //  vector<string> tmp = split(string(ssid), string("|"));
    //  if (tmp.size() != 2){
    //      printMsg(conn, STRING_INVALID_COOKIE);
    //      return;
    //  }else{
    //      strncpy(username, tmp[0].c_str(), 10);
        
    //      if (banlist.find(tmp[0]) != banlist.end()){
    //          //this id is banned, so we destory it
    //          destoryCookie(conn);
    //          logLog("Cookie Destoryed: '%s'", ssid);
    //          printMsg(conn, STRING_YOUR_ID_IS_BANNED);
    //          return;
    //      }

    //      char *testssid = generateSSID(username);
    //      if (strcmp(testssid, tmp[1].c_str()) != 0){
    //          printMsg(conn, STRING_INVALID_COOKIE);
    //          delete [] testssid;
    //          return;
    //      }
    //      delete [] testssid;
    //  }
    // }

    if (strcmp(var3, adminPassword) == 0) strcpy(username, "Admin");
    if (admin_ctrl) strcpy(username, "Admin");

    //replace some important things
    string tmpcontent(var2);    cleanString(tmpcontent);
    string tmpname(var1);       cleanString(tmpname);
    //string tmpemail(var3);        cleanString(tmpemail);

    strncpy(var1, tmpname.c_str(), 64);

    vector<string> imageDetector = split(tmpcontent, "\n");
    tmpcontent = "";

    for (auto i = 0; i < imageDetector.size(); ++i){
        if (startsWith(imageDetector[i], "http"))
            imageDetector[i] = "<a href='" + imageDetector[i] + "'>" + imageDetector[i] + "</a>";
        bool refFlag = false;
        if (startsWith(imageDetector[i], "&gt;&gt;No.")){
            vector<string> gotoLink = split(imageDetector[i], string("."));
            if (gotoLink.size() == 2){
                imageDetector[i] = "<div class='div-thread-" + gotoLink[1] + "'><a href='javascript:ajst(" + gotoLink[1] + ")'>" + imageDetector[i] + "</a></div>";
                refFlag = true;
            }
        }
        if (startsWith(imageDetector[i], "&gt;"))
                    imageDetector[i] = "<font color='#789922'>" + imageDetector[i] + "</font>";
        if(refFlag)
            tmpcontent += imageDetector[i];
        else
            tmpcontent += (imageDetector[i] + "<br/>");
    }

    const char * cip = getClientIP(conn);
    logLog("New Thread: (Sub: '%s', Opt: '%s', Img: '%s', Dump: '%s', IP: '%s')", var1, var3, var4, dump_name, cip);

    if (strstr(conn->uri, "/post_reply/")) {
        cclong id = extractLastNumber(conn);
        struct Thread* t = readThread_(pDb, id);

        if(t->state & LOCKED_THREAD)
            printMsg(conn, STRING_NO_REPLY_TO_LOCKED);
        else{
            newReply(pDb, id, tmpcontent.c_str(), var1, cip, username, var4, sage);
            
            mg_printf(conn, "HTTP/1.1 302 Moved Temporarily\r\nLocation: /success/%s/%d\r\n\r\n", ssid, id);
        }

        if(t) delete t;
    }
    else{
        newThread(pDb, tmpcontent.c_str(), var1, cip, username, var4, sage);
        //printMsg(conn, "Successfully start a new thread");
        mg_printf(conn, "HTTP/1.1 302 Moved Temporarily\r\nLocation: /success/%s/0\r\n\r\n", ssid);
    }
}

void returnPage(mg_connection* conn, bool indexPage, bool galleryPage = false){
    mg_send_header(conn, "charset", "utf-8");
    printHeader(conn);
    mg_printf_data(conn, show_hide_button);
    mg_printf_data(conn, html_form, "/post_thread", "hiding", STRING_NEW_THREAD);

    if(indexPage)
        showThreads(conn, 1, threadsPerPage);
    else{
        cclong j = extractLastNumber(conn) * threadsPerPage;
        cclong i = j - threadsPerPage + 1;
        if(galleryPage)
            showGallery(conn, i, j);
        else
            showThreads(conn, i, j);
    }

    char ssid[10];
    mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, 9); ssid[9] = 0;
    time_t nn;
    time(&nn);
    mg_printf_data(conn, "<small>%d.%.3lf.%ld.%s</small>", 
        stopNewcookie? 0 : 1, difftime(nn, gStartupTime) / 3600, readMemusage() * 4, ssid);
    printFooter(conn);
}

static void sendReply(struct mg_connection *conn) {

    if (strcmp(conn->uri, "/post_thread") == 0)
        postSomething(conn, conn->uri);         //post a thread
    else if (strstr(conn->uri, "/post_reply/"))
        postSomething(conn, conn->uri); //post a reply
    else if (strstr(conn->uri, "/page/"))
        returnPage(conn, false);        //view page
    else if (strstr(conn->uri, "/gallery/"))
        returnPage(conn, false, true);  //view gallery
    else if (strstr(conn->uri, "/success/")){
        string url(conn->uri);
        vector<string> tmp = split(url, "/");

        if(tmp.size() != 3){
            printMsg(conn, "Error");
            return;
        }
        cclong id  = atol(tmp[tmp.size() - 1].c_str());
        //renewCookie(conn, username);
        setCookie(conn, tmp[tmp.size() - 2].c_str());
        mg_send_header(conn, "charset", "utf-8");
        
        if(id == 0)
            printMsg(conn, STRING_NEW_THREAD_SUCCESS);
        else{
            printHeader(conn);
            mg_printf_data(conn, html_redirtothread, id, id, id);
            printFooter(conn);
        }   //post successfully page
    }
    else if (strstr(conn->uri, "/thread/")){
        cclong id = extractLastNumber(conn);
        cclong pid = findParent(pDb, id);

        if (pid == -1 && !verifyAdmin(conn)){
            printMsg(conn, STRING_THREAD_DELETED);
            return ;
        }
        else{
            mg_send_header(conn, "charset", "utf-8");
            printHeader(conn);

            if (pid)
                mg_printf_data(conn, "<a href='/thread/%d'>&lt;&lt; No.%d</a><hr>", pid, pid);
            else
                mg_printf_data(conn, "<a href='/'>&lt;&lt; "STRING_HOMEPAGE"</a><hr>");

            showThread(conn, id);

            printFooter(conn);
        }   //view a thread and its replies
    }
    else if (strstr(conn->uri, "/api/")){
        cclong id = extractLastNumber(conn);
        struct Thread *r = readThread_(pDb, id); // get the root thread
        bool admin_view = verifyAdmin(conn);

        if (!(r->state & NORMAL_DISPLAY)){
            //printMsg(conn, "This thread has been deleted");
            mg_printf_data(conn, "<Invalid Thread>");
            return;
        }
        else
            //sendThread(conn, r, false, false, false, false, admin_view);
            sendThread(conn, r, 0, admin_view);

        delete r;       //return a thread's plain HTML
    }
    else if (strstr(conn->uri, "/ascend/")){
        if(!checkIP(conn, true)) return;

        if(strstr(conn->uri, adminPassword)){
            char ssid[128]; 
            mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, sizeof(ssid));

            if(strcmp(ssid, "") != 0){
                strncpy(adminCookie, ssid, 64);
                printMsg(conn, "Admin's cookie has been set to %s", adminCookie);
                logLog("Set New Admin Cookie '%s' by '%s'", adminCookie, getClientIP(conn));
            }
            else
                printMsg(conn, "Can't set admin's cookie to null");
        }else{
            printMsg(conn, STRING_NO_PERMISSION);
            checkIP(conn, true);
        }   //ascend the admin privilege using new cookie
    }
    else if (strstr(conn->uri, "/acquire/")){
        if(!checkIP(conn, true)) return;

        if(strstr(conn->uri, adminPassword)){
            setCookie(conn, adminCookie);
            printMsg(conn, "Acquire admin's cookie: %s", adminCookie);
            logLog("Acquire Admin Cookie '%s' by '%s'", adminCookie, getClientIP(conn));
        }else{
            printMsg(conn, STRING_NO_PERMISSION);
            checkIP(conn, true);
        }   //transfer the admin privilege to a new browser
    }
    else if (strstr(conn->uri, "/abdicate/")){
        if(!checkIP(conn, true)) return;

        if(strstr(conn->uri, adminPassword)){
            strcpy(adminCookie, adminPassword);
            printMsg(conn, "Admin has abdicated");
            logLog("Admin Has Abdicated");
        }else{
            printMsg(conn, STRING_NO_PERMISSION);
            checkIP(conn, true);
        }   //drop the admin privilege
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
            printMsg(conn, STRING_NO_PERMISSION);
            checkIP(conn, true);
        }       //update a thread's state
    }
    else if (strstr(conn->uri, "/delete/")){
        cclong id = extractLastNumber(conn);
        struct Thread * t = readThread_(pDb, id); 

        if (verifyAdmin(conn))
            userDeleteThread(conn, id, true);
        else{
            printMsg(conn, STRING_NO_PERMISSION);
            checkIP(conn, true);
        }       //delete a thread
    }
    else if (strstr(conn->uri, "/sage/")){
        if (verifyAdmin(conn))
            doThread(conn, extractLastNumber(conn), SAGE_THREAD);
        else{
            printMsg(conn, STRING_NO_PERMISSION);
            checkIP(conn, true);
        }       //sage a thread
    }
    else if (strstr(conn->uri, "/lock/")){
        if (verifyAdmin(conn))
            doThread(conn, extractLastNumber(conn), LOCKED_THREAD);
        else{
            printMsg(conn, STRING_NO_PERMISSION);
            checkIP(conn, true);
        }       //lock a thread
    }
    else if (strstr(conn->uri, "/ban/")){
        string url(conn->uri);
        vector<string> tmp = split(url, "/");
        string id = tmp[tmp.size() - 1];

        if (verifyAdmin(conn)){
            unordered_set<string>& xlist = strstr(conn->uri, "/ip/") ? IPBanList : IDBanList;

            auto iter = xlist.find(id);
            if (iter == xlist.end()){
                xlist.insert(id);
                printMsg(conn, "You have banned ID/IP: %s", id.c_str());
                logLog("ID/IP Banned: '%s'", id.c_str());
            }
            else{
                xlist.erase(iter);
                printMsg(conn, "You have UNbanned ID/IP: %s", id.c_str());
                logLog("ID/IP Unbanned: '%s'", id.c_str());
            }

            std::ofstream f(banListPath);
            for(auto i = IPBanList.begin(); i != IPBanList.end(); ++i)
                f << *i << '\n';
            for(auto j = IDBanList.begin(); j != IDBanList.end(); ++j)
                f << "ID:" << *j << '\n';
            f.close();
        }
        else{
            printMsg(conn, STRING_NO_PERMISSION);
            checkIP(conn, true);
        }       //ban ID/IP
    }
    else if (strstr(conn->uri, "/list")){
        if(!checkIP(conn, true)) return;

        string url(conn->uri);
        vector<string> tmp = split(url, "/");
        string id = tmp[tmp.size() - 1];

        if (verifyAdmin(conn)){
            // the following actions could be extremely slow
            struct Thread *r = readThread_(pDb, 0); 
            struct Thread *t;
            bool ipflag = strstr(conn->uri, "/ip/"), killflag = strstr(conn->uri, "/kill/");

            if (strcmp(conn->uri, "/list") == 0){
                userListThread(conn, true);
                return;
            }

            printHeader(conn);
            clock_t startc = clock();

            for(cclong i = r->childCount; i > 0; i--){
                t = readThread_(pDb, i);
                if(ipflag){
                    if(strcmp(id.c_str(), t->email) == 0){
                        if(killflag)
                            deleteThread(pDb, i);
                        else
                            //sendThread(conn, t, true, false, true, true, true);
                            sendThread(conn, t, SEND_IS_REPLY + SEND_CUT_LONG_COMMENT + SEND_CUT_IMAGE, true);
                    }
                }else{
                    if(strcmp(id.c_str(), t->ssid) == 0){
                        if(killflag)
                            deleteThread(pDb, i);
                        else
                            sendThread(conn, t, SEND_IS_REPLY + SEND_CUT_LONG_COMMENT + SEND_CUT_IMAGE, true);
                    }
                }

                if(t) delete t;
            }
            clock_t endc = clock();
            mg_printf_data(conn, "Completed in %.3lfs<br/>",(float)(endc - startc) / CLOCKS_PER_SEC);
            printFooter(conn);

            if(r) delete r;
        }
        else{
            //printMsg(conn, "Your don't have the permission");
            userListThread(conn);
            //checkIP(conn, true);
        }       //list ID/IP, kill all posted by ID/IP
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
            printMsg(conn, STRING_NO_PERMISSION);
            checkIP(conn, true);
        }   //rename an image
    }
    else if (strstr(conn->uri, "/images/")){
        const char *ims = mg_get_header(conn, "If-Modified-Since");
        const char *inm = mg_get_header(conn, "If-None-Match");
        //logLog("[%s]\n", ims);
        if (ims)
            if (strcmp(ims, "") != 0){
                mg_printf(conn, "HTTP/1.1 304 Not Modified\r\n\r\n");
                return;
            }
        if (inm)
            if (strcmp(inm, "") != 0){
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
                "Expires: Tue, 31 Jul 2035 00:00:00 GMT\r\n"
                // "Cache-Control: must-revalidate\r\n"
                // "Cache-Control: Public\r\n"
                "Cache-Control: max-age=2592000\r\n"
                "ETag: \"placeholder\"\r\n"
                "Content-Length: %lu\r\n\r\n", (unsigned cclong)st.st_size);
            while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
                mg_write(conn, buf, n);
            }
            fclose(fp);
            mg_write(conn, "\r\n", 2);
        }else{
            mg_printf(conn, "HTTP/1.1 404 Not Found\r\nContent-Type: image/jpeg\r\nContent-Length: 0\r\n\r\n");
        }   //image
    }
    else if (strstr(conn->uri, "/cookie/")){
        if (strstr(conn->uri, "/destory")){
            destoryCookie(conn);
            printMsg(conn, STRING_RESET_COOKIE);
        }else{
            if (verifyAdmin(conn)){
                stopNewcookie = strstr(conn->uri, "/close") ? true: false;
                printMsg(conn, "Your have closed/opened new cookie delivering");
                logLog("Cookie Closed/Opened");
            }else{
                printMsg(conn, STRING_NO_PERMISSION);
                checkIP(conn, true);
            }
        }       //open/close/destory cookie
    }
    else if (strstr(conn->uri, "/adminconsole/")){
        if (verifyAdmin(conn)){
            cclong id = extractLastNumber(conn);
            struct Thread * t = readThread_(pDb, id); 
            requestAdminConsole(conn, t);
            delete t;
        }else{
            printMsg(conn, STRING_NO_PERMISSION);
            checkIP(conn, true);
        }   //admin console plain HTML
    }
    else 
        returnPage(conn, true);

    fflush(log_file); // flush the buffer
}

static int eventHandler(struct mg_connection *conn, enum mg_event ev) {
    if (ev == MG_REQUEST) {
        sendReply(conn);
        return MG_TRUE;
    }
    else if (ev == MG_AUTH) {
        return MG_TRUE;
    }
    else {
        return MG_FALSE;
    }
}

int main(int argc, char *argv[]){
    strcpy(siteTitle, "");
    char lport[64] = "8080";
    char zPath[64] = "default.db"; 
    log_file = stdout;

    for (int i = 0; i < argc; ++i){
        // if(TEST_ARG("--help", "-h")){
        //     puts(help_text);
        //     return 0;
        // }

        if(TEST_ARG("--database", "-d")){
            strncpy(zPath, argv[++i], 64);
            continue;
        }
        if(TEST_ARG("--log", "-l")){
            log_file = fopen(argv[++i], "a");
            logLog(">>>>>>>> Log File: %s(%d) <<<<<<<<", argv[i], log_file);
            continue;
        }
    }

    int rc = unqlite_open(&pDb, zPath, UNQLITE_OPEN_CREATE);
    if (rc != UNQLITE_OK) Fatal("Out of memory");

    logLog("Database: '%s'", zPath);

    //generate a random admin password
    adminPassword = new char[11];

    unqlite_util_random_string(pDb, adminPassword, 10); adminPassword[10] = 0;
    strcpy(adminCookie, adminPassword);
    strcpy(banListPath, "ipbanlist");

    for (int i = 0; i < argc; ++i){
        if(TEST_ARG("--reset", "-r"))           { logLog("Database: [New Profile]"); resetDatabase(pDb); }
        if(TEST_ARG("--title", "-t"))           { strncpy(siteTitle,     argv[++i], 64); continue; }
        if(TEST_ARG("--admin-spell", "-a"))     { strncpy(adminPassword, argv[++i], 11); continue; }
        if(TEST_ARG("--port", "-p"))            { strncpy(lport,         argv[++i], 64); continue; }
        if(TEST_ARG("--salt", "-m"))            { strncpy(md5Salt,       argv[++i], 64); continue; }
        if(TEST_ARG("--admin-cookie","-A"))     { strncpy(adminCookie,   argv[++i], 64); continue; }
        if(TEST_ARG("--ban-list", "-b"))        { strncpy(banListPath,   argv[++i], 64); continue; }
        if(TEST_ARG("--tpp", "-T"))             { threadsPerPage =  atoi(argv[++i]);    continue; }
        if(TEST_ARG("--cd-time", "-c"))         { postCDTime =      atoi(argv[++i]);    continue; }
        if(TEST_ARG("--max-image-size", "-I"))  { maxFileSize =     atoi(argv[++i]);    continue; }
        if(TEST_ARG("--stop-cookie", "-s"))     stopNewcookie = true;
        if(TEST_ARG("--stop-ipcheck", "-i"))    stopCheckIP = true;
        if(TEST_ARG("--xff-ip", "-x"))          useXFF = true;
    }

    logLog("Site Title: '%s' -> Admin Password: '%s'", siteTitle, adminPassword);
    logLog("Threads Per Page: %d", threadsPerPage);
    logLog("Cooldown Time: %ds", postCDTime);
    logLog("MD5 Salt: '%s'", md5Salt);
    logLog("Admin Cookie: '%s'", adminCookie);

    std::ifstream f(banListPath);
    string line;
    while (f >> line) {
        if(line != "") {
            if(!line.compare(0, 3, "ID:"))
                IDBanList.insert(line.substr(3));
            else
                IPBanList.insert(line);
        }
    }
    f.close();

    logLog("IP/ID Ban List: '%s' -> Total: %d/%d", banListPath, IPBanList.size(), IDBanList.size());    

    struct mg_server *server = mg_create_server(NULL, eventHandler);

    mg_set_option(server, "listening_port", lport);

    logLog("Listening on Port: '%s'", mg_get_option(server, "listening_port"));

    fflush(log_file);

    time(&gStartupTime);

    for (;;) {
        mg_poll_server(server, 1000);
    }

    mg_destroy_server(&server);
    unqlite_close(pDb);

    return 0;
}