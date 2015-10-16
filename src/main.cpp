//unsafe

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_set>
#include <deque>
#include <fstream>
#include <chrono>
#include <sys/stat.h>
#include <signal.h>

extern "C" {
#include "../lib/unqlite/unqlite.h"
#include "../lib/mongoose/mongoose.h"
}

#include "lang.h"
#include "tags.h"

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
string  admin_password;              // * administrator's password
string  admin_cookie;
char    siteTitle[64];              //site's title
char    md5Salt[64]     = "coyove"; // * MD5 salt
// char    adminCookie[64];            //Admin's cookie
char    thumbPrefix[64];
int     threads_per_page  = 5;        //threads per page to show
int     postCDTime      = 20;       //cooldown time
int     maxFileSize     = 2;        //megabytes
int     maxPageShown    = 0;
int     max_replies  = 1000;
bool    stopNewcookie   = false;    //stop delivering new cookies
bool    stop_check_ip     = false;    //stop checking ip
// bool    useXFF          = false;
bool    archiveMode     = false;
FILE*   log_file;                   //log file

char                    banListPath[64];//file to store ip ban list
unordered_set<string>   IDBanList;      //cookie ban list
unordered_set<string>   IPBanList;      //ip ban list
map<string, cclong>     IPAccessList;   //remote ip list
deque<struct History *> chatHistory;

struct mg_server *server;

TemplateManager templates;

#define TEST_ARG(b1, b2) (strcmp(argv[i], b1) == 0 || strcmp(argv[i], b2) == 0)

static const unsigned long long BUILD_DATE = __BUILD_DATE;

const char * get_client_ip(mg_connection* conn){
    const char * xff = mg_get_header(conn, "X-Forwarded-For");
    return (xff ? xff : conn->remote_ip);
}

bool is_admin(mg_connection* conn){
    return (extract_ssid(conn) == admin_cookie);
}

void site_footer(mg_connection* conn, float e_time = 0.0f){
    time_t c; 
    time(&c);

    unsigned long dummy;
    unsigned long res = 0;

    FILE *f = fopen("/proc/self/statm", "r");
    if(f){
        fscanf(f,"%ld %ld %ld %ld %ld %ld %ld", &dummy, &res, &dummy, &dummy, &dummy, &dummy, &dummy);
        fclose(f);
    }
    
    templates.invoke("site_footer") \
        .var("COOKIE", stopNewcookie? "0" : "1") \
        .var("RUNNING_TIME", (int)((c - gStartupTime) / 3600)) \
        .var("MEMORY", res * 4) \
        .var("TIME", (int)(e_time * 1000)) \
        .pipe_to(conn).destory();
}

void printMsg(mg_connection* conn, const char* msg, ...){
    templates.invoke("site_header").pipe_to(conn).destory();
    
    char tmpbuf[512]; //=w=
    va_list argptr;
    va_start(argptr, msg);
    vsprintf(tmpbuf, msg, argptr);
    va_end(argptr);

    templates.invoke("info_page") \
        .var("CONTENT", string(tmpbuf)).toggle("info_page").pipe_to(conn).destory();

    site_footer(conn);
}

