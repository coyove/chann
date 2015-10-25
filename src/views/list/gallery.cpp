#include "gallery.h"

using namespace std;

namespace views{
    namespace gallery{
        void showGallery(mg_connection* conn, cclong startID, cclong endID){
            struct Thread *r = unq_read_thread(pDb, 0); // get the root thread
            cclong c = 0;

            bool admin_view = is_admin(conn);

            cclong totalThreads = r->childCount;
            // cclong totalPages = 100;

            templates.invoke("site_slogan").pipe_to(conn).destory();

            for(cclong i = totalThreads; i > 0; --i){
                delete r;
                r = unq_read_thread(pDb, i);
                if(strlen(r->imgSrc) == 15 && r->state & NORMAL_DISPLAY) {
                    c++;
                    if (c >= startID && c <= endID){
                        mg_printf_data(conn, "<hr>");
                        views::each_thread(conn, r, SEND_SHOW_REPLY_LINK + SEND_CUT_LONG_COMMENT, admin_view);
                    }
                }
                
                if (c == endID + 1) break;
            }
            if(r) delete r;
        }
        
        void render(mg_connection* conn){
            ConfigManager configs;
            int num = cc_extract_uri_num(conn);

            templates.invoke("site_header").var("CURRENT_PAGE", num).toggle("is_admin", is_admin(conn)) \
                .toggle("gallery_page").pipe_to(conn).destory();

            if(!configs.global().get<bool>("archive")) 
                templates.invoke("post_form").toggle("is_admin", is_admin(conn)).toggle("post_new_thread").pipe_to(conn).destory();

            clock_t startc = clock();
            int max_page_viewable = configs.global().get<int>("user::viewable_pages");
            int threads_per_page = configs.global().get<int>("user::threads_per_page");

            bool admin_view = is_admin(conn);
            int start_no = 1, end_no = threads_per_page;

            cclong cp = cc_extract_uri_num(conn);
            if(cp <= max_page_viewable || max_page_viewable == 0 || admin_view){
                end_no = cp * threads_per_page;
                start_no = end_no - threads_per_page + 1;
                showGallery(conn, start_no, end_no);
            }
            
            cclong current_page = end_no / threads_per_page;
            queue<string> before, after;
            for (cclong i = 1; i <= (max_page_viewable == 0 || admin_view ? 10000 : max_page_viewable); ++i){
                if (current_page - i < 4 && current_page - i > 0) before.push(to_string(i));
                else if (i - current_page < 4 && i - current_page > 0) after.push(to_string(i));
                else if (i - current_page > 4) break;
            }
            
            templates.invoke("pager").toggle("gallery_page") \
                .var("CURRENT_PAGE", current_page) \
                .var("PREVIOUS_PAGE", current_page > 1 ? (current_page - 1) : 1) \
                .var("NEXT_PAGE", current_page + 1) \
                .loop("before_pages", before).loop("after_pages", after).pipe_to(conn).destory();

            clock_t endc = clock();
            templates.invoke("site_footer").var("TIME", (int)((endc-startc) * 1000 / CLOCKS_PER_SEC)).pipe_to(conn).destory();
        }
    }

}