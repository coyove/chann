#include "single.h"

#include "../general.h"
#include "../helper.h"
#include "../config.h"
#include "../tags.h"

using namespace std;

namespace views{
    void each_thread(mg_connection* conn, struct Thread* r, char display_state, bool admin_view, 
        const char* iid,
        const char* uid){
        time_t now = time(NULL);

        string thread_post_time = cc_timestamp_to_time(r->date);
        int diff_day = cc_timestamp_diff_day(now, r->date);

        const char* content = unq_read_string(pDb, r->content);
        string thread_content(content);
        delete [] content;

        if((display_state & SEND_CUT_LONG_COMMENT) && thread_content.size() > 1024) 
            thread_content = thread_content.substr(0, 1024) + templates.invoke("misc").toggle("more_contents").build_destory();
            //+ "<font color='red'><b>&#128065;" + STRING_MORE + "</b></font>";

        HTMLTemplate *ht = templates.invoke_pointer("single_thread");

        map<string, bool> stats;
        map<string, string> vars;

        ConfigManager c;

        stats["reply"]                      = display_state & SEND_IS_REPLY;
        stats["show_reply"]                 = display_state & SEND_SHOW_REPLY_LINK;
        stats["archive"]                    = c.global().get<bool>("archive");
        stats["normal_display"]             = r->state & NORMAL_DISPLAY || admin_view;
        stats["thread_poster_is_admin"]     = (strcmp(r->ssid, "Admin") == 0);
        stats["thread_poster_is_sameone"]   = (strcmp(r->ssid, iid) == 0);
        stats["is_sameone"]                 = (strcmp(r->ssid, uid) == 0);
        stats["sage"]                       = (r->state & SAGE_THREAD && !stats["reply"]);
        stats["lock"]                       = (r->state & LOCKED_THREAD);
        stats["delete"]                     = !(r->state & NORMAL_DISPLAY);
        stats["show_admin"]                 = admin_view;
        if (strlen(r->imgSrc) >= 4){
            string fname(r->imgSrc);
            vars["THREAD_IMAGE"]            = fname;

            if (!cc_valid_image_ext(fname).empty()){
                stats["image_attached"]     = true;
                if(display_state & SEND_CUT_IMAGE){
                    struct stat st;
                    stat(("images/" + fname).c_str(), &st);
                    stats["show_size_only"] = true;
                    vars["THREAD_IMAGE_SIZE"] = to_string((int)(st.st_size / 1024));
                }
                else{
                    stats["show_full_image"] = true;
                    vars["THREAD_THUMB_PREFIX"] = c.global().get("image::thumb_prefix");
                }
            }else{
                stats["file_attached"]      = true;
            }
        }
        if(r->childThread && !(display_state & SEND_CUT_REPLY_COUNT)){
            struct Thread* c = unq_read_thread(pDb, r->childThread);
            const char *first_reply = unq_read_string(pDb, c->content);
            string display_reply = (c->state & NORMAL_DISPLAY) ? string(first_reply) : "";

            stats["show_num_replies"]       = true;
            vars["NUM_REPLIES"]             = to_string(c->childCount);
            vars["FIRST_REPLY"]             = cc_smart_shorten(display_reply);// display_reply.substr(0, display_reply.find_first_of("<")).substr(0,15);

            delete c;
            delete[] first_reply;
        }

        vars["THREAD_POSTER"]               = string(r->ssid);
        vars["THREAD_NO"]                   = to_string(r->threadID);
        vars["THREAD_CONTENT"]              = thread_content;

        // stats["show_title"]					= !stats["reply"] || strcmp(r->author, STRING_UNTITLED) != 0;
        vars["THREAD_TITLE"]                = string(r->author);

        vars["THREAD_IP"]                   = string(r->email);
        vars["THREAD_POST_TIME"]            = thread_post_time;
        vars["THREAD_STATE"]                = to_string(r->state);

        if(diff_day >= 0 && diff_day <= 2){
            stats["show_easy_date"]         = true;
            vars["THREAD_POST_DATE"]        = to_string(diff_day);
        }else{
            char timetmp[64];
            struct tm post_date;
            localtime_r(&(r->date), &post_date);

            strftime(timetmp, 64, "%Y-%m-%d", &post_date);
            vars["THREAD_POST_DATE"]        = string(timetmp);
        }

        map<string, queue<string>> loops; //nothing

        mg_printf_data(conn, "%s", ht->build(vars, stats, loops).c_str());

        // if(content) delete [] content;
        delete ht;
    }
}