bool checkIP(mg_connection* conn, bool verbose = false){
    if(stop_check_ip) return true;

    time_t rawtime;
    time(&rawtime);
    const char * cip = get_client_ip(conn);
    string sip(cip);
    time_t lasttime = IPAccessList[sip];
    
    if(strstr(conn->uri, admin_password.c_str())) return true;
    
    if(is_admin(conn)) return true;

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

#define SEND_IS_REPLY           1
#define SEND_SHOW_REPLY_LINK    2
#define SEND_CUT_LONG_COMMENT   4
#define SEND_CUT_IMAGE          8
#define SEND_CUT_REPLY_COUNT    16

void sendThread(mg_connection* conn, struct Thread* r, char display_state, bool admin_view = false, char* iid = ""){
    struct tm post_date;
    struct tm today;
    time_t now = time(NULL);

    localtime_r(&(r->date), &post_date);
    localtime_r(&now, &today);

    char std_time[64];
    strftime(std_time, 64, "%X", &post_date);

    //the date and time
    char timetmp[64];
    time_t diff = now - (today.tm_sec + today.tm_min * 60 + today.tm_hour * 3600) -
                  (r->date - (post_date.tm_sec + post_date.tm_min * 60 + post_date.tm_hour * 3600));
    cclong diff_day = diff / 3600 / 24;

    // switch(diff_day){
    //     case 0: strcpy(timetmp, STRING_TODAY); strcat(timetmp, std_time); break;
    //     case 1: strcpy(timetmp, STRING_YESTERDAY); strcat(timetmp, std_time); break;
    //     case 2: strcpy(timetmp, STRING_DAY_BEFORE_YESTERDAY); strcat(timetmp, std_time); break;
    //     default:
    //         strftime(timetmp, 64, TIME_FORMAT, &ti);
    // }

    bool reply      = display_state & SEND_IS_REPLY;
    bool show_reply = display_state & SEND_SHOW_REPLY_LINK;
    bool cut_cclong = display_state & SEND_CUT_LONG_COMMENT;
    bool cut_image  = display_state & SEND_CUT_IMAGE;
    bool cut_count  = display_state & SEND_CUT_REPLY_COUNT;

    // char crl[128] = {'\0'};
    // if(archiveMode)
    //     snprintf(crl, 127, show_reply ? "[ <a href=\"/thread/%d\">"STRING_VIEW"</a> ]" : "", r->threadID); 
    // else
    //     snprintf(crl, 127, show_reply ? "[ <a href=\"/thread/%d\">"STRING_REPLY"</a> ]" : "", r->threadID); 
    
    // char reply_count[256] = {'\0'};
    // if(r->childThread && !cut_count){
    //     struct Thread* c = readThread_(pDb, r->childThread);
    //     snprintf(reply_count, 255,
    //         "<a class='dcyan' href='/thread/%d'>&#128172;&nbsp;"SRTING_THREAD_REPLIES"</a><br/>", 
    //         r->threadID, c->childCount); 

    //     delete c;
    // }

    // char ref_or_link[64] = {'\0'};
    // if(show_reply)
    //     snprintf(ref_or_link, 63, "<a href='javascript:qref(%d)'>No.%d</a>", r->threadID, r->threadID);
    // else
    //     snprintf(ref_or_link, 63, "<a href='/thread/%d'>No.%d</a>", r->threadID, r->threadID);

    // char display_image[256] = {'\0'};

    // if (strlen(r->imgSrc) >= 4){
    //  string fname(r->imgSrc);
        
    //     if (endsWith(fname, ".jpg") || endsWith(fname, ".gif") || endsWith(fname, ".png")){
       //      if(cut_image){
       //          struct stat st;
       //          stat(("images/" + fname).c_str(), &st);

       //          snprintf(display_image, 255, 
       //           "<div class='img'>"
       //               "<a id='img-%d' href='javascript:void(0)' onclick='exim(\"img-%d\",\"%s\")'>["STRING_VIEW_IMAGE" (%d kb)]</a>"
       //           "</div>", 
       //              r->threadID, r->threadID, r->imgSrc, st.st_size / 1024);
       //      }
       //      else{
                
       //          snprintf(display_image, 255, 
       //               "<div class='img'>"
       //                   "<a id='img-%d' href='javascript:void(0)' onclick=\"enim('img-%d','/images/%s','%s%s')\">"
       //                       "<img class='%s' src='%s%s'/>"
       //                   "</a>"
       //               "</div>", 
    //                     r->threadID, r->threadID, r->imgSrc, thumbPrefix, r->imgSrc,
    //                     reply ? "img-s" : "img-n", thumbPrefix, r->imgSrc);
       //      }
       //  }else{
       //   snprintf(display_image, 255, 
       //           "<div class='img file'>"
       //               "<a class='wp-btn' href='/images/%s'>"STRING_VIEW_FILE"</a>"
       //           "</div>", r->imgSrc);
    //     }
    // }

    
    // char admin_ctrl[128] = {'\0'};
    // if(admin_view)
    //     snprintf(admin_ctrl, 127, "&nbsp;<span id='admin-%d'><a class='wp-btn' onclick='javascript:admc(%d)'>%s</a></span>", r->threadID, r->threadID, r->email);

    //if the content is too cclong to display
    char c_content[1050] = {'\0'};
    char* content = readString(pDb, r->content);
    strncpy(c_content, content, 1000);
    c_content[1000] = 0;

    // char thread_title[128] = {0};
    // if(reply && strcmp(r->author, STRING_UNTITLED) == 0){}
    // else
    //     strcpy(thread_title, r->author);

    if (strlen(content) > 1000) strcat(c_content, "<font color='red'><b>[...]</b></font>");

    // if (r->state & NORMAL_DISPLAY || admin_view)
    //     len = snprintf(tmp, 8192,
    //         "<div>%s"
    //          "<div %s>"
    //              /*image*/
    //              "%s"
    //              /*thread header*/
    //              "<div class='reply-header'>%s&nbsp;<ttt>%s</ttt>&nbsp;"
    //              "<span class='tmsc'><ssid>%s%s</ssid> "STRING_POSTED_AT" %s</span>&nbsp;%s%s"
    //              "</div>"
    //              /*thread comment*/
    //              "<div class='quote'>%s</div>"
    //              "%s"
    //              "%s"
    //              "%s"
    //              "%s"
    //          "</div>"
    //         "</div>",
    //         /*place holder*/
    //         reply                                ? "<div class='holder'><holder></holder></div>" : "", 
    //         reply                                ? "class='thread header'" : "class='thread'",
    //         /*image*/
    //         display_image, 
    //         ref_or_link, thread_title, 
    //         // reply                               ? "" : "ID:",
    //         (strcmp(r->ssid, "Admin") == 0)  ? "<red>"STRING_ADMIN"</red>" : r->ssid, 
    //         (strcmp(r->ssid, iid) == 0)          ? "<pox>"STRING_POSTER"</pox>" : "", 
    //         timetmp, 
    //         crl, 
    //         admin_ctrl,
    //         /*do we cut long comment?*/
    //         cut_cclong                           ? c_content : content, 
    //         /*thread state*/
    //         (r->state & SAGE_THREAD && !reply)   ? "<red><b>&#128078;&nbsp;"STRING_THREAD_SAGED"</b></red><br/>" : "", 
    //         (r->state & LOCKED_THREAD)           ? "<red><b>&#128274;&nbsp;"STRING_THREAD_LOCKED"</b></red><br/>" : "", 
    //         !(r->state & NORMAL_DISPLAY)         ? "<red><b>&#10006;&nbsp;"STRING_THREAD_DELETED"</b></red><br/>" : "",
    //         /*reply count*/
    //         reply_count);
    // else
    //     len = snprintf(tmp, 8192,
    //         "<div>%s"
    //             "<div %s>"
    //                 "<div class='reply-header'>No.%d&nbsp;<ttt>%s</ttt>&nbsp;"
    //                 "<span class='tmsc'><ssid>%s%s</ssid> "STRING_POSTED_AT" %s</span>&nbsp;%s"
    //                 "</div>"
    //                 "<div class='alert-box'>"STRING_THREAD_DELETED2"</div>"
    //             "</div>"
    //         "</div>",
    //         /*place holder*/
    //         reply                               ? "<div class='holder'>&nbsp;&nbsp;</div>" : "", 
    //         reply                               ? "class='thread header'" : "class='thread'",
    //         r->threadID, thread_title, 
    //         // reply                               ? "" : "ID:",
    //         (strcmp(r->ssid, "Admin") == 0)     ? "<red>"STRING_ADMIN"</red>" : r->ssid, 
    //         (strcmp(r->ssid, iid) == 0)         ? "<pox>"STRING_POSTER"</pox>" : "", 
    //         timetmp, 
    //         admin_ctrl);

    // mg_send_data(conn, tmp, len);

    HTMLTemplate *ht = templates.invoke_pointer("single_thread");

    map<string, bool> stats;
    map<string, string> vars;

    stats["reply"]                      = reply;
    stats["show_reply"]                 = show_reply;
    stats["archive"]                    = archiveMode;
    stats["normal_display"]             = r->state & NORMAL_DISPLAY || admin_view;
    stats["thread_poster_is_admin"]     = (strcmp(r->ssid, "Admin") == 0);
    stats["thread_poster_is_sameone"]   = (strcmp(r->ssid, iid) == 0);
    stats["sage"]                       = (r->state & SAGE_THREAD && !reply);
    stats["lock"]                       = (r->state & LOCKED_THREAD);
    stats["delete"]                     = !(r->state & NORMAL_DISPLAY);
    stats["show_admin"]                 = admin_view;
    if (strlen(r->imgSrc) >= 4){
        string fname(r->imgSrc);
        vars["THREAD_IMAGE"]            = fname;

        if (endsWith(fname, ".jpg") || endsWith(fname, ".gif") || endsWith(fname, ".png")){
            stats["image_attached"]     = true;
            if(cut_image){
                struct stat st;
                stat(("images/" + fname).c_str(), &st);
                stats["show_size_only"] = true;
                vars["THREAD_IMAGE_SIZE"] = to_string((int)(st.st_size / 1024));
            }
            else{
                stats["show_full_image"] = true;
                vars["THREAD_THUMB_PREFIX"] = string(thumbPrefix);
            }
        }else{
            stats["file_attached"]      = true;
        }
    }
    if(r->childThread && !cut_count){
        struct Thread* c = readThread_(pDb, r->childThread);
        stats["show_num_replies"]       = true;
        vars["NUM_REPLIES"]             = to_string(c->childCount);
        delete c;
    }

    vars["THREAD_POSTER"]               = string(r->ssid);
    vars["THREAD_NO"]                   = to_string(r->threadID);
    vars["THREAD_CONTENT"]              = string(cut_cclong ? c_content : content);
    vars["THREAD_TITLE"]                = string((reply && strcmp(r->author, STRING_UNTITLED) == 0) ? "" : r->author);
    vars["THREAD_IP"]                   = string(r->email);
    vars["THREAD_POST_TIME"]            = string(std_time);

    if(diff_day >= 0 && diff_day <= 2){
        stats["show_easy_date"]         = true;
        vars["THREAD_POST_DATE"]        = to_string(diff_day);
    }else{
        strftime(timetmp, 64, TIME_FORMAT, &post_date);
        vars["THREAD_POST_DATE"]        = string(timetmp);
    }

    map<string, queue<string>> loops; //nothing

    mg_printf_data(conn, "%s", ht->build(vars, stats, loops).c_str());

    if(content) delete [] content;
    delete ht;
}

void requestAdminConsole(mg_connection* conn, struct Thread* r){
    templates.invoke("admin_console") \
        .var("THREAD_NO", r->threadID) \
        .var("THREAD_IP", r->email) \
        .var("THREAD_POSTER", r->ssid) \
        .var("THREAD_IMAGE", r->imgSrc) \
        .var("THREAD_STATE", (unsigned int)r->state) \
        .pipe_to(conn).destory();
}

void showGallery(mg_connection* conn, cclong startID, cclong endID){
    struct Thread *r = readThread_(pDb, 0); // get the root thread
    cclong c = 0;

    bool admin_view = is_admin(conn);

    cclong totalThreads = r->childCount;
    // cclong totalPages = 100;

    templates.invoke("site_slogan").pipe_to(conn).destory();

    for(cclong i = totalThreads; i > 0; --i){
        delete r;
        r = readThread_(pDb, i);
        if(strlen(r->imgSrc) == 15 && r->state & NORMAL_DISPLAY) {
            c++;
            if (c >= startID && c <= endID){
                mg_printf_data(conn, "<hr>");
                sendThread(conn, r, SEND_SHOW_REPLY_LINK + SEND_CUT_LONG_COMMENT, admin_view);
            }
        }
        
        if (c == endID + 1) break;
    }
    if(r) delete r;
    
    cclong current_page = endID / threads_per_page;
    queue<string> before, after;
    for (cclong i = 1; i <= (maxPageShown == 0 || admin_view ? 10000 : maxPageShown); ++i){
        if (current_page - i < 4 && current_page - i > 0) before.push(to_string(i));
        else if (i - current_page < 4 && i - current_page > 0) after.push(to_string(i));
        else if (i - current_page > 4) break;
    }
    
    templates.invoke("pager").toggle("gallery_page") \
        .var("CURRENT_PAGE", current_page) \
        .var("PREVIOUS_PAGE", current_page > 1 ? (current_page - 1) : 1) \
        .var("NEXT_PAGE", current_page + 1) \
        .loop("before_pages", before).loop("after_pages", after).pipe_to(conn).destory();
}

void showThreads(mg_connection* conn, cclong startID, cclong endID){
    struct Thread *r = readThread_(pDb, 0); // get the root thread
    cclong c = 0;
    // clock_t startc = clock();

    bool admin_view = is_admin(conn);

    cclong totalThreads = r->childCount;
    // cclong totalPages;
    // if (totalThreads <= threads_per_page)
    //     totalPages = 1;
    // else if (totalThreads % threads_per_page == 0)
    //     totalPages = (cclong)(totalThreads / threads_per_page);
    // else
    //     totalPages = (cclong)(totalThreads / threads_per_page) + 1;

    templates.invoke("site_slogan").pipe_to(conn).destory();

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
                struct Thread* first_thread;
                first_thread = readThread_(pDb, r->childThread); // beginning of the circle
                r = first_thread;

                vector<struct Thread *> vt;
                // vector<struct Thread *> vt_next;

                int i = 1;
                while(i <= 4){
                    r = readThread_(pDb, r->prevThread);

                    if(r->threadID != first_thread->threadID) {
                        vt.push_back(r);
                        i++;
                    }else
                        break;
                }

                vt.push_back(first_thread);

                if(first_thread->childCount <= 5)
                    for (int i = vt.size() - 1; i >= 0; i--){
                        sendThread(conn, vt[i], SEND_IS_REPLY + SEND_CUT_LONG_COMMENT, admin_view, iid);
                        delete vt[i];
                    }
                else{
                    for (int i = vt.size() - 1; i >= 0; i--){
                        sendThread(conn, vt[i], SEND_IS_REPLY + SEND_CUT_LONG_COMMENT, admin_view, iid);
                        delete vt[i];
                        if(i == vt.size() - 1){
                            // mg_printf_data(conn, 
                            //     "<div>"
                            //         "<div class='holder'>&nbsp;&nbsp;</div>"
                            //         "<div class='header'>"
                            //             "<div class='reply-header round'>"
                            //                 "<a class='dcyan' href='/thread/%d'>&#128172;&nbsp;"SRTING_THREAD_REPLIES_HIDE"</a>"
                            //             "</div>"
                            //         "</div>"
                            //     "</div>", oldr->threadID, first_thread->childCount - 5);
                            templates.invoke("expand_hidden_replies") \
                                .var("THREAD_NO", oldr->threadID) \
                                .var("NUM_HIDDEN_REPLIES", first_thread->childCount - 5) \
                                .pipe_to(conn).destory();
                        }
                    }
                }

            }

            r = oldr;
            delete [] iid;
        }
        
        if (c == endID + 1) break;
    }
    if(r) delete r;

    cclong current_page = endID / threads_per_page;
    queue<string> before, after;
    for (cclong i = 1; i <= (maxPageShown == 0 || admin_view ? 10000 : maxPageShown); ++i){
        if (current_page - i < 4 && current_page - i > 0) before.push(to_string(i));
        else if (i - current_page < 4 && i - current_page > 0) after.push(to_string(i));
        else if (i - current_page > 4) break;
    }
    
    templates.invoke("pager").toggle("timeline_page") \
        .var("CURRENT_PAGE", current_page) \
        .var("PREVIOUS_PAGE", current_page > 1 ? (current_page - 1) : 1) \
        .var("NEXT_PAGE", current_page + 1) \
        .loop("before_pages", before).loop("after_pages", after).pipe_to(conn).destory();
}

