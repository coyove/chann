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
#include <sys/stat.h>
#include <signal.h>

extern "C" {
#include "../lib/mongoose/mongoose.h"
}

// include "general.h"
#include "helper.h"
#include "config.h"
#include "tags.h"
#include "router.h"
#include "data.h"

using namespace std;

ConfigManager configs;
LevelDB ldb;

time_t gStartupTime;   // server's startup time
string admin_password; // administrator's password

string admin_cookie;
map<string, string> assist_cookie;

FILE *log_file;                  // log file

unordered_set<string> IDBanList; // cookie ban list
unordered_set<string> IPBanList; // ip ban list
map<string, int> IPAccessList;   // remote ip list

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
#endif // ifdef GOOGLE_RECAPTCHA

static const unsigned long long BUILD_DATE = __BUILD_DATE;
static int s_signal_received               = 0;

bool check_ip(mg_connection *conn, bool verbose = false) {
  if (!configs.global().get<bool>("security::access_control")) return true;

  time_t rawtime;
  time(&rawtime);
  const char *cip = cc_get_client_ip(conn);
  string sip(cip);
  time_t lasttime = IPAccessList[sip];

  if (is_admin(conn)) return true;

  if (!is_assist(conn).empty()) return true;

  if (IPBanList.find(sip) != IPBanList.end()) {
    views::info::render(conn, templates.invoke("misc").toggle(
                          "ip_is_banned").build_destory().c_str());
    logLog("Banned IP %s trying to access", cip);
    return false; // banned
  }

  if ((abs(lasttime - rawtime) < configs.global().get<int>("user::cooldown")) &&
      (lasttime != 0)) {
    views::info::render(conn, templates.invoke("misc").toggle(
                          "rapid_accessing").var("IP",
                                                 cip).build_destory().c_str());

    if (verbose) logLog("%s is rapidly accessing", cip);
    return false;
  }
  IPAccessList[sip] = rawtime;
  return true;
}

namespace actions {
namespace admin {
void call(struct mg_connection *conn) {
  if (conn->content_len <= 0) {
    views::info::render(conn, "Error");
    return;
  }
  char action_name[32]   = { 0 }, action_param1[32] = { 0 },
       action_param2[32] = { 0 };
  string action, param1, param2;

  mg_get_var(conn, "action_name", action_name,   sizeof(action_name));
  action = string(action_name);
  mg_get_var(conn, "action_1",    action_param1, sizeof(action_param1));
  param1 = string(action_param1);
  mg_get_var(conn, "action_2",    action_param2, sizeof(action_param2));
  param2 = string(action_param2);

  if (action == "") {
    views::info::render(conn, "Error");
    return;
  }

  string ret = "Action completed.";

  if (action == "login") {
    string new_name = cc_random_username();
    string ssid     = cck_create_ssid(new_name);

    if ((param1 == configs.global().get("security::admin::username")) &&
        (param2 ==
         configs.global().get("security::admin::password")))
    {
      cck_send_admin_ssid(conn, ssid);
      admin_cookie = ssid;
      logLog("Admin (%s) has logined", cc_get_client_ip(conn));
    } else {
      string pwd = configs.global().get(
        "security::assist::" + param1 + "::password");

      if (!pwd.empty() && (param2 == pwd)) {
        cck_send_admin_ssid(conn, ssid, "assist");
        assist_cookie[param1] = ssid;
        logLog("Assist %s (%s) has logined", param1.c_str(), cc_get_client_ip(
                 conn));
      }
    } // finish verifying identity
  } // login

  string aid = "";

  if (is_admin(conn)) {
    aid = "admin";
  } else {
    aid = is_assist(conn);

    if (aid.empty()) { // username and password are wrong
      views::info::render(conn, templates.invoke("misc").toggle(
                            "try_to_usurp").build_destory().c_str());
      check_ip(conn, true);
      return;
    }
  }

#define ASSIST(x) configs.global().get<bool>( \
    "security::assist::" + aid + "::" + x)

