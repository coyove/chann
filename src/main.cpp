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

#include "general.h"

#include "helper.h"
#include "config.h"
#include "tags.h"

using namespace std;

ConfigManager configs;

unqlite *pDb;
time_t  gStartupTime;               //server's startup time
string  admin_password;              // * administrator's password
string  admin_cookie;
FILE*   log_file;                   //log file

unordered_set<string>   IDBanList;      //cookie ban list
unordered_set<string>   IPBanList;      //ip ban list
map<string, cclong>     IPAccessList;   //remote ip list
deque<struct History *> chatHistory;

struct mg_server *server;

TemplateManager templates;

#include "./views/single.h"
#include "./views/info.h"
#include "./views/success.h"
#include "./views/list/linear.h"
#include "./views/list/timeline.h"
#include "./views/list/gallery.h"
#include "./views/list/expand.h"

static const unsigned long long BUILD_DATE = __BUILD_DATE;
static int s_signal_received = 0;

bool checkIP(mg_connection* conn, bool verbose = false){
    // if(stop_check_ip) return true;
    if(!configs.global().get<bool>("security::access_control")) return true;

    time_t rawtime;
    time(&rawtime);
    const char * cip = cc_get_client_ip(conn);
    string sip(cip);
    time_t lasttime = IPAccessList[sip];
    
    // if(strstr(conn->uri, admin_password.c_str())) return true;
    
    if(is_admin(conn)) return true;

    if(IPBanList.find(sip) != IPBanList.end()) {
        // views::info::render(conn, STRING_YOUR_IP_IS_BANNED);
        views::info::render(conn, templates.invoke("misc").toggle("ip_is_banned").build_destory().c_str());
        logLog("Banned IP %s trying to access", cip);
        return false; //banned
    }

    if( abs(lasttime - rawtime) < configs.global().get<int>("user::cooldown") && lasttime != 0){
        views::info::render(conn, templates.invoke("misc").toggle("rapid_accessing").var("IP", cip).build_destory().c_str());
        if(verbose) logLog("%s is rapidly accessing", cip);
        return false;
    }
    IPAccessList[sip] = rawtime;
    return true;
}

// void views::each_thread(mg_connection* conn, struct Thread* r, char display_state, bool admin_view = false, 
//     const char* iid = "",
//     const char* uid = ""){
//     time_t now = time(NULL);

//     string thread_post_time = cc_timestamp_to_time(r->date);
//     int diff_day = cc_timestamp_diff_day(now, r->date);

//     char* content = readString(pDb, r->content);
//     string thread_content(content);
//     delete [] content;

//     if((display_state & SEND_CUT_LONG_COMMENT) && thread_content.size() > 1024) 
//         thread_content = thread_content.substr(0, 1024) + templates.invoke("misc").toggle("more_contents").build_destory();
//         //+ "<font color='red'><b>&#128065;" + STRING_MORE + "</b></font>";

//     HTMLTemplate *ht = templates.invoke_pointer("single_thread");

//     map<string, bool> stats;
//     map<string, string> vars;

//     stats["reply"]                      = display_state & SEND_IS_REPLY;
//     stats["show_reply"]                 = display_state & SEND_SHOW_REPLY_LINK;
//     stats["archive"]                    = configs.global().get<bool>("archive");
//     stats["normal_display"]             = r->state & NORMAL_DISPLAY || admin_view;
//     stats["thread_poster_is_admin"]     = (strcmp(r->ssid, "Admin") == 0);
//     stats["thread_poster_is_sameone"]   = (strcmp(r->ssid, iid) == 0);
//     stats["is_sameone"]                 = (strcmp(r->ssid, uid) == 0);
//     stats["sage"]                       = (r->state & SAGE_THREAD && !stats["reply"]);
//     stats["lock"]                       = (r->state & LOCKED_THREAD);
//     stats["delete"]                     = !(r->state & NORMAL_DISPLAY);
//     stats["show_admin"]                 = admin_view;
//     if (strlen(r->imgSrc) >= 4){
//         string fname(r->imgSrc);
//         vars["THREAD_IMAGE"]            = fname;

//         if (!cc_valid_image_ext(fname).empty()){
//             stats["image_attached"]     = true;
//             if(display_state & SEND_CUT_IMAGE){
//                 struct stat st;
//                 stat(("images/" + fname).c_str(), &st);
//                 stats["show_size_only"] = true;
//                 vars["THREAD_IMAGE_SIZE"] = to_string((int)(st.st_size / 1024));
//             }
//             else{
//                 stats["show_full_image"] = true;
//                 vars["THREAD_THUMB_PREFIX"] = configs.global().get("image::thumb_prefix");
//             }
//         }else{
//             stats["file_attached"]      = true;
//         }
//     }
//     if(r->childThread && !(display_state & SEND_CUT_REPLY_COUNT)){
//         struct Thread* c = readThread_(pDb, r->childThread);
//         stats["show_num_replies"]       = true;
//         vars["NUM_REPLIES"]             = to_string(c->childCount);
//         delete c;
//     }