void showThread(mg_connection* conn, cclong id, bool reverse = false){ 
    struct Thread *r = readThread_(pDb, id); // get the root thread
    cclong c = 0;

    bool admin_view = is_admin(conn);

    sendThread(conn, r, 0, admin_view);
    mg_printf_data(conn, "<hr>");

    char iid[10];
    strcpy(iid, r->ssid);

    if(!archiveMode) {
        templates.invoke("post_form") \
            .var("THREAD_NO", to_string(id)).toggle("reply_to_thread").toggle("is_admin", admin_view).pipe_to(conn).destory();
    }
    // if(admin_view) ADMIN_VIEW();
        
    if (r->childThread) {
        cclong r_childThread = r->childThread;
        delete r;

        r = readThread_(pDb, r_childThread); // beginning of the circle
        cclong rid = r->threadID; //the ID
        bool too_many_replies = (r->childCount > 20);

        int di = 1;

        if(reverse){
            cclong n_rid = r->prevThread;
            delete r;
            r = readThread_(pDb, n_rid);

            sendThread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK, admin_view, iid);

            while (r->prevThread != n_rid){
                di++;
                cclong r_prevThread = r->prevThread;

                delete r;
                r = readThread_(pDb, r_prevThread);

                if(too_many_replies && (di > 20))
                    sendThread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK + SEND_CUT_IMAGE, admin_view, iid);
                else
                    sendThread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK, admin_view, iid);
            }
        }else{
            sendThread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK, admin_view, iid);

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
    }
    /*else{
        mg_printf_data(conn, "<i>no reply yet</i><hr>", r->childCount);
    }*/

    // clock_t endc = clock();
    // mg_printf_data(conn, "Completed in %.3lfs<br/>", (float)(endc - startc) / CLOCKS_PER_SEC);
    // PRINT_TIME();
}