  if (action == "quit-admin") {
    admin_cookie = cc_random_username();
    ret          = "admin quitted.";
  } // quit-admin

  if (action == "quit-assist") {
    assist_cookie[aid] = cc_random_username();
    ret                = aid + " quitted.";
  } // quit-assist

  if (action == "delete-thread") {
    if ((aid == "admin") || ASSIST("delete")) {
      actions::kill::call(conn, std::stol(param1), true);
    } else {
      ret = "Access denied.";
    }
  } // delete-thread

  if (action == "update" && (aid == "admin")) {
    configs.global().try_set(param1, param2);
  }

  if (action == "new-state" && !param1.empty()) {
    if ((aid == "admin") || ASSIST("change_state")) {
      uint32_t id = std::stol(param1);

      _Thread th = _Thread::read(id);
      th.state = std::stol(param2);
      _Thread::write(th);

      ret += (" New state: " + th.resolve_state());
    } else {
      ret = "Access denied.";
    }
  } // new-state

  if (action == "ban") {
    if ((aid == "admin") || ASSIST("ban")) {
      string id                    = param1;
      unordered_set<string>& xlist = (param2 == "ip") ? IPBanList : IDBanList;

      auto iter = xlist.find(id);

      if (iter == xlist.end()) {
        xlist.insert(id);
        ret += (" " + id + " banned");
      } else {
        xlist.erase(iter);
        ret += (" " + id + " unbanned");
      }

      cc_store_set_to_file(configs.global().get("file::ban::ip_based"), IPBanList);
      cc_store_set_to_file(configs.global().get("file::ban::id_based"), IDBanList);
    } else {
      ret = "Access denied.";
    }
  } // ban

  if (action == "hide-image") {
    _Thread th = _Thread::read(std::stol(param1));

    string oldname = "images/" + th.img;
    string newname = oldname + ".tmp";
    rename(oldname.c_str(), newname.c_str());
  }

  if (action == "kill-all") {
    if ((aid == "admin") || ASSIST("kill")) {
      string id = param1;
      _Thread th, root = _Thread::read(0);
      bool    ipflag = (param2 == "ip");

      clock_t startc = clock();

      for (int32_t i = root.children_count; i > 0; i--) {
        th = _Thread::read(i);

        if (ipflag)
        {
          if (th.ip == id) actions::kill::call(conn, i, true);
        }
        else
        {
          if (th.author == id) actions::kill::call(conn, i, true);
        }
      }
      clock_t endc = clock();

      ret += (" Take " + to_string((float)(endc - startc) / CLOCKS_PER_SEC));
    } else {
      ret = "Access denied.";
    }
  } // kill-all

  if (action == "search") {
    if ((aid == "admin") || ASSIST("search")) {
      _Thread th, root = _Thread::read(0);

      int32_t c = 0, limit = atol(param2.c_str());
      ret = "[";

      for (int32_t i = root.children_count; i > 0; i--) {
        if (++c == limit) break;
        th = _Thread::read(i);

        if (th.content.find(param1) != std::string::npos) {
          ret += ("\"" + to_string(th.no) + "\",");
        }
      }

      ret += "\"\"]";
    } else {
      ret = "[]";
    }
  } // search

  mg_send_header(conn, "Content-Type", "text/plain");
  mg_printf_data(conn, ret.c_str());
}
} // namespace admin
} // namespace actions