//     vars["THREAD_POSTER"]               = string(r->ssid);
//     vars["THREAD_NO"]                   = to_string(r->threadID);
//     vars["THREAD_CONTENT"]              = thread_content;

//     // stats["show_title"]					= !stats["reply"] || strcmp(r->author, STRING_UNTITLED) != 0;
//     vars["THREAD_TITLE"]                = string(r->author);

//     vars["THREAD_IP"]                   = string(r->email);
//     vars["THREAD_POST_TIME"]            = thread_post_time;
//     vars["THREAD_STATE"]                = to_string(r->state);

//     if(diff_day >= 0 && diff_day <= 2){
//         stats["show_easy_date"]         = true;
//         vars["THREAD_POST_DATE"]        = to_string(diff_day);
//     }else{
//         char timetmp[64];
//         struct tm post_date;
//         localtime_r(&(r->date), &post_date);

//         strftime(timetmp, 64, "%Y-%m-%d", &post_date);
//         vars["THREAD_POST_DATE"]        = string(timetmp);
//     }

//     map<string, queue<string>> loops; //nothing

//     mg_printf_data(conn, "%s", ht->build(vars, stats, loops).c_str());

//     // if(content) delete [] content;
//     delete ht;
// }

// void showGallery(mg_connection* conn, cclong startID, cclong endID){
//     struct Thread *r = readThread_(pDb, 0); // get the root thread
//     cclong c = 0;

//     bool admin_view = is_admin(conn);

//     cclong totalThreads = r->childCount;
//     // cclong totalPages = 100;

//     templates.invoke("site_slogan").pipe_to(conn).destory();

//     for(cclong i = totalThreads; i > 0; --i){
//         delete r;
//         r = readThread_(pDb, i);
//         if(strlen(r->imgSrc) == 15 && r->state & NORMAL_DISPLAY) {
//             c++;
//             if (c >= startID && c <= endID){
//                 mg_printf_data(conn, "<hr>");
//                 views::each_thread(conn, r, SEND_SHOW_REPLY_LINK + SEND_CUT_LONG_COMMENT, admin_view);
//             }
//         }
        
//         if (c == endID + 1) break;
//     }
//     if(r) delete r;
// }

// void showThreads(mg_connection* conn, cclong startID, cclong endID){
//     struct Thread *r = readThread_(pDb, 0); // get the root thread
//     cclong c = 0;

//     bool admin_view = is_admin(conn);
//     string username = cck_verify_ssid(conn);

//     cclong totalThreads = r->childCount;

//     templates.invoke("site_slogan").pipe_to(conn).destory();

//     while (r->nextThread){
//         cclong tmpid = r->nextThread;
//         delete r;
//         r = readThread_(pDb, tmpid);
//         c++;
//         if (c >= startID && c <= endID){
//             mg_printf_data(conn, "<hr>");
//             //views::each_thread(conn, r, false, true, true, false, admin_view);
//             views::each_thread(conn, r, SEND_SHOW_REPLY_LINK + SEND_CUT_LONG_COMMENT + SEND_CUT_REPLY_COUNT, admin_view, "", username.c_str());
//             char *iid = new char[10];
//             strcpy(iid, r->ssid);

//             struct Thread *oldr = r;

//             if (r->childThread) {
//                 struct Thread* first_thread;
//                 first_thread = readThread_(pDb, r->childThread); // beginning of the circle
//                 r = first_thread;

//                 vector<struct Thread *> vt;
//                 // vector<struct Thread *> vt_next;

//                 int i = 1;
//                 while(i <= 4){
//                     r = readThread_(pDb, r->prevThread);

//                     if(r->threadID != first_thread->threadID) {
//                         vt.push_back(r);
//                         i++;
//                     }else
//                         break;
//                 }

//                 vt.push_back(first_thread);

//                 if(first_thread->childCount <= 5)
//                     for (int i = vt.size() - 1; i >= 0; i--){
//                         views::each_thread(conn, vt[i], SEND_IS_REPLY + SEND_CUT_LONG_COMMENT, admin_view, iid, username.c_str());
//                         delete vt[i];
//                     }
//                 else{
//                     for (int i = vt.size() - 1; i >= 0; i--){
//                         views::each_thread(conn, vt[i], SEND_IS_REPLY + SEND_CUT_LONG_COMMENT, admin_view, iid, username.c_str());
//                         delete vt[i];
//                         if(i == vt.size() - 1){
//                             templates.invoke("expand_hidden_replies") \
//                                 .var("THREAD_NO", oldr->threadID) \
//                                 .var("NUM_HIDDEN_REPLIES", first_thread->childCount - 5) \
//                                 .pipe_to(conn).destory();
//                         }
//                     }
//                 }

