#include "timeline.h"

using namespace std;

namespace views{
    namespace timeline{
        void showThreads(mg_connection* conn, int startID, int endID){
            struct Thread *r = unq_read_thread(pDb, 0); // get the root thread
            int c = 0;

            bool admin_view = is_admin(conn);
            string username = cck_verify_ssid(conn);

            int totalThreads = r->childCount;

            templates.invoke("site_slogan").pipe_to(conn).destory();

            while (r->nextThread){
                int tmpid = r->nextThread;
                delete r;
                r = unq_read_thread(pDb, tmpid);
                c++;
                if (c >= startID && c <= endID){
                    mg_printf_data(conn, "<hr>");
                    //views::each_thread(conn, r, false, true, true, false, admin_view);
                    views::each_thread(conn, r, SEND_SHOW_REPLY_LINK + SEND_CUT_LONG_COMMENT + SEND_CUT_REPLY_COUNT, admin_view, "", username.c_str());
                    char *iid = new char[10];
                    strcpy(iid, r->ssid);

                    struct Thread *oldr = r;

                    if (r->childThread) {
                        struct Thread* first_thread;
                        first_thread = unq_read_thread(pDb, r->childThread); // beginning of the circle
                        r = first_thread;

                        vector<struct Thread *> vt;
                        // vector<struct Thread *> vt_next;

                        int i = 1;
                        while(i <= 4){
                            r = unq_read_thread(pDb, r->prevThread);

                            if(r->threadID != first_thread->threadID) {
                                vt.push_back(r);
                                i++;
                            }else
                                break;
                        }

                        vt.push_back(first_thread);

                        if(first_thread->childCount <= 5)
                            for (int i = vt.size() - 1; i >= 0; i--){
                                views::each_thread(conn, vt[i], SEND_IS_REPLY + SEND_CUT_LONG_COMMENT, admin_view, iid, username.c_str());
                                delete vt[i];
                            }
                        else{
                            for (int i = vt.size() - 1; i >= 0; i--){
                                views::each_thread(conn, vt[i], SEND_IS_REPLY + SEND_CUT_LONG_COMMENT, admin_view, iid, username.c_str());
                                delete vt[i];
                                if(i == vt.size() - 1){
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
        }
        
        void render(mg_connection* conn, bool indexPage){
            ConfigManager configs;
            int num = indexPage ? 1 : cc_extract_uri_num(conn);

            templates.invoke("site_header").var("CURRENT_PAGE", num).toggle("is_admin", is_admin(conn)) \
                .toggle("timeline_page").pipe_to(conn).destory();

            if(!configs.global().get<bool>("archive")) 
                templates.invoke("post_form").toggle("is_admin", is_admin(conn) || !is_assist(conn).empty()) \
                .toggle("post_new_thread").pipe_to(conn).destory();

            clock_t startc = clock();
            int max_page_viewable = configs.global().get<int>("user::viewable_pages");
            int threads_per_page = configs.global().get<int>("user::threads_per_page");

            bool admin_view = is_admin(conn);
            int start_no = 1, end_no = threads_per_page;

            if(indexPage)
                showThreads(conn, start_no, end_no);
            else{
                cclong cp = cc_extract_uri_num(conn);
                if(cp <= max_page_viewable || max_page_viewable == 0 || admin_view){
                    end_no = cp * threads_per_page;
                    start_no = end_no- threads_per_page + 1;
                    showThreads(conn, start_no, end_no);
                }
            }

            cclong current_page = end_no / threads_per_page;
            queue<string> before, after;
            for (cclong i = 1; i <= (max_page_viewable == 0 || admin_view ? 10000 : max_page_viewable); ++i){
                if (current_page - i < 4 && current_page - i > 0) before.push(to_string(i));
                else if (i - current_page < 4 && i - current_page > 0) after.push(to_string(i));
                else if (i - current_page > 4) break;
            }
            
            templates.invoke("pager").toggle("timeline_page") \
                .var("CURRENT_PAGE", current_page) \
                .var("PREVIOUS_PAGE", current_page > 1 ? (current_page - 1) : 1) \
                .var("NEXT_PAGE", current_page + 1) \
                .loop("before_pages", before).loop("after_pages", after).pipe_to(conn).destory();

            clock_t endc = clock();
            templates.invoke("site_footer").var("TIME", (int)((endc-startc) * 1000 / CLOCKS_PER_SEC)).pipe_to(conn).destory();
        }
    }

}