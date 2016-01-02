#include "expand.h"

using namespace std;

namespace views{
    namespace expand{
        void render(mg_connection* conn, int id, bool reverse){
            ConfigManager configs;
            // int id = cc_extract_uri_num(conn);

            int limit, count;
            //auto r = unq_read_thread_sp(pDb, 0); // get the root thread
            _Thread r = _Thread::read(0);
            //count = r.get()->childCount;
            count = r.children_count;
            limit = count - configs.global().get<int>("user::viewable_threads");

            if (id < 0 || (id < limit && limit != 0) || id > count){
                views::info::render(conn, templates.invoke("misc").toggle("invalid_thread_no").build_destory().c_str());
                return;
            }

            //r = unq_read_thread_sp(pDb, id); // get the root thread
            r = _Thread::read(id);
            //int pid = unq_thread_parent(pDb, r.get());
            int32_t pid = _Thread::find_parent(r); 
            
            if ((pid == -1) && !is_admin(conn)){
                views::info::render(conn, templates.invoke("misc").toggle("invalid_thread_no").build_destory().c_str());
                return;
            }
            
            templates.invoke("site_header").toggle("thread_page").toggle("is_admin", is_admin(conn)) \
                .var("THREAD_NO", id).var("THREAD_TITLE", r.title).pipe_to(conn).destory();

            templates.invoke("single_thread_header").toggle("homepage", !pid).var("THREAD_NO", id) \
                .var("PARENT_NO", pid).toggle("thread", strstr(conn->uri, "/thread/")).pipe_to(conn).destory();
            
            clock_t startc = clock();

            // bool reverse = strstr(conn->uri, "/daerht/");
            bool admin_view = is_admin(conn);
            string username = cck_verify_ssid(conn);

            views::each_thread(conn, r, 0, admin_view);
            mg_printf_data(conn, "<hr>");

            //char iid[10];
            //strcpy(iid, r.get()->ssid);
            std::string iid = r.author;

            if(!configs.global().get<bool>("archive"))
                templates.invoke("post_form") \
                    .var("THREAD_NO", to_string(id)).toggle("reply_to_thread"). \
                toggle("is_admin", admin_view || !is_assist(conn).empty()).pipe_to(conn).destory();
                
            if (r.child) {
                // int r_childThread = r->childThread;
                // delete r;

                uint32_t limit = configs.global().get<int>("user::collapse_image");

                //r = unq_read_thread_sp(pDb, r.get()->childThread); // beginning of the circle
                _Thread first_child = _Thread::read(r.child);
                //int rid = r.get()->threadID; //the ID
                bool too_many_replies = (r.children_count > limit);

                uint32_t di = 1;

                if(reverse)
                {
                    first_child.goto_prev();
                    _Thread iter = first_child;

                    views::each_thread(conn, iter, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK, admin_view, iid, username);

                    while (iter.goto_prev().no != first_child.no){
                        di++;

//                        r = unq_read_thread_sp(pDb, r.get()->prevThread);

                        if(too_many_replies && (di > limit))
                            views::each_thread(conn, iter, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK + SEND_CUT_IMAGE, admin_view, iid, username);
                        else
                            views::each_thread(conn, iter, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK, admin_view, iid, username);
                    }
                }
                else
                {
                    views::each_thread(conn, first_child, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK, admin_view, iid, username);
                    _Thread iter = first_child;

                    while (iter.goto_next().no != first_child.no)
                    {
                        //r = unq_read_thread_sp(pDb, r.get()->nextThread);
                        di++;

                        if(too_many_replies && (di > limit))
                            views::each_thread(conn, iter, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK + SEND_CUT_IMAGE, admin_view, iid, username);
                        else
                            views::each_thread(conn, iter, SEND_IS_REPLY + SEND_SHOW_REPLY_LINK, admin_view, iid, username);
                    }

                    // if(r) delete r;
                } 
            }

            clock_t endc = clock();

            // site_footer(conn, (float)(endc-startc)/CLOCKS_PER_SEC);
            templates.invoke("site_footer").var("TIME", (int)((endc-startc) * 1000 / CLOCKS_PER_SEC)).pipe_to(conn).destory();
        }
    }

}