namespace views {
namespace admin {
void render(mg_connection *conn) {
  string ip_ban_list = "";
  string id_ban_list = "";

  for (auto i = IPBanList.begin(); i != IPBanList.end();
       ++i) ip_ban_list += (*i + ",");

  for (auto j = IDBanList.begin(); j != IDBanList.end();
       ++j) id_ban_list += (*j + ",");

  time_t c; time(&c);
  unsigned long dummy, res = 0;

  FILE *f = fopen("/proc/self/statm", "r");

  if (f) {
    fscanf(f,
           "%ld %ld %ld %ld %ld %ld %ld",
           &dummy,
           &res,
           &dummy,
           &dummy,
           &dummy,
           &dummy,
           &dummy);
    fclose(f);
  }

  string ass = is_assist(conn);

  templates.invoke("site_header").toggle("admin_panel_page").pipe_to(conn).destory();
  templates.invoke("admin_panel")                              \
  .toggle("is_admin",  is_admin(conn) || !ass.empty())         \
  .toggle("is_assist", !ass.empty())                           \
  .var("ASSIST_NAME",    ass)                                  \
  .var("RUNNING_TIME",   (int)((c - gStartupTime) / 3600))     \
  .var("MEMORY_USAGE",   res * 4)                              \
  .var("IP_BAN_LIST",    ip_ban_list)                          \
  .var("ID_BAN_LIST",    id_ban_list)                          \
  .var("GLOBAL_CONFIGS", configs.global().serialize_to_json()) \
  .pipe_to(conn).destory();
  templates.invoke("site_footer").var("TIME", 0).pipe_to(conn).destory();
}
} // admin
} // views