//             }

//             r = oldr;
//             delete [] iid;
//         }
        
//         if (c == endID + 1) break;
//     }
//     if(r) delete r;
// }

// void render_single_thread(mg_connection* conn, cclong id){
//     struct Thread *r = readThread_(pDb, id); // get the root thread
//     cclong pid = findParent(pDb, r);

//     // cout << pid;
//     if (pid == -1 && !is_admin(conn)){
//         views::info::render(conn, templates.invoke("misc").toggle("invalid_thread_no").build_destory().c_str());
//         delete r;
//         return;
//     }

//     templates.invoke("site_header").toggle("thread_page").toggle("is_admin", is_admin(conn)) \
//         .var("THREAD_NO", id).var("THREAD_TITLE", r->author).pipe_to(conn).destory();

//     templates.invoke("single_thread_header").toggle("homepage", !pid).var("THREAD_NO", pid) \
//         .toggle("thread", strstr(conn->uri, "/thread/")).pipe_to(conn).destory();

//     clock_t startc = clock();

//     bool reverse = strstr(conn->uri, "/daerht/");
//     bool admin_view = is_admin(conn);
//     string username = cck_verify_ssid(conn);

//     views::each_thread(conn, r, 0, admin_view);
//     mg_printf_data(conn, "<hr>");

//     char iid[10];
//     strcpy(iid, r->ssid);

//     if(!configs.global().get<bool>("archive"))
//         templates.invoke("post_form") \
//             .var("THREAD_NO", to_string(id)).toggle("reply_to_thread").toggle("is_admin", admin_view).pipe_to(conn).destory();
        
//     if (r->childThread) {
//         cclong r_childThread = r->childThread;
//         delete r;

//         int limit = configs.global().get<int>("user::collapse_image");

//         r = readThread_(pDb, r_childThread); // beginning of the circle
//         cclong rid = r->threadID; //the ID
//         bool too_many_replies = (r->childCount > limit);

//         int di = 1;

//         if(reverse){
//             cclong n_rid = r->prevThread;
//             delete r;
//             r = readThread_(pDb, n_rid);

//             views::each_thread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK, admin_view, iid, username.c_str());

//             while (r->prevThread != n_rid){
//                 di++;
//                 cclong r_prevThread = r->prevThread;

//                 delete r;
//                 r = readThread_(pDb, r_prevThread);

//                 if(too_many_replies && (di > limit))
//                     views::each_thread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK + SEND_CUT_IMAGE, admin_view, iid, username.c_str());
//                 else
//                     views::each_thread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK, admin_view, iid, username.c_str());
//             }
//         }else{
//             views::each_thread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK, admin_view, iid, username.c_str());

//             while (r->nextThread != rid){
//                 cclong r_nextThread = r->nextThread;
//                 delete r;

//                 r = readThread_(pDb, r_nextThread);
//                 di++;

//                 //views::each_thread(conn, r, true, true, false, too_many_replies && (di > 20), admin_view, iid);
//                 if(too_many_replies && (di > limit))
//                     views::each_thread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK + SEND_CUT_IMAGE, admin_view, iid, username.c_str());
//                 else
//                     views::each_thread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK, admin_view, iid, username.c_str());
//             }

//             if(r) delete r;
//         } 
//     }

//     clock_t endc = clock();

//     // site_footer(conn, (float)(endc-startc)/CLOCKS_PER_SEC);
//     templates.invoke("site_footer").var("TIME", (int)((endc-startc) * 1000 / CLOCKS_PER_SEC)).pipe_to(conn).destory();
// }

bool try_delete_thread(mg_connection* conn, cclong tid, bool admin = false){
    string username = cck_verify_ssid(conn);
    bool flag = false;

    if (!username.empty()){
        struct Thread* t = readThread_(pDb, tid);

        if (strcmp(t->ssid, username.c_str()) == 0 || admin){
            deleteThread(pDb, tid);
            // views::info::render(conn, STRING_DELETE_SUCCESS, tid);
            logLog("%s has deleted No.%d", username.c_str(), tid);
            flag = true;
        }
        else{
            // views::info::render(conn, templates.invoke("misc").toggle("try_to_usurp").build_destory().c_str());
            flag = false;
        }

        if(t) delete t;
    }else
        // views::info::render(conn, templates.invoke("misc").toggle("try_to_usurp").build_destory().c_str());
        flag = false;

    return flag;
}

// void views::linear::render(mg_connection* conn, 
//     bool admin_view = false, 
//     bool ip_based = false,
//     bool id_based = false,
//     const char* needle = ""){

//     string username = cck_verify_ssid(conn);

