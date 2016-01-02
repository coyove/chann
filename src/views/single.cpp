#include "single.h"

using namespace std;

namespace views{
    void each_thread(mg_connection* conn, _Thread & r, char display_state, bool admin_view, 
        std::string iid,
        std::string uid){
        time_t now = time(NULL);

        string thread_post_time = cc_timestamp_to_time(r.date);
        int diff_day = cc_timestamp_diff_day(now, r.date);

        string thread_content = r.content;

        if((display_state & SEND_CUT_LONG_COMMENT) && thread_content.size() > 1024) 
            thread_content = thread_content.substr(0, 1024) + templates.invoke("misc").toggle("more_contents").build_destory();

        HTMLTemplate * ht = templates.invoke_pointer("single_thread");

        map<string, bool> stats;
        map<string, string> vars;

        ConfigManager c;

        stats["reply"]                      = display_state & SEND_IS_REPLY;
        stats["show_reply"]                 = display_state & SEND_SHOW_REPLY_LINK;
        stats["archive"]                    = c.global().get<bool>("archive");
        stats["normal_display"]             = r.state & _Thread::NORMAL || admin_view;
        stats["thread_poster_is_admin"]     = r.author == "Admin";
        stats["thread_poster_is_sameone"]   = r.author == iid;
        stats["is_sameone"]                 = r.author == uid;
        stats["sage"]                       = (r.state & _Thread::SAGE && !stats["reply"]);
        stats["lock"]                       = (r.state & _Thread::LOCK);
        stats["delete"]                     = !(r.state & _Thread::NORMAL);
        //std::cout << r.no << ":" << r.state << std::endl;
        stats["show_admin"]                 = admin_view;
        if (r.img.size() >= 4){
            string fname = (r.img);
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
        if(r.child && !(display_state & SEND_CUT_REPLY_COUNT)){
            _Thread fr = _Thread::read(r.child);
            string display_reply = (fr.state & _Thread::NORMAL) ? fr.content : "";

            stats["show_num_replies"]       = true;
            vars["NUM_REPLIES"]             = to_string(r.children_count);
            vars["FIRST_REPLY"]             = cc_smart_shorten(display_reply);
        }

        vars["THREAD_POSTER"]               = r.author;
        vars["THREAD_NO"]                   = to_string(r.no);
        vars["THREAD_CONTENT"]              = thread_content;

        // stats["show_title"]					= !stats["reply"] || strcmp(r->author, STRING_UNTITLED) != 0;
        vars["THREAD_TITLE"]                = r.title;

        vars["THREAD_IP"]                   = r.ip;
        vars["THREAD_POST_TIME"]            = thread_post_time;
        vars["THREAD_STATE"]                = to_string(r.state);

        if(diff_day >= 0 && diff_day <= 2){
            stats["show_easy_date"]         = true;
            vars["THREAD_POST_DATE"]        = to_string(diff_day);
        }else{
            char timetmp[64];
            struct tm post_date;
            localtime_r(&(r.date), &post_date);

            strftime(timetmp, 64, "%Y-%m-%d", &post_date);
            vars["THREAD_POST_DATE"]        = string(timetmp);
        }

        map<string, queue<string>> loops; //nothing

        mg_printf_data(conn, "%s", ht->build(vars, stats, loops).c_str());

        // if(content) delete [] content;
        delete ht;
    }

} 