bool userDeleteThread(mg_connection* conn, cclong tid, bool admin = false){
    string username = verify_cookie(conn);
    bool flag = false;

    if (!username.empty()){
        struct Thread* t = readThread_(pDb, tid);

        if (strcmp(t->ssid, username.c_str()) == 0 || admin){
            deleteThread(pDb, tid);
            // printMsg(conn, STRING_DELETE_SUCCESS, tid);
            logLog("Thread No.%d Deleted", tid);
            flag = true;
        }
        else{
            // printMsg(conn, STRING_NO_PERMISSION);
            flag = false;
        }

        if(t) delete t;
    }else
        // printMsg(conn, STRING_NO_PERMISSION);
        flag = false;

    return flag;
}

void userListThread(mg_connection* conn, bool admin_view = false){
    string username = verify_cookie(conn);

    templates.invoke("site_header").toggle("my_post_page").pipe_to(conn).destory();

    if (!username.empty()){
        struct Thread *r = readThread_(pDb, 0); 
        struct Thread *t;

        clock_t startc = clock();
        for(cclong i = r->childCount; i > 0; i--){
            t = readThread_(pDb, i);
            if(r->childCount - i > 500 && !admin_view) break; // only search the most recent 500 threads/replies

            if(strcmp(username.c_str(), t->ssid) == 0 || admin_view)
                //sendThread(conn, t, true, false, true, true, false);
                sendThread(conn, t, SEND_IS_REPLY + SEND_CUT_LONG_COMMENT + SEND_CUT_IMAGE, admin_view);

            if(t) delete t;
        }
        clock_t endc = clock();
        // PRINT_TIME();
        site_footer(conn, (float)(endc-startc)/CLOCKS_PER_SEC);

        if(r) delete r;
    }else{
        site_footer(conn);
    }
}