//     if (!username.empty()){
//         templates.invoke("site_header").toggle("my_post_page").toggle("is_admin", admin_view).pipe_to(conn).destory();

//         struct Thread *r = readThread_(pDb, 0); 
//         struct Thread *t;

//         clock_t startc = clock();

//         int limit = configs.global().get<int>("user::linear_threads");

//         for(cclong i = r->childCount; i > 0; i--){
//             t = readThread_(pDb, i);
//             if(r->childCount - i > limit && !admin_view) break; // only search the most recent 500 threads/replies

//             bool flag = false;

//             if(!ip_based && !id_based){ //list the threads only based on username, admin can see all
//                 if(admin_view) flag = true;
//                 if(strcmp(username.c_str(), t->ssid) == 0) flag = true;
//             }else if(ip_based){ // list the threads based on ip
//                 if(strcmp(t->email, needle) == 0) flag = true;
//             }else if(id_based){ // list the threads based on id
//                 if(strcmp(t->ssid, needle) == 0) flag = true;
//             }

//             if(flag){
//                 if(t->state & MAIN_THREAD)
//                     views::each_thread(conn, t, SEND_CUT_LONG_COMMENT + SEND_CUT_IMAGE, admin_view);
//                 else    
//                     views::each_thread(conn, t, SEND_IS_REPLY + SEND_CUT_LONG_COMMENT + SEND_CUT_IMAGE, admin_view);
//                 mg_printf_data(conn, "<hr>");
//             }

//             if(t) delete t;
//         }
//         clock_t endc = clock();
//         //site_footer(conn, (float)(endc-startc)/CLOCKS_PER_SEC);
//         templates.invoke("site_footer").var("TIME", (int)((endc-startc) * 1000 / CLOCKS_PER_SEC)).pipe_to(conn).destory();

//         if(r) delete r;
//     }else{
//         templates.invoke("site_header").toggle("my_post_page").pipe_to(conn).destory();
//         templates.invoke("site_footer").var("TIME", 0).pipe_to(conn).destory();
//     }
// }

