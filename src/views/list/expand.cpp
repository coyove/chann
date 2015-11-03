#include "expand.h"

using namespace std;

namespace views{
    namespace expand{
        void render(mg_connection* conn, int id, bool reverse){
            ConfigManager configs;
            // int id = cc_extract_uri_num(conn);

            int limit;
            struct Thread *r = unq_read_thread(pDb, 0); // get the root thread
            limit = r->childCount - configs.global().get<int>("user::viewable_threads");
            delete r;

            if (id < 0 || (id < limit && limit != 0)){
                views::info::render(conn, templates.invoke("misc").toggle("invalid_thread_no").build_destory().c_str());
                return;
            }

            r = unq_read_thread(pDb, id); // get the root thread
            int pid = unq_thread_parent(pDb, r);
            
            if (pid == -1 && !is_admin(conn)){
                views::info::render(conn, templates.invoke("misc").toggle("invalid_thread_no").build_destory().c_str());
                delete r;
                return;
            }

            templates.invoke("site_header").toggle("thread_page").toggle("is_admin", is_admin(conn)) \
                .var("THREAD_NO", id).var("THREAD_TITLE", r->author).pipe_to(conn).destory();

            templates.invoke("single_thread_header").toggle("homepage", !pid).var("THREAD_NO", id) \
                .var("PARENT_NO", pid).toggle("thread", strstr(conn->uri, "/thread/")).pipe_to(conn).destory();

            clock_t startc = clock();

            // bool reverse = strstr(conn->uri, "/daerht/");
            bool admin_view = is_admin(conn);
            string username = cck_verify_ssid(conn);

            views::each_thread(conn, r, 0, admin_view);
            mg_printf_data(conn, "<hr>");

            char iid[10];
            strcpy(iid, r->ssid);

            if(!configs.global().get<bool>("archive"))
                templates.invoke("post_form") \
                    .var("THREAD_NO", to_string(id)).toggle("reply_to_thread"). \
                    toggle("is_admin", admin_view || !is_assist(conn).empty()).pipe_to(conn).destory();
                
            if (r->childThread) {
                cclong r_childThread = r->childThread;
                delete r;

                int limit = configs.global().get<int>("user::collapse_image");

                r = unq_read_thread(pDb, r_childThread); // beginning of the circle
                cclong rid = r->threadID; //the ID
                bool too_many_replies = (r->childCount > limit);

                int di = 1;

                if(reverse){
                    cclong n_rid = r->prevThread;
                    delete r;
                    r = unq_read_thread(pDb, n_rid);

                    views::each_thread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK, admin_view, iid, username.c_str());

                    while (r->prevThread != n_rid){
                        di++;
                        cclong r_prevThread = r->prevThread;

                        delete r;
                        r = unq_read_thread(pDb, r_prevThread);

                        if(too_many_replies && (di > limit))
                            views::each_thread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK + SEND_CUT_IMAGE, admin_view, iid, username.c_str());
                        else
                            views::each_thread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK, admin_view, iid, username.c_str());
                    }
                }else{
                    views::each_thread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK, admin_view, iid, username.c_str());

                    while (r->nextThread != rid){
                        cclong r_nextThread = r->nextThread;
                        delete r;

                        r = unq_read_thread(pDb, r_nextThread);
                        di++;

                        //views::each_thread(conn, r, true, true, false, too_many_replies && (di > 20), admin_view, iid);
                        if(too_many_replies && (di > limit))
                            views::each_thread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK + SEND_CUT_IMAGE, admin_view, iid, username.c_str());
                        else
                            views::each_thread(conn, r, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK, admin_view, iid, username.c_str());
                    }

                    if(r) delete r;
                } 
            }

            clock_t endc = clock();

            // site_footer(conn, (float)(endc-startc)/CLOCKS_PER_SEC);
            templates.invoke("site_footer").var("TIME", (int)((endc-startc) * 1000 / CLOCKS_PER_SEC)).pipe_to(conn).destory();
        }
    }

}