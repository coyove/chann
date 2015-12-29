#include "linear.h"

using namespace std;

namespace views{
    namespace linear{
        void render(mg_connection* conn, 
            bool ip_based,
            bool id_based,
            const char* needle){

            string ssid = cck_extract_ssid(conn);
            string username = cck_verify_ssid(conn);
            ConfigManager configs;

            bool admin_view = is_admin(conn);
            if(!admin_view){
                string ass = is_assist(conn);
                if(!ass.empty() && configs.global().get<bool>("security::assist::" + ass + "::list")) admin_view = true;
            }

            if (!username.empty()){
                templates.invoke("site_header").toggle("my_post_page").toggle("is_admin", admin_view).pipe_to(conn).destory();

                struct Thread *r = unq_read_thread(pDb, 0); 
                struct Thread *t;

                clock_t startc = clock();

                int limit = configs.global().get<int>("user::linear_threads");

                for(int i = r->childCount; i > 0; i--){
                    t = unq_read_thread(pDb, i);
                    if(r->childCount - i > limit && !admin_view) break; // only search the most recent 500 threads/replies
                    // cout << i << endl;
                    bool flag = false;

                    if(!ip_based && !id_based){ //list the threads only based on username, admin can see all
                        if(admin_view) flag = true;
                        if(strcmp(username.c_str(), t->ssid) == 0) flag = true;
                    }else if(ip_based){ // list the threads based on ip
                        if(strcmp(t->email, needle) == 0) flag = true;
                    }else if(id_based){ // list the threads based on id
                        if(strcmp(t->ssid, needle) == 0) flag = true;
                    }

                    if(flag){
                        if(t->state & MAIN_THREAD)
                            views::each_thread(conn, t, SEND_CUT_LONG_COMMENT + SEND_CUT_IMAGE, admin_view);
                        else    
                            views::each_thread(conn, t, SEND_IS_REPLY + SEND_CUT_LONG_COMMENT + SEND_CUT_IMAGE, admin_view);
                        mg_printf_data(conn, "<hr>");
                    }

                    if(t) delete t;
                }
                clock_t endc = clock();
                //site_footer(conn, (float)(endc-startc)/CLOCKS_PER_SEC);
                templates.invoke("site_footer").var("BUILD_DATE", ssid).var("TIME", (int)((endc-startc) * 1000 / CLOCKS_PER_SEC)).pipe_to(conn).destory();

                if(r) delete r;
            }else{
                templates.invoke("site_header").toggle("my_post_page").pipe_to(conn).destory();
                templates.invoke("site_footer").var("TIME", 0).pipe_to(conn).destory();
            }
        }
    }

}