void new_post(mg_connection* conn, const char* uri){
    char var1[64] = {'\0'}, var2[4096] = {'\0'}, var3[64] = {'\0'}, var4[16] = {'\0'};
    bool sage = false;
    bool fileAttached = false;
    const char *data;
    int data_len, ofs = 0, mofs = 0;
    char var_name[100], file_name[100];

    //first check the ip
    if(!checkIP(conn) || configs.global().get<bool>("archive")) return;
    bool admin_ctrl = is_admin(conn);
    
    string dump_name;
    if(configs.global().get<bool>("post::dump::raw")){
        dump_name = cc_random_chars(11);
        cc_write_binary((configs.global().get("post::dump::to") + dump_name).c_str(), conn->content, conn->content_len);
    }

    if(conn->content_len <= 0){
        views::info::render(conn, "Error");
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
        GET_VAR("input_email", 63, var3);
        GET_VAR("input_content", 4095, var2);

        if (strcmp(var_name, "input_file") == 0 && strcmp(file_name, "") != 0) {    //var4: the image attached (if has)
            int ilimit = configs.global().get<int>("image::max_size");
            if (data_len > 1024 * ilimit){
                views::info::render(conn, templates.invoke("misc").toggle("too_big_image").var("MAX", ilimit).build_destory().c_str());
                return;
            }

            // char fname[12];
            // unqlite_util_random_string(pDb, fname, 11);
            // fname[11] = 0;
            string original(file_name), new_name = cc_random_chars(11);
            std::transform(original.begin(), original.end(), original.begin(), ::tolower);

            string ext = cc_valid_image_ext(original);
            if(ext.empty()){
                if (admin_ctrl) 
                    ext = original.substr(original.size() - 4);
                else{
                    views::info::render(conn, templates.invoke("misc").toggle("invalid_image").build_destory().c_str());
                    return;
                }
            }

            fileAttached = true;
            string fpath = "images/" + new_name + ext;

            cc_write_binary(fpath.c_str(), data, data_len);

            strcpy(var4, (new_name + ext).c_str());
            logLog("Image uploaded: %s", var4);
        }
    }

    #undef GET_VAR

    //see if there is a SPECIAL string in the text field
    if (strcmp(var1, "") == 0) strcpy(var1, "untitled");
    //user trying to sega a thread/reply
    if (strstr(var3, "sage") && configs.global().get<bool>("post::allow_self::sage")) sage = true;
    //user trying to delete a thread/reply
    if (strstr(var3, "delete") && configs.global().get<bool>("post::allow_self::delete")){
        cclong id = cc_extract_uri_num(conn);
        // struct Thread * t = readThread_(pDb, id); //what he replies to is which he wants to delete
        if(try_delete_thread(conn, id, admin_ctrl))
            views::info::render(conn, templates.invoke("misc").toggle("delete_okay").var("THREAD_NO", id).build_destory().c_str());
        else
            views::info::render(conn, templates.invoke("misc").toggle("try_to_usurp").build_destory().c_str());
        return;
    }
    if (strstr(var3, "url")){
        templates.invoke("site_header").toggle("upload_image_page").pipe_to(conn).destory();
        templates.invoke("views::info::render").toggle("upload_image_page").var("IMAGE", var4).pipe_to(conn).destory();
        templates.invoke("site_footer").var("TIME", 0).pipe_to(conn).destory();

        logLog("Image uploaded and won't be posted");
        return;
    }
    //admin trying to update a thread/reply
    if (strstr(var3, "update") && admin_ctrl){
        cclong id = cc_extract_uri_num(conn);
        struct Thread * t = readThread_(pDb, id); 
        //what admin replies to is which he wants to update
        //note the server doesn't filter the special chars such as "<" and ">"
        //so it's possible for admin to use HTML here
        if(!strstr(var3, "html")){
            string up_str = cc_replace(string(var2), string("\n"), string("<br>"));
            writeString(pDb, t->content, up_str.c_str(), true); 
        }else
            writeString(pDb, t->content, var2, true); 

        views::info::render(conn, "Thread updated successfully");
        logLog("Admin has edited No.%d", t->threadID);
        return;
    }
    
    // char ipath[64]; FILE *fp; struct stat st;
    // sprintf(ipath, "images/%s", var3);
    
        //image or comment or both
    if (strcmp(var2, "") == 0 && !fileAttached) {
        views::info::render(conn, templates.invoke("misc").toggle("cannot_post_null").build_destory().c_str());
        return;
    }else if (strcmp(var2, "") == 0 && fileAttached){
        // strcpy(var2, STRING_UNCOMMENTED);
        strcpy(var2, templates.invoke("misc").toggle("null_comment").build_destory().c_str());
    }
    

    string username = cck_verify_ssid(conn);
    string ssid = cck_extract_ssid(conn);
    // mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, sizeof(ssid));

    if (!username.empty()){  //passed
        string findID = username;
        // if(strcmp(ssid, adminCookie) != 0)
        if (IDBanList.find(findID) != IDBanList.end()){
            //this id is banned, so we destory it
            cck_destory_ssid(conn);
            logLog("Banned cookie destoryed: '%s'", ssid.c_str());
            views::info::render(conn, templates.invoke("misc").toggle("id_is_banned").build_destory().c_str());
            return;
        }
    }else{
        if (configs.global().get<bool>("cookie::stop")){
            views::info::render(conn, templates.invoke("misc").toggle("cookie_is_stopped").build_destory().c_str());
            return;
        }
        else{
            username = cc_random_username();
            ssid = cck_create_ssid(username);
        }
    }

    if (admin_ctrl) username = "Admin";

    //replace some important things
    string tmpcontent(var2);    cc_clean_string(tmpcontent);
    string tmpname(var1);       cc_clean_string(tmpname);

    strncpy(var1, tmpname.c_str(), 64);

    vector<string> imageDetector = cc_split(tmpcontent, "\n");
    tmpcontent = "";

    for (auto i = 0; i < imageDetector.size(); ++i){
        if (startsWith(imageDetector[i], "http"))
            imageDetector[i] = "<a href='" + imageDetector[i] + "'>" + imageDetector[i] + "</a>";
        bool refFlag = false;
        if (startsWith(imageDetector[i], "&gt;&gt;No.")){
            vector<string> gotoLink = cc_split(imageDetector[i], string("."));
            if (gotoLink.size() == 2){
                imageDetector[i] = "<div class='div-thread-" + gotoLink[1] + "'><a href='javascript:ajst(" + gotoLink[1] + ")'>" + imageDetector[i] + "</a></div>";
                refFlag = true;
            }
        }
        if (startsWith(imageDetector[i], "&gt;"))
            imageDetector[i] = "<ttt>" + imageDetector[i] + "</ttt>";

        tmpcontent.append(imageDetector[i] + ((!refFlag) ? "<br/>" : ""));
    }

    const char * cip = cc_get_client_ip(conn);
    logLog("New thread: (Sub: '%s', Opt: '%s', Img: '%s', Dump: '%s', IP: '%s')", var1, var3, var4, dump_name.c_str(), cip);

    if (strstr(conn->uri, "/post_reply/")) {
        cclong id = cc_extract_uri_num(conn);
        struct Thread* t = readThread_(pDb, id);

        if(t->state & TOOMANY_REPLIES){
            views::info::render(conn, templates.invoke("misc").toggle("cannot_reply_toomany") \
                    .var("MAX", configs.global().get<int>("user::max_replies")).build_destory().c_str());
            delete t;
            return;
        }

        if(t->childThread){
            struct Thread* tc = readThread_(pDb, t->childThread);
            if(tc->childCount >= configs.global().get<int>("user::max_replies") - 1){
                changeState(t, TOOMANY_REPLIES, true);
                writeThread(pDb, t->threadID, t, true);
            }
            delete tc;            
        }

        if(t->state & LOCKED_THREAD)
            views::info::render(conn, templates.invoke("misc").toggle("cannot_reply_locked").build_destory().c_str());
        else{
            newReply(pDb, id, tmpcontent.c_str(), var1, cip, username.c_str(), var4, sage);
            
            mg_printf(conn, "HTTP/1.1 302 Moved Temporarily\r\nLocation: /success/%s/%d\r\n\r\n", ssid.c_str(), id);
        }

        if(t) delete t;
    }
    else{
        newThread(pDb, tmpcontent.c_str(), var1, cip, username.c_str(), var4, sage);
        //views::info::render(conn, "Successfully start a new thread");
        mg_printf(conn, "HTTP/1.1 302 Moved Temporarily\r\nLocation: /success/%s/0\r\n\r\n", ssid.c_str());
    }
}

