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
#include "router.h"

using namespace std;

ConfigManager configs;

unqlite *pDb;
time_t  gStartupTime;                   // server's startup time
string  admin_password;                 // administrator's password
string  admin_cookie;
FILE*   log_file;                       // log file

unordered_set<string>   IDBanList;      // cookie ban list
unordered_set<string>   IPBanList;      // ip ban list
map<string, int>     IPAccessList;      // remote ip list

struct mg_server *server;

TemplateManager templates;

#include "./views/single.h"
#include "./views/info.h"
#include "./views/success.h"
#include "./views/list/linear.h"
#include "./views/list/timeline.h"
#include "./views/list/gallery.h"
#include "./views/list/expand.h"

#include "./actions/chat.h"
#include "./actions/post.h"
#include "./actions/kill.h"

#ifdef GOOGLE_RECAPTCHA
#include <curl/curl.h>
#endif

static const unsigned long long BUILD_DATE = __BUILD_DATE;
static int s_signal_received = 0;

bool check_ip(mg_connection* conn, bool verbose = false){
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

namespace actions{
    namespace admin{
        void call(struct mg_connection *conn){
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
                check_ip(conn, true);
                return;
            }

            if(HAS_KEY("quit-admin"))           {admin_cookie = cc_random_username(); ret = "quitted";}
            // if(HAS_KEY("new-password"))         admin_password = param1;
            if(HAS_KEY("delete-thread"))        actions::kill::call(conn, atol(param1.c_str()), true);
            // if(HAS_KEY("max-image-size"))       max_image_size = atol(param1.c_str());
            if(HAS_KEY("update")){
            	// cout << param1 << "," << param2 << endl;
            	configs.global().try_set(param1, param2);
            }
            if(HAS_KEY("new-state") && !param1.empty()){
                int id = atol(param1.c_str());

                struct Thread * t = unq_read_thread(pDb, id);
                t->state = atol(param2.c_str());
                unq_write_thread(pDb, t->threadID, t, true);

                ret += (" New state: " + unq_resolve_state(t->state));
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

                cc_store_set_to_file(configs.global().get("file::ban::ip_based"), IPBanList);
            	cc_store_set_to_file(configs.global().get("file::ban::id_based"), IDBanList);
            }
            if(HAS_KEY("hide-image")){
                struct Thread * t = unq_read_thread(pDb, atol(param1.c_str()));
                
                string oldname = "images/" + string(t->imgSrc);
                string newname = oldname + ".tmp";
                rename(oldname.c_str(), newname.c_str());

                delete t;
            }
            if(HAS_KEY("kill-all")){
                string id = param1;

                struct Thread *r = unq_read_thread(pDb, 0); 
                struct Thread *t;
                bool ipflag = (param2 == "ip");

                clock_t startc = clock();
                for(cclong i = r->childCount; i > 0; i--){
                    t = unq_read_thread(pDb, i);
                    if(ipflag){
                        if(strcmp(id.c_str(), t->email) == 0) unq_delete_thread(pDb, i);
                    }else{
                        if(strcmp(id.c_str(), t->ssid) == 0) unq_delete_thread(pDb, i);
                    }

                    if(t) delete t;
                }
                clock_t endc = clock();

                if(r) delete r;
                ret += (" Take " + to_string((float)(endc-startc)/CLOCKS_PER_SEC));
            }

            if(HAS_KEY("search")){
                struct Thread *t, *r = unq_read_thread(pDb, 0); 
                int c = 0, limit = atol(param2.c_str());
                ret = "[";

                for(cclong i = r->childCount; i > 0; i--){
                    if(++c == limit) break;
                    t = unq_read_thread(pDb, i);
                    const char* content = unq_read_string(pDb, t->content);
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
    }
}

namespace views{
    namespace admin{
        void render(mg_connection* conn){
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
    }
}

static int event_handle(struct mg_connection *conn, enum mg_event ev) {
    switch(ev){
        case MG_REQUEST:
            if (conn->is_websocket) {
                websocket::poll(conn);
                return MG_TRUE;
            } 
            else if (strstr(conn->uri, "/assets/")){
                mg_send_file(conn, conn->uri + 1, "");
                return MG_MORE;
            }
            else{
                pair<int, map<string, string>> r = route::route(conn->uri);
                // for(auto i = r.second.begin(); i != r.second.end(); ++i) cout << i->first << "=" << i ->second << endl;
                
                int id;
                struct Thread *tt;

                switch(r.first){
                    case route::POST_THREAD:
                    case route::POST_REPLY:
                        if(!check_ip(conn) || configs.global().get<bool>("archive")) break;
                        actions::post::call(conn, r.first == route::POST_THREAD ? 0 : atol(r.second["no"].c_str()));
                        break;
                    case route::SUCCESS:
                        views::success::render(conn, r.second["cookie"], atol(r.second["no"].c_str()));
                        break;
                    case route::THREAD:
                    case route::DAERHT:
                        id = atol(r.second["no"].c_str());
                        views::expand::render(conn, id, r.first == route::DAERHT);
                        break;
                    case route::TIMELINE:
                        views::timeline::render(conn, false);
                        break;
                    case route::GALLERY:
                        views::gallery::render(conn);
                        break;
                    case route::LINEAR:
                        if(!check_ip(conn, true)) break;
                        views::linear::render(conn);
                        break;
                    case route::LINEAR_IP:
                        views::linear::render(conn, true, false, r.second["ip"].c_str());
                        break;
                    case route::LINEAR_ID:
                        views::linear::render(conn, false, true, r.second["id"].c_str());
                        break;
                    case route::ADMIN_PANEL:
                        views::admin::render(conn);
                        break;
                    case route::SELF_DELETE:
                        id = atol(r.second["no"].c_str());
                        if(actions::kill::call(conn, id))
                            views::info::render(conn, templates.invoke("misc").toggle("delete_okay").var("THREAD_NO", id).build_destory().c_str());
                        else
                            views::info::render(conn, templates.invoke("misc").toggle("try_to_usurp").build_destory().c_str());
                        break;
                    case route::API_THREAD:
                        tt = unq_read_thread(pDb, atol( r.second["no"].c_str() ));
                        views::each_thread(conn, tt, 0, is_admin(conn));
                        delete tt;
                        break;
                    case route::IMAGE:
                        cc_serve_image_file(conn);
                        break;
                    case route::ADMIN_ACTION:
                        actions::admin::call(conn);
                        break;
                    default:
                        views::timeline::render(conn, true);
                }

                fflush(log_file); // flush the buffer
                return MG_TRUE;
            }
        case MG_WS_CONNECT:
            websocket::connect(conn);
            return MG_FALSE;
        case MG_CLOSE:
            websocket::disconnect(conn);
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
    string conf_path = "./chann.conf";
    log_file = stdout;

    for (int i = 0; i < argc; ++i){
        if((strcmp(argv[i], "--load") == 0 || strcmp(argv[i], "-l") == 0)){
            conf_path = string(argv[++i]);
            continue;
        }      
    }

    configs.global().load(conf_path);
    logLog("Load configuration: %s", conf_path.c_str());

    string db_path = configs.global().get("database"); 

    int rc = unqlite_open(&pDb, db_path.c_str(), UNQLITE_OPEN_CREATE + UNQLITE_OPEN_MMAP);
    if (rc != UNQLITE_OK) database_fatal("Out of memory");
    logLog("Load database: %s", db_path.c_str());

    unqlite_int64 nBytes = 4;
    int dummy = 0;

    if (unqlite_kv_fetch(pDb, "global_counter", -1, &dummy, &nBytes) != UNQLITE_OK) unq_reset(pDb);

    //generate a random admin password
    admin_password  = cc_random_chars(10);
    admin_cookie    = admin_password;
    configs.global().set("security::admin::cookie", admin_password);

    templates.use_lang(configs.global().get("lang"));
    logLog("Load %d templates", templates.load_templates());

    templates.add_template("site_header").var("SITE_TITLE", configs.global().get("title"));
    templates.add_template("site_footer").var("BUILD_DATE", to_string(BUILD_DATE));

    bool use_captcha = configs.global().get<bool>("security::captcha");
    #ifndef GOOGLE_RECAPTCHA
    use_captcha = false;
    #endif

    templates.add_template("post_form").toggle("use_captcha", use_captcha) \
        .var("LANG", configs.global().get("lang")).var("PUBLIC", configs.global().get("security::captcha::public_key"));

    cc_load_file_to_set(configs.global().get("file::ban::ip_based"), IPBanList);
    cc_load_file_to_set(configs.global().get("file::ban::id_based"), IDBanList);
    logLog("Load banlist with %d/%d records", IPBanList.size(), IDBanList.size());    

    server = mg_create_server(NULL, event_handle);
    mg_set_option(server, "listening_port", to_string(configs.global().get<int>("listen")).c_str());
    logLog("Start listening on port %s", mg_get_option(server, "listening_port"));

    fflush(log_file);

    time(&gStartupTime);

    signal(SIGTERM, signal_handle);
    signal(SIGINT, signal_handle);

    route::init();
    route::add("/thread/:no",               route::THREAD);
    route::add("/daerht/:no",               route::DAERHT);
    route::add("/page/:page",               route::TIMELINE);
    route::add("/gallery/:page",            route::GALLERY);
    route::add("/list",                     route::LINEAR);
    route::add("/list/ip/:ip",              route::LINEAR_IP);
    route::add("/list/id/:id",              route::LINEAR_ID);
    route::add("/admin",                    route::ADMIN_PANEL);
    route::add("/del/:no",                  route::SELF_DELETE);
    route::add("/api/:no",                  route::API_THREAD);
    route::add("/images/:path",             route::IMAGE);
    route::add("/success/:cookie/to/:no",   route::SUCCESS);
    route::add("/admin_action",             route::ADMIN_ACTION);
    route::add("/post_thread",              route::POST_THREAD);
    route::add("/post_reply/:no",           route::POST_REPLY);

    #ifdef GOOGLE_RECAPTCHA
    curl_global_init(CURL_GLOBAL_ALL);
    #endif

    while (s_signal_received == 0) {
        mg_poll_server(server, 1000);
    }

    #ifdef GOOGLE_RECAPTCHA
    curl_global_cleanup();
    #endif

    mg_destroy_server(&server);
    unqlite_close(pDb);

    return 0;
}