static int event_handle(struct mg_connection *conn, enum mg_event ev) {
  switch (ev) {
  case MG_REQUEST:

    if (conn->is_websocket) {
      websocket::poll(conn);
      return MG_TRUE;
    }
    else if (strstr(conn->uri, "/assets/")) {
      mg_send_file(conn, conn->uri + 1, "");
      return MG_MORE;
    }
    else {
      pair<int, map<string, string> >r = route::route(conn->uri);

      // for(auto i = r.second.begin(); i != r.second.end(); ++i) cout <<
      // i->first << "=" << i ->second << endl;

      int id;
      _Thread th;

      switch (r.first) {
      case route::POST_THREAD:
      case route::POST_REPLY:

        if (!check_ip(conn) || configs.global().get<bool>("archive")) break;
        actions::post::call(conn,
                            r.first ==
                            route::POST_THREAD ? 0 : atol(r.second["no"].c_str()));
        break;

      case route::SUCCESS:
        views::success::render(conn, r.second["cookie"],
                               atol(r.second["no"].c_str()));
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

        if (!check_ip(conn, true)) break;
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

        if (actions::kill::call(conn, id)) {
          views::info::render(conn, templates.invoke("misc")
              .toggle("delete_okay").var("THREAD_NO", id).build_destory().c_str());
        } else {
          views::info::render(conn, templates.invoke("misc")
              .toggle("try_to_usurp").build_destory().c_str());
        }
        break;

      case route::API_THREAD:
        th = _Thread::read(std::stol(r.second["no"]));
        views::each_thread(conn, th, 0, is_admin(conn));
        break;

      case route::IMAGE:
        cc_serve_image_file(conn);
        break;

      case route::ADMIN_ACTION:
        actions::admin::call(conn);
        break;

      default:

        if (configs.global().get<bool>("welcome")) {
          templates.invoke("site_header").pipe_to(conn).destory();
          templates.invoke("site_welcome").pipe_to(conn).destory();
          templates.invoke("site_footer").var("TIME", 0).pipe_to(conn).destory();
        }
        else {
          views::timeline::render(conn, true);
        }
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
  signal(sig_num, signal_handle); // Reinstantiate signal handler
  s_signal_received = sig_num;
}

int main(int argc, char *argv[]) {
  string conf_path = "./chann.conf";

  log_file = stdout;

  for (int i = 0; i < argc; ++i) {
    if (((strcmp(argv[i], "--load") == 0) || (strcmp(argv[i], "-l") == 0))) {
      conf_path = string(argv[++i]);
      continue;
    }
  }

  configs.global().load(conf_path);
  logLog("Load configuration: %s", conf_path.c_str());

  string db_path = configs.global().get("database");

  ldb.global().init(db_path);
  _Thread::init();

  /*for(int i = 0; i < 100; ++i)
     {
      _Thread th = _Thread::make();
      th.content = to_string(i);
      _Thread::insert_after(_Thread::read(0), th);
     }

     _Thread th = _Thread::make();
     th.content = "reply";
     _Thread::append_child(_Thread::read(3), th);
     th = _Thread::make();
     th.content = "reply 2";
     _Thread::append_child(_Thread::read(3), th);
   */

  // generate a random admin password
  admin_password = cc_random_chars(10);
  admin_cookie   = admin_password;
  configs.global().set("security::admin::cookie", admin_password);

  for (auto& x : cc_split(configs.global().get("security::assist::list"), ",")) {
    assist_cookie[x] = cc_random_chars(10);
  }

  templates.use_lang(configs.global().get("lang"));
  logLog("Load %d templates", templates.load_templates());

  string homepage = configs.global().get<bool>("welcome") ? "/page/1" : "/";
  templates.add_template("site_header") \
    .var("SITE_TITLE", configs.global().get("title")).var("HOMEPAGE", homepage);
  templates.add_template("single_thread_header").var("HOMEPAGE", homepage);
  templates.add_template("info_page").var("HOMEPAGE", homepage);
  templates.add_template("site_footer").var("BUILD_DATE", to_string(BUILD_DATE));

  bool use_captcha = configs.global().get<bool>("security::captcha");
#ifndef GOOGLE_RECAPTCHA
  use_captcha = false;
#endif // ifndef GOOGLE_RECAPTCHA

  templates.add_template("post_form").toggle("use_captcha", use_captcha) \
    .var("LANG", configs.global().get("lang")) \
    .var("PUBLIC", configs.global().get("security::captcha::public_key"));

  cc_load_file_to_set(configs.global().get("file::ban::ip_based"), IPBanList);
  cc_load_file_to_set(configs.global().get("file::ban::id_based"), IDBanList);
  logLog("Load banlist with %d/%d records", IPBanList.size(), IDBanList.size());

  server = mg_create_server(NULL, event_handle);
  mg_set_option(server, "listening_port",
                to_string(configs.global().get<int>("listen")).c_str());
  logLog("Start listening on port %s", mg_get_option(server, "listening_port"));

  fflush(log_file);

  time(&gStartupTime);

  signal(SIGTERM, signal_handle);
  signal(SIGINT,  signal_handle);

  route::init();
  route::add("/thread/:no",             route::THREAD);
  route::add("/daerht/:no",             route::DAERHT);
  route::add("/page/:page",             route::TIMELINE);
  route::add("/gallery/:page",          route::GALLERY);
  route::add("/list",                   route::LINEAR);
  route::add("/list/ip/:ip",            route::LINEAR_IP);
  route::add("/list/id/:id",            route::LINEAR_ID);
  route::add("/admin",                  route::ADMIN_PANEL);
  route::add("/del/:no",                route::SELF_DELETE);
  route::add("/api/:no",                route::API_THREAD);
  route::add("/images/:path",           route::IMAGE);
  route::add("/success/:cookie/to/:no", route::SUCCESS);
  route::add("/admin_action",           route::ADMIN_ACTION);
  route::add("/post_thread",            route::POST_THREAD);
  route::add("/post_reply/:no",         route::POST_REPLY);

#ifdef GOOGLE_RECAPTCHA
  curl_global_init(CURL_GLOBAL_ALL);
#endif // ifdef GOOGLE_RECAPTCHA

  while (s_signal_received == 0) {
    mg_poll_server(server, 1000);
  }

#ifdef GOOGLE_RECAPTCHA
  curl_global_cleanup();
#endif // ifdef GOOGLE_RECAPTCHA

  mg_destroy_server(&server);

  return 0;
}