void postSomething(mg_connection* conn, const char* uri){
    char var1[64] = {'\0'}, var2[4096] = {'\0'}, var3[64] = {'\0'}, var4[16] = {'\0'};
    bool sage = false;
    bool fileAttached = false;
    bool user_delete = false;
    const char *data;
    int data_len, ofs = 0, mofs = 0;
    char var_name[100], file_name[100];

    //first check the ip
    if(!checkIP(conn)) return;
    if(archiveMode) return;
    bool admin_ctrl = is_admin(conn);
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

    #define GET_VAR(x, l, v) \
        if (strcmp(var_name, x) == 0) { \
            data_len = data_len > l ? l : data_len; \
            strncpy(v, data, data_len); \
            v[data_len] = 0; }

    while ((mofs = mg_parse_multipart(conn->content + ofs, conn->content_len - ofs,
        var_name, sizeof(var_name),
        file_name, sizeof(file_name),
        &data, &data_len)) > 0) {
        
        ofs += mofs;
        GET_VAR("input_name", 63, var1);
        // if (strcmp(var_name, "input_name") == 0) {  //var1: the subject of the thread
        //     data_len = data_len > 63 ? 63 : data_len;
        //     strncpy(var1, data, data_len);
        //     var1[data_len] = 0;
        // }
        GET_VAR("input_email", 63, var3);
        // if (strcmp(var_name, "input_email") == 0) { //var3: the options
        //     data_len = data_len > 63 ? 63 : data_len;
        //     strncpy(var3, data, data_len);
        //     var3[data_len] = 0;
        // }
        GET_VAR("input_content", 4095, var2);
        // if (strcmp(var_name, "input_content") == 0) { //var2: the comment
        //     data_len = data_len > 4095 ? 4095 : data_len;
        //     strncpy(var2, data, data_len);
        //     var2[data_len] = 0;
        // }

        if (strcmp(var_name, "input_file") == 0 && strcmp(file_name, "") != 0) {    //var4: the image attached (if has)
            // strcpy(var4, "");
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
                ext = ".dat";
            else if (admin_ctrl){
                ext = sfname.substr(sfname.size() - 4);
            }
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

    #undef GET_VAR

    //see if there is a SPECIAL string in the text field
    if (strcmp(var1, "") == 0) strcpy(var1, STRING_UNTITLED);
    //user trying to sega a thread/reply
    if (strstr(var3, "sage")) sage = true;
    //user trying to delete a thread/reply
    if (strstr(var3, "delete")){
        cclong id = extractLastNumber(conn);
        struct Thread * t = readThread_(pDb, id); //what he replies to is which he wants to delete
        if(userDeleteThread(conn, id, admin_ctrl))
            printMsg(conn, STRING_DELETE_SUCCESS, id);
        else
            printMsg(conn, STRING_NO_PERMISSION);
        return;
    }
    if (strstr(var3, "list")){
        userListThread(conn);
        return;
    }
    if (strstr(var3, "url")){
        templates.invoke("site_header").toggle("upload_image_page").pipe_to(conn).destory();
        templates.invoke("info_page").toggle("upload_image_page").var("IMAGE", var4).pipe_to(conn).destory();
        site_footer(conn);

        logLog("Image Uploaded But Not As New Thread");
        return;
    }
    //admin trying to update a thread/reply
    if (strstr(var3, "update") && admin_ctrl){
        cclong id = extractLastNumber(conn);
        struct Thread * t = readThread_(pDb, id); 
        //what admin replies to is which he wants to update
        //note the server doesn't filter the special chars such as "<" and ">"
        //so it's possible for admin to use HTML here
        if(!strstr(var3, "html")){
            string up_str = replaceAll(string(var2), string("\n"), string("<br>"));
            writeString(pDb, t->content, up_str.c_str(), true); 
        }else
            writeString(pDb, t->content, var2, true); 

        printMsg(conn, "Thread No.%d updated successfully", t->threadID);
        logLog("Admin Edited Thread No.%d", t->threadID);
        return;
    }
    
    char ipath[64]; FILE *fp; struct stat st;
    sprintf(ipath, "images/%s", var3);
    
        //image or comment or both
    if (strcmp(var2, "") == 0 && !fileAttached) {
        printMsg(conn, STRING_EMPTY_COMMENT);
        return;
    }else if (strcmp(var2, "") == 0 && fileAttached){
        strcpy(var2, STRING_UNCOMMENTED);
    }
    // }
    //logLog("addr: %d %d %d %d %d", var1, var2, var3, var4, ipath);
    //verify the cookie
    // char ssid[100];//, username[10];
    string username = verify_cookie(conn);
    string ssid = extract_ssid(conn);
    // mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, sizeof(ssid));

    if (!username.empty()){  //passed
        string findID = username;
        // if(strcmp(ssid, adminCookie) != 0)
        if (IDBanList.find(findID) != IDBanList.end()){
            //this id is banned, so we destory it
            destoryCookie(conn);
            logLog("Cookie Destoryed: '%s'", ssid.c_str());
            printMsg(conn, STRING_YOUR_ID_IS_BANNED);
            return;
        }
    }else{
        if (stopNewcookie){
            printMsg(conn, STRING_STOP_COOKIE);
            return;
        }
        else{
            username = random_9chars();
            ssid = to_ssid(username);
        }
    }

    // if (strcmp(var3, adminPassword) == 0) strcpy(username, "Admin");
    if (admin_ctrl) username = "Admin";

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
                    imageDetector[i] = "<ttt>" + imageDetector[i] + "</ttt>";
        if(refFlag)
            tmpcontent += imageDetector[i];
        else
            tmpcontent += (imageDetector[i] + "<br/>");
    }

    const char * cip = get_client_ip(conn);
    logLog("New Thread: (Sub: '%s', Opt: '%s', Img: '%s', Dump: '%s', IP: '%s')", var1, var3, var4, dump_name, cip);

    if (strstr(conn->uri, "/post_reply/")) {
        cclong id = extractLastNumber(conn);
        struct Thread* t = readThread_(pDb, id);

        if(t->state & TOOMANY_REPLIES){
            printMsg(conn, STRING_NO_REPLY_TO_TOOMANY, max_replies);
            delete t;
            return;
        }

        if(t->childThread){
            struct Thread* tc = readThread_(pDb, t->childThread);
            if(tc->childCount >= max_replies - 1){
                changeState(t, TOOMANY_REPLIES, true);
                writeThread(pDb, t->threadID, t, true);
            }
            delete tc;            
        }

        if(t->state & LOCKED_THREAD)
            printMsg(conn, STRING_NO_REPLY_TO_LOCKED);
        else{
            newReply(pDb, id, tmpcontent.c_str(), var1, cip, username.c_str(), var4, sage);
            
            mg_printf(conn, "HTTP/1.1 302 Moved Temporarily\r\nLocation: /success/%s/%d\r\n\r\n", ssid.c_str(), id);
        }

        if(t) delete t;
    }
    else{
        newThread(pDb, tmpcontent.c_str(), var1, cip, username.c_str(), var4, sage);
        //printMsg(conn, "Successfully start a new thread");
        mg_printf(conn, "HTTP/1.1 302 Moved Temporarily\r\nLocation: /success/%s/0\r\n\r\n", ssid.c_str());
    }
}

void returnPage(mg_connection* conn, bool indexPage, bool galleryPage = false){

    int num = indexPage ? 1 : extractLastNumber(conn);

    templates.invoke("site_header").var("CURRENT_PAGE", num) \  
        .toggle(!galleryPage ? "timeline_page" : "gallery_page").pipe_to(conn).destory();

    if(!archiveMode) 
        templates.invoke("post_form").toggle("is_admin", is_admin(conn)).toggle("post_new_thread").pipe_to(conn).destory();

    clock_t startc = clock();

    if(indexPage)
        showThreads(conn, 1, threads_per_page);
    else{
        cclong cp = extractLastNumber(conn);
        if(cp <= maxPageShown || maxPageShown == 0 || is_admin(conn)){
            cclong j = cp * threads_per_page;
            cclong i = j - threads_per_page + 1;
            if(galleryPage)
                showGallery(conn, i, j);
            else
                showThreads(conn, i, j);
        }
    }

    clock_t endc = clock();
    site_footer(conn, (float)(endc-startc)/CLOCKS_PER_SEC);
}

void admin_actions(struct mg_connection *conn){
    if(conn->content_len <= 0){
        printMsg(conn, STRING_UNKNOWN_ERROR);
        return;
    }
    char action_name[32] = {0}, action_param1[32] = {0}, action_param2[32] = {0};
    string action, param1, param2;

    mg_get_var(conn, "action_name", action_name, sizeof(action_name)); action = string(action_name);
    mg_get_var(conn, "action_1", action_param1, sizeof(action_param1)); param1 = string(action_param1);
    mg_get_var(conn, "action_2", action_param2, sizeof(action_param2)); param2 = string(action_param2);

    if(action == ""){
        printMsg(conn, STRING_UNKNOWN_ERROR);
        return;
    }
    
    #define HAS_KEY(x) x == action

    string ret = "Action completed.";

    if(HAS_KEY("login") && param1 == admin_password){
        string ssid = extract_ssid(conn);

        string new_name = random_9chars();
        ssid = to_ssid(new_name.c_str());
        set_cookie(conn, ssid);

        // strncpy(adminCookie, ssid, 64);
        admin_cookie = ssid;
        logLog("Set New Admin Cookie '%s' by '%s'", admin_cookie.c_str(), get_client_ip(conn));
    }

    if(!is_admin(conn)) {
        printMsg(conn, STRING_NO_PERMISSION);
        checkIP(conn, true);
        return;
    }

    if(HAS_KEY("cookie"))               stopNewcookie = (param1 == "off");
    if(HAS_KEY("archive"))              archiveMode = (param1 == "on");
    if(HAS_KEY("acl"))                  stop_check_ip = (param1 == "off");
    if(HAS_KEY("max-page-viewable"))    maxPageShown = atol(param1.c_str());
    if(HAS_KEY("cd-time"))              postCDTime = atol(param1.c_str());
    if(HAS_KEY("threads-per-page"))     threads_per_page = atol(param1.c_str());
    if(HAS_KEY("max-replies"))          max_replies = atol(param1.c_str());
    if(HAS_KEY("quit-admin"))           {admin_cookie = random_9chars(); ret = "quitted";}
    if(HAS_KEY("new-password"))         admin_password = param1;
    if(HAS_KEY("delete-thread"))        userDeleteThread(conn, atol(param1.c_str()), true);
    if(HAS_KEY("max-image-size"))       maxFileSize = atol(param1.c_str());
    if(HAS_KEY("new-state") && !param1.empty()){
        int id = atol(param1.c_str());

        struct Thread * t = readThread_(pDb, id);
        t->state = atol(param2.c_str());
        writeThread(pDb, t->threadID, t, true);

        ret += (" New state: " + resolve_state(t->state));
        delete t;
    }
    if(HAS_KEY("ban")){
        string id = param1;
        unordered_set<string>& xlist = (param2 == "ip") ? IPBanList : IDBanList;

        auto iter = xlist.find(id);
        if (iter == xlist.end()){
            xlist.insert(id);
            ret += (" " + id + " banned");
        }
        else{
            xlist.erase(iter);
            ret += (" " + id + " unbanned");
        }

        std::ofstream f(banListPath);
        for(auto i = IPBanList.begin(); i != IPBanList.end(); ++i) f << *i << '\n';
        for(auto j = IDBanList.begin(); j != IDBanList.end(); ++j) f << "ID:" << *j << '\n';
        f.close();
    }
    if(HAS_KEY("hide-image")){
        struct Thread * t = readThread_(pDb, atol(param1.c_str()));
        
        string oldname = "images/" + string(t->imgSrc);
        string newname = oldname + ".tmp";
        rename(oldname.c_str(), newname.c_str());

        delete t;
    }
    if(HAS_KEY("kill-all")){
        string id = param1;

        struct Thread *r = readThread_(pDb, 0); 
        struct Thread *t;
        bool ipflag = (param2 == "ip");

        clock_t startc = clock();
        for(cclong i = r->childCount; i > 0; i--){
            t = readThread_(pDb, i);
            if(ipflag){
                if(strcmp(id.c_str(), t->email) == 0) deleteThread(pDb, i);
            }else{
                if(strcmp(id.c_str(), t->ssid) == 0) deleteThread(pDb, i);
            }

            if(t) delete t;
        }
        clock_t endc = clock();

        if(r) delete r;
        ret += (" Take " + to_string((float)(endc-startc)/CLOCKS_PER_SEC));
    }

    if(HAS_KEY("search")){
        struct Thread *t, *r = readThread_(pDb, 0); 
        int c = 0, limit = atol(param2.c_str());
        ret = "[";

        for(cclong i = r->childCount; i > 0; i--){
            if(++c == limit) break;
            t = readThread_(pDb, i);
            char* content = readString(pDb, t->content);
            if(content && strstr(content, param1.c_str())){
                ret += ("\"" + to_string(t->threadID) + "\",");
                delete content;
            }
            delete t;
        }

        ret += "\"\"]";

        if(r) delete r;
    }
    
    mg_send_header(conn, "Content-Type", "text/plain");
    mg_printf_data(conn, ret.c_str());
}

static void sendReply(struct mg_connection *conn) {

    if (strcmp(conn->uri, "/post_thread") == 0) postSomething(conn, conn->uri); //post a thread
    else if (strstr(conn->uri, "/post_reply/")) postSomething(conn, conn->uri); //post a reply
    else if (strstr(conn->uri, "/page/"))       returnPage(conn, false);        //view page
    else if (strstr(conn->uri, "/gallery/"))    returnPage(conn, false, true);
    else if (strstr(conn->uri, "/success/")){
        string url(conn->uri);
        vector<string> tmp = split(url, "/");

        if(tmp.size() != 3){
            printMsg(conn, "Error");
            return;
        }
        cclong id  = atol(tmp[tmp.size() - 1].c_str());

        set_cookie(conn, tmp[tmp.size() - 2]);
        mg_send_header(conn, "charset", "utf-8");
        mg_send_header(conn, "Content-Type", "text/html");
        mg_send_header(conn, "cache-control", "private, max-age=0");
        
        if(id == 0)
            printMsg(conn, STRING_NEW_THREAD_SUCCESS);
        else{
            templates.invoke("site_header").toggle("success_post_page").pipe_to(conn).destory();
            templates.invoke("info_page").var("THREAD_NO", id).toggle("return_page").pipe_to(conn).destory();

            site_footer(conn);
        }   //post successfully page
    }
    else if (strstr(conn->uri, "/thread/") || strstr(conn->uri, "/daerht/")){
        cclong id = extractLastNumber(conn);
        struct Thread* t = readThread_(pDb, id);
        cclong pid = findParent(pDb, t);

        if (pid == -1 && !is_admin(conn)){
            printMsg(conn, STRING_THREAD_DELETED);
            return ;
        }
        else{
        //     mg_send_header(conn, "charset", "utf-8");
        //     mg_send_header(conn, "Content-Type", "text/html");
            if (id < 0) {
                printMsg(conn, STRING_INVALID_ID);
                return;
            }

            templates.invoke("site_header").toggle("thread_page") \
                .var("THREAD_NO", id).var("THREAD_TITLE", t->author).pipe_to(conn).destory();

            templates.invoke("single_thread_header").toggle("homepage", !pid).var("THREAD_NO", id) \
                .toggle("thread", strstr(conn->uri, "/thread/")).pipe_to(conn).destory();

            clock_t startc = clock();
            showThread(conn, id, strstr(conn->uri, "/daerht/"));
            clock_t endc = clock();

            site_footer(conn, (float)(endc-startc)/CLOCKS_PER_SEC);
        }   //view a thread and its replies

        delete t;
    }
    else if (strstr(conn->uri, "/api/")){
        cclong id = extractLastNumber(conn);
        struct Thread *r = readThread_(pDb, id); // get the root thread
        bool admin_view = is_admin(conn);

        sendThread(conn, r, 0, admin_view);

        delete r;       //return a thread's plain HTML
    }
    else if (strstr(conn->uri, "/list")){
        if(!checkIP(conn, true)) return;

        string url(conn->uri);
        vector<string> tmp = split(url, "/");
        string id = tmp[tmp.size() - 1];

        if (is_admin(conn)){
            // the following actions could be extremely slow
            struct Thread *r = readThread_(pDb, 0); 
            struct Thread *t;
            bool ipflag = strstr(conn->uri, "/ip/");

            if (strcmp(conn->uri, "/list") == 0){
                userListThread(conn, true);
                return;
            }

            templates.invoke("site_header").toggle("admin_list_all_page").pipe_to(conn).destory();
            clock_t startc = clock();

            for(cclong i = r->childCount; i > 0; i--){
                t = readThread_(pDb, i);
                if(ipflag){
                    if(strcmp(id.c_str(), t->email) == 0) sendThread(conn, t, SEND_IS_REPLY + SEND_CUT_LONG_COMMENT + SEND_CUT_IMAGE, true);
                }else{
                    if(strcmp(id.c_str(), t->ssid) == 0) sendThread(conn, t, SEND_IS_REPLY + SEND_CUT_LONG_COMMENT + SEND_CUT_IMAGE, true);
                }

                if(t) delete t;
            }
            clock_t endc = clock();
            // PRINT_TIME();
            site_footer(conn, (float)(endc-startc)/CLOCKS_PER_SEC);

            if(r) delete r;
        }
        else{
            //printMsg(conn, "Your don't have the permission");
            userListThread(conn);
            //checkIP(conn, true);
        }       //list ID/IP, kill all posted by ID/IP
    }
    else if (strstr(conn->uri, "/images/")){
        const char *ims = mg_get_header(conn, "If-Modified-Since");
        // const char *inm = mg_get_header(conn, "If-None-Match");
        //logLog("[%s]\n", ims);
        if (ims)
            if (strcmp(ims, "") != 0){
                mg_printf(conn, "HTTP/1.1 304 Not Modified\r\n\r\n");
                return;
            }

        string url(conn->uri);
        vector<string> tmp = split(url, "/");
        string fname = tmp[tmp.size() - 1];
        string ctype;
        char ipath[64];
        strcpy(ipath, ("images/" + fname).c_str());

        if (endsWith(fname, ".jpg"))
            ctype = "image/jpeg";
        else if (endsWith(fname, ".gif"))
            ctype = "image/gif";
        else if (endsWith(fname, ".png"))
            ctype = "image/png";
        else
            ctype = "application/octet-stream";

        FILE *fp; struct stat st;
        char buf[1024];
        int n;

        // char expire[100];
        // time_t t = time(NULL) + 60 * 60 * 24 * 30;
        // strftime(expire, sizeof(expire), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));

        if (stat(ipath, &st) == 0 && (fp = fopen(ipath, "rb")) != NULL) {
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
    else if (strstr(conn->uri, "/adminconsole/")){
        if (is_admin(conn)){
            cclong id = extractLastNumber(conn);
            struct Thread * t = readThread_(pDb, id); 
            requestAdminConsole(conn, t);
            delete t;
        }else{
            printMsg(conn, STRING_NO_PERMISSION);
            checkIP(conn, true);
        }   //admin console plain HTML
    }
    else if (strcmp(conn->uri, "/admin") == 0){
        string ip_ban_list = "";
        string id_ban_list = "";
        for(auto i = IPBanList.begin(); i != IPBanList.end(); ++i) ip_ban_list += (*i + ",");
        for(auto j = IDBanList.begin(); j != IDBanList.end(); ++j) id_ban_list += (*j + ",");

        templates.invoke("site_header").toggle("admin_panel_page").pipe_to(conn).destory();
        templates.invoke("admin_panel") \
            .toggle("is_admin", is_admin(conn)) \
            .toggle("cookie", !stopNewcookie) \
            .toggle("archive", archiveMode) \
            .toggle("acl", !stop_check_ip) \
            .var("MAX_PAGES_VIEWABLE", maxPageShown) \
            .var("IP_BAN_LIST", ip_ban_list) \
            .var("ID_BAN_LIST", id_ban_list) \
            .var("COOLDOWN_TIME", postCDTime) \
            .var("MAX_IMAGE_SIZE", maxFileSize) \
            .var("THREADS_PER_PAGE", threads_per_page) \
            .var("MAX_REPLIES", max_replies) \
            .var("ADMIN_PASSWORD", admin_password) \
            .pipe_to(conn).destory();
        site_footer(conn);
    }
    else if (strcmp(conn->uri, "/admin_action") == 0){
        admin_actions(conn);
    }
    else 
        returnPage(conn, true);

    fflush(log_file); // flush the buffer
}

static void broadcastMessage(struct mg_connection *conn){
    struct chatData *d = (struct chatData *) conn->connection_param;
    struct mg_connection *c;

    time_t rawtime;
    time(&rawtime);

    struct History *h = new History();
    strcpy(h->chatterSSID, d->chatterSSID);

    cclong len = conn->content_len - 4;
    len = len > 1023 ? 1023 : len;
    strncpy(h->message, conn->content + 4, len);
    h->message[len] = 0;

    h->postTime = rawtime;

    chatHistory.push_front(h);
    while(chatHistory.size() > 20){
        struct History *h2 = chatHistory.back();
        delete h2;
        chatHistory.pop_back();
    }

    for (c = mg_next(server, NULL); c != NULL; c = mg_next(server, c)) {
        struct chatData *d2 = (struct chatData *) c->connection_param;
        if (!c->is_websocket || d2->roomID != d->roomID) continue;

        if(strcmp(d->chatterSSID, admin_cookie.c_str()) == 0)
            mg_websocket_printf(c, WEBSOCKET_OPCODE_TEXT, "msg %c Admin %d %.*s",
                      (char) d->roomID, rawtime, conn->content_len - 4, conn->content + 4);
        else
            mg_websocket_printf(c, WEBSOCKET_OPCODE_TEXT, "msg %c %.9s %d %.*s",
                      (char) d->roomID, d->chatterSSID, rawtime, len, h->message);
    }

    //delete [] bufMessage;
}

static void webChat(struct mg_connection *conn) {
    if(!conn->connection_param) return;

    struct chatData *d = (struct chatData *) conn->connection_param;

    char *username = verifyCookieStr(d->chatterSSID);
    if(!username) return;
    delete [] username;

    struct mg_connection *c;

  // printf("[%.*s]\n", (int) conn->content_len, conn->content);
    if (conn->content_len > 5 && !memcmp(conn->content, "join ", 5)) {
    // Client joined new room
        d->roomID = conn->content[5];
    } else if (conn->content_len > 4 && !memcmp(conn->content, "msg ", 4) &&
             d->roomID != 0 && d->roomID != '?') {
        broadcastMessage(conn);
    }
}

static int eventHandler(struct mg_connection *conn, enum mg_event ev) {
    string username, ssid;

    switch(ev){
        case MG_REQUEST:
            if (conn->is_websocket) {
                webChat(conn);
                return MG_TRUE;
            } 
            else if (strstr(conn->uri, "/assets/")){
                mg_send_file(conn, conn->uri + 1, "");
                return MG_MORE;
            }
            else{
                sendReply(conn);
                return MG_TRUE;
            }
        case MG_WS_CONNECT:
            username = verify_cookie(conn);
            ssid = extract_ssid(conn);

            conn->connection_param = new chatData();
            strcpy(((struct chatData *)conn->connection_param)->chatterSSID, ssid.c_str());

            if(!username.empty()){
                mg_websocket_printf(conn, WEBSOCKET_OPCODE_TEXT, "id %s", username.c_str());               
            }else
                mg_websocket_printf(conn, WEBSOCKET_OPCODE_TEXT, "id (null)");
            
            if(chatHistory.size() > 0){
                    // for(auto i = chatHistory.size() - 1; i >= 0; i--){
                for(auto i = 0; i < chatHistory.size(); ++i){
                    if(strcmp(chatHistory[i]->chatterSSID, admin_cookie.c_str()) == 0)
                        mg_websocket_printf(conn, WEBSOCKET_OPCODE_TEXT, "history %d Admin %s", 
                        chatHistory[i]->postTime, chatHistory[i]->message);
                    else
                        mg_websocket_printf(conn, WEBSOCKET_OPCODE_TEXT, "history %d %.9s %s", 
                            chatHistory[i]->postTime, chatHistory[i]->chatterSSID, chatHistory[i]->message);
                }
            }

            mg_websocket_printf(conn, WEBSOCKET_OPCODE_TEXT, "end history");

            return MG_FALSE;
        case MG_CLOSE:
            if (conn->connection_param)
                delete (struct chatData*)conn->connection_param;
            return MG_TRUE;
        case MG_AUTH:
            return MG_TRUE;
        default:
            return MG_FALSE;
    }
}

static int s_signal_received = 0;
static void signalHandler(int sig_num) {
  signal(sig_num, signalHandler);  // Reinstantiate signal handler
  s_signal_received = sig_num;
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

    int rc = unqlite_open(&pDb, zPath, UNQLITE_OPEN_CREATE + UNQLITE_OPEN_MMAP);
    if (rc != UNQLITE_OK) Fatal("Out of memory");

    logLog("Database: '%s'", zPath);

    unqlite_int64 nBytes = 4;
    cclong dummy = 0;

    if (unqlite_kv_fetch(pDb, "global_counter", -1, &dummy, &nBytes) != UNQLITE_OK)
        resetDatabase(pDb);

    //generate a random admin password
    admin_password = random_9chars();
    admin_cookie = admin_password;
    // strcpy(adminCookie, adminPassword);
    strcpy(banListPath, "ipbanlist");
    strcpy(thumbPrefix, "/images/");

    for (int i = 0; i < argc; ++i){
        // if(TEST_ARG("--reset",          "-r")) { logLog("Database: [New Profile]"); resetDatabase(pDb); }
        if(TEST_ARG("--set-counter",    "-C")) { writecclong(pDb, "global_counter", atoi(argv[++i]), true); }
        if(TEST_ARG("--title",          "-t")) { strncpy(siteTitle,     argv[++i], 64); continue; }
        if(TEST_ARG("--admin-spell",    "-a")) { admin_password = string(argv[++i]); continue; }
        if(TEST_ARG("--port",           "-p")) { strncpy(lport,         argv[++i], 64); continue; }
        if(TEST_ARG("--salt",           "-m")) { strncpy(md5Salt,       argv[++i], 64); continue; }
        // if(TEST_ARG("--admin-cookie",   "-A")) { strncpy(adminCookie,   argv[++i], 64); continue; }
        if(TEST_ARG("--ban-list",       "-b")) { strncpy(banListPath,   argv[++i], 64); continue; }
        if(TEST_ARG("--use-thumb",      "-S")) { strncpy(thumbPrefix,   argv[++i], 64); continue; }
        if(TEST_ARG("--tpp",            "-T")) { threads_per_page =  atoi(argv[++i]);     continue; }
        if(TEST_ARG("--cd-time",        "-c")) { postCDTime =      atoi(argv[++i]);     continue; }
        if(TEST_ARG("--max-image-size", "-I")) { maxFileSize =     atoi(argv[++i]);     continue; }
        if(TEST_ARG("--auto-lock",      "-L")) { max_replies =  atoi(argv[++i]);     continue; }
        if(TEST_ARG("--page-shown",     "-P")) { maxPageShown =    atoi(argv[++i]);     continue; }
        if(TEST_ARG("--stop-cookie",    "-s")) stopNewcookie = true;
        if(TEST_ARG("--stop-ipcheck",   "-i")) stop_check_ip = true;
        // if(TEST_ARG("--xff-ip",         "-x")) useXFF = true;
        if(TEST_ARG("--archive",        "-Z")) archiveMode = true;
    }

    // logLog("Site Title: '%s' -> Admin Password: '%s'", siteTitle, adminPassword);
    // logLog("Threads Per Page: %d", threads_per_page);
    // logLog("Cooldown Time: %ds", postCDTime);
    // logLog("MD5 Salt: '%s'", md5Salt);
    // logLog("Admin Cookie: '%s'", adminCookie);

    templates.add_template("single_thread");
    templates.add_template("expand_hidden_replies");
    templates.add_template("admin_console");
    templates.add_template("post_form");
    templates.add_template("site_header").var("SITE_TITLE", siteTitle);
    templates.add_template("site_footer").var("BUILD_DATE", to_string(BUILD_DATE));
    templates.add_template("site_slogan");
    templates.add_template("info_page");
    templates.add_template("admin_panel");
    templates.add_template("pager");
    templates.add_template("single_thread_header");

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

    server = mg_create_server(NULL, eventHandler);

    mg_set_option(server, "listening_port", lport);
    // mg_set_option(server, "document_root", "/");
    logLog("Listening on Port: '%s'", mg_get_option(server, "listening_port"));

    fflush(log_file);

    time(&gStartupTime);

    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
    while (s_signal_received == 0) {
        mg_poll_server(server, 1000);
    }

    mg_destroy_server(&server);
    unqlite_close(pDb);

    return 0;
}