// void renew_cookie(mg_connection* conn){
//     string url(conn->uri);
//     vector<string> tmp = cc_split(url, "/");

//     if(tmp.size() != 3){
//         views::info::render(conn, "Error");
//         return;
//     }
//     cclong id  = atol(tmp[tmp.size() - 1].c_str());

//     cck_send_ssid(conn, tmp[tmp.size() - 2]);
//     mg_send_header(conn, "charset", "utf-8");
//     mg_send_header(conn, "Content-Type", "text/html");
//     mg_send_header(conn, "cache-control", "private, max-age=0");
    
//     // if(id == 0)
//     //     views::info::render(conn, STRING_NEW_THREAD_SUCCESS);
//     // else{
//     templates.invoke("site_header").toggle("success_post_page").pipe_to(conn).destory();
//     templates.invoke("views::info::render").var("THREAD_NO", id).toggle("return_page").pipe_to(conn).destory();
//     templates.invoke("site_footer").var("TIME", 0).pipe_to(conn).destory();
//     // }   //post successfully page
// }

// void render_page(mg_connection* conn, bool indexPage, bool galleryPage = false){

//     int num = indexPage ? 1 : cc_extract_uri_num(conn);

//     templates.invoke("site_header").var("CURRENT_PAGE", num).toggle("is_admin", is_admin(conn)) \  
//         .toggle(!galleryPage ? "timeline_page" : "gallery_page").pipe_to(conn).destory();

//     if(!configs.global().get<bool>("archive")) 
//         templates.invoke("post_form").toggle("is_admin", is_admin(conn)).toggle("post_new_thread").pipe_to(conn).destory();

//     clock_t startc = clock();
//     int max_page_viewable = configs.global().get<int>("user::viewable_pages");
//     int threads_per_page = configs.global().get<int>("user::threads_per_page");

//     bool admin_view = is_admin(conn);
//     int start_no = 1, end_no = threads_per_page;

//     if(indexPage)
//         showThreads(conn, start_no, end_no);
//     else{
//         cclong cp = cc_extract_uri_num(conn);
//         if(cp <= max_page_viewable || max_page_viewable == 0 || admin_view){
//             end_no = cp * threads_per_page;
//             start_no = end_no- threads_per_page + 1;

//             if(galleryPage)
//                 showGallery(conn, start_no, end_no);
//             else
//                 showThreads(conn, start_no, end_no);
//         }
//     }

//     cclong current_page = end_no / threads_per_page;
//     queue<string> before, after;
//     for (cclong i = 1; i <= (max_page_viewable == 0 || admin_view ? 10000 : max_page_viewable); ++i){
//         if (current_page - i < 4 && current_page - i > 0) before.push(to_string(i));
//         else if (i - current_page < 4 && i - current_page > 0) after.push(to_string(i));
//         else if (i - current_page > 4) break;
//     }
    
//     templates.invoke("pager").toggle(galleryPage ? "gallery_page" : "timeline_page") \
//         .var("CURRENT_PAGE", current_page) \
//         .var("PREVIOUS_PAGE", current_page > 1 ? (current_page - 1) : 1) \
//         .var("NEXT_PAGE", current_page + 1) \
//         .loop("before_pages", before).loop("after_pages", after).pipe_to(conn).destory();

//     clock_t endc = clock();
//     templates.invoke("site_footer").var("TIME", (int)((endc-startc) * 1000 / CLOCKS_PER_SEC)).pipe_to(conn).destory();
// }

void admin_actions(struct mg_connection *conn){
    if(conn->content_len <= 0){
        views::info::render(conn, "Error");
        return;
    }
    char action_name[32] = {0}, action_param1[32] = {0}, action_param2[32] = {0};
    string action, param1, param2;

    mg_get_var(conn, "action_name", action_name, sizeof(action_name)); action = string(action_name);
    mg_get_var(conn, "action_1", action_param1, sizeof(action_param1)); param1 = string(action_param1);
    mg_get_var(conn, "action_2", action_param2, sizeof(action_param2)); param2 = string(action_param2);

    if(action == ""){
        views::info::render(conn, "Error");
        return;
    }
    
    #define HAS_KEY(x) x == action

    string ret = "Action completed.";

    // if(HAS_KEY("login") && param1 == admin_password){
    if(HAS_KEY("login") && param1 == configs.global().get("security::admin::password")){
        string ssid = cck_extract_ssid(conn);

        string new_name = cc_random_username();
        ssid = cck_create_ssid(new_name);
        cck_send_admin_ssid(conn, ssid);

        // strncpy(adminCookie, ssid, 64);
        admin_cookie = ssid;
        logLog("Admin %s has logined", cc_get_client_ip(conn));
    }

    if(!is_admin(conn)) {
        views::info::render(conn, templates.invoke("misc").toggle("try_to_usurp").build_destory().c_str());
        checkIP(conn, true);
        return;
    }

    // if(HAS_KEY("cookie"))               stopNewcookie = (param1 == "off");
    // if(HAS_KEY("archive"))              archiveMode = (param1 == "on");
    // if(HAS_KEY("acl"))                  stop_check_ip = (param1 == "off");
    // if(HAS_KEY("max-page-viewable"))    max_page_viewable = atol(param1.c_str());
    // if(HAS_KEY("cd-time"))              cooldown_time = atol(param1.c_str());
    // if(HAS_KEY("threads-per-page"))     threads_per_page = atol(param1.c_str());
    // if(HAS_KEY("max-replies"))          max_replies = atol(param1.c_str());
    if(HAS_KEY("quit-admin"))           {admin_cookie = cc_random_username(); ret = "quitted";}
    // if(HAS_KEY("new-password"))         admin_password = param1;
    if(HAS_KEY("delete-thread"))        try_delete_thread(conn, atol(param1.c_str()), true);
    // if(HAS_KEY("max-image-size"))       max_image_size = atol(param1.c_str());
    if(HAS_KEY("update")){
    	// cout << param1 << "," << param2 << endl;
    	configs.global().try_set(param1, param2);
    }
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

        // std::ofstream f(ban_list_file);
        // for(auto i = IPBanList.begin(); i != IPBanList.end(); ++i) f << *i << '\n';
        // for(auto j = IDBanList.begin(); j != IDBanList.end(); ++j) f << "ID:" << *j << '\n';
        // f.close();
        cc_store_set_to_file(configs.global().get("file::ban::ip_based"), IPBanList);
    	cc_store_set_to_file(configs.global().get("file::ban::id_based"), IDBanList);
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

static void response(struct mg_connection *conn) {

    if (strcmp(conn->uri, "/post_thread") == 0) {
        new_post(conn, conn->uri); 
    }
    else if (strstr(conn->uri, "/post_reply/")) {
        new_post(conn, conn->uri); 
    }
    else if (strstr(conn->uri, "/page/")) {
        views::timeline::render(conn, false);
    }
    else if (strstr(conn->uri, "/gallery/")) {
        views::gallery::render(conn);
    }
    else if (strstr(conn->uri, "/success/")) {
        views::success::render(conn);
    }
    else if (strstr(conn->uri, "/thread/") || strstr(conn->uri, "/daerht/")){
        views::expand::render(conn);
    }
    else if (strstr(conn->uri, "/api/")){
        cclong id = cc_extract_uri_num(conn);
        struct Thread *r = readThread_(pDb, id); 
        bool admin_view = is_admin(conn);

        views::each_thread(conn, r, 0, admin_view);

        delete r;
    }
    else if (strstr(conn->uri, "/list")){
        if(!checkIP(conn, true)) return;

        string url(conn->uri);
        vector<string> tmp = cc_split(url, "/");
        string id = tmp[tmp.size() - 1];

        if (is_admin(conn)){
            if (strcmp(conn->uri, "/list") == 0)
                views::linear::render(conn, true);
            else if(strstr(conn->uri, "/ip/"))
                views::linear::render(conn, true, true, false, id.c_str());
            else 
                views::linear::render(conn, true, false, true, id.c_str());
        }
        else{
            views::linear::render(conn);          
        }       //list ID/IP, kill all posted by ID/IP
    }
    else if (strstr(conn->uri, "/del/")){
        int id = cc_extract_uri_num(conn);
        if(try_delete_thread(conn, id))
            views::info::render(conn, templates.invoke("misc").toggle("delete_okay").var("THREAD_NO", id).build_destory().c_str());
        else
            views::info::render(conn, templates.invoke("misc").toggle("try_to_usurp").build_destory().c_str());
        return;
    }
    else if (strstr(conn->uri, "/images/")){
        cc_serve_image_file(conn);
    }
    else if (strcmp(conn->uri, "/admin") == 0){
        string ip_ban_list = "";
        string id_ban_list = "";
        for(auto i = IPBanList.begin(); i != IPBanList.end(); ++i) ip_ban_list += (*i + ",");
        for(auto j = IDBanList.begin(); j != IDBanList.end(); ++j) id_ban_list += (*j + ",");

        time_t c; time(&c);
        unsigned long dummy, res = 0;

        FILE *f = fopen("/proc/self/statm", "r");
        if(f){
            fscanf(f,"%ld %ld %ld %ld %ld %ld %ld", &dummy, &res, &dummy, &dummy, &dummy, &dummy, &dummy);
            fclose(f);
        }

        templates.invoke("site_header").toggle("admin_panel_page").pipe_to(conn).destory();
        templates.invoke("admin_panel") \
            .toggle("is_admin", is_admin(conn)) \
            .var("RUNNING_TIME", (int)((c - gStartupTime) / 3600)) \
            .var("MEMORY_USAGE", res * 4) \
            .var("IP_BAN_LIST", ip_ban_list) \
            .var("ID_BAN_LIST", id_ban_list) \
            .var("GLOBAL_CONFIGS", configs.global().serialize_to_json()) \
            .pipe_to(conn).destory();
        templates.invoke("site_footer").var("TIME", 0).pipe_to(conn).destory();
    }
    else if (strcmp(conn->uri, "/admin_action") == 0){
        admin_actions(conn);
    }
    else 
        views::timeline::render(conn, true);

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
    while(chatHistory.size() > configs.global().get<int>("chat::max_histories")){
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

    string username = cck_verify_ssid(d->chatterSSID);
    if(username.empty()) return;

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

static int event_handle(struct mg_connection *conn, enum mg_event ev) {
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
                response(conn);
                return MG_TRUE;
            }
        case MG_WS_CONNECT:
            username = cck_verify_ssid(conn);
            ssid = cck_extract_ssid(conn);

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

static void signal_handle(int sig_num) {
  signal(sig_num, signal_handle);  // Reinstantiate signal handler
  s_signal_received = sig_num;
}

int main(int argc, char *argv[]){
    string conf_path = "./cchan.conf";

    for (int i = 0; i < argc; ++i){
        if((strcmp(argv[i], "--load") == 0 || strcmp(argv[i], "-l") == 0)){
            conf_path = string(argv[++i]);
            continue;
        }      
    }

    configs.global().load(conf_path);
    string db_path = configs.global().get("database"); 
    log_file = stdout;

    int rc = unqlite_open(&pDb, db_path.c_str(), UNQLITE_OPEN_CREATE + UNQLITE_OPEN_MMAP);
    if (rc != UNQLITE_OK) database_fatal("Out of memory");

    logLog("Database '%s' loaded", db_path.c_str());

    unqlite_int64 nBytes = 4;
    cclong dummy = 0;

    if (unqlite_kv_fetch(pDb, "global_counter", -1, &dummy, &nBytes) != UNQLITE_OK)
        resetDatabase(pDb);

    //generate a random admin password
    admin_password  = cc_random_chars(10);
    admin_cookie    = admin_password;
    configs.global().set("security::admin::cookie", admin_password);

    templates.add_template("single_thread");
    templates.add_template("expand_hidden_replies");
    templates.add_template("post_form");
    templates.add_template("site_header").var("SITE_TITLE", configs.global().get("title"));
    templates.add_template("site_footer").var("BUILD_DATE", to_string(BUILD_DATE));
    templates.add_template("site_slogan");
    templates.add_template("info_page");
    templates.add_template("admin_panel");
    templates.add_template("pager");
    templates.add_template("misc");
    templates.add_template("single_thread_header");

    cc_load_file_to_set(configs.global().get("file::ban::ip_based"), IPBanList);
    cc_load_file_to_set(configs.global().get("file::ban::id_based"), IDBanList);
    logLog("Banlist loaded with %d/%d records", IPBanList.size(), IDBanList.size());    

    server = mg_create_server(NULL, event_handle);
    mg_set_option(server, "listening_port", to_string(configs.global().get<int>("listen")).c_str());
    logLog("Start listening on port %s", mg_get_option(server, "listening_port"));

    fflush(log_file);

    time(&gStartupTime);

    signal(SIGTERM, signal_handle);
    signal(SIGINT, signal_handle);
    while (s_signal_received == 0) {
        mg_poll_server(server, 1000);
    }

    mg_destroy_server(&server);
    unqlite_close(pDb);

    return 0;
}
