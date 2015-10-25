#include "list.h"

void views_list_threads_linear(mg_connection* conn, 
    bool admin_view = false, 
    bool ip_based = false,
    bool id_based = false,
    const char* needle = ""){

    string username = cck_verify_ssid(conn);

    if (!username.empty()){
        templates.invoke("site_header").toggle("my_post_page").toggle("is_admin", admin_view).pipe_to(conn).destory();

        struct Thread *r = readThread_(pDb, 0); 
        struct Thread *t;

        clock_t startc = clock();

        int limit = configs.global().get<int>("user::linear_threads");

        for(int i = r->childCount; i > 0; i--){
            t = readThread_(pDb, i);
            if(r->childCount - i > limit && !admin_view) break; // only search the most recent 500 threads/replies

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
                    send_single_thread(conn, t, SEND_CUT_LONG_COMMENT + SEND_CUT_IMAGE, admin_view);
                else    
                    send_single_thread(conn, t, SEND_IS_REPLY + SEND_CUT_LONG_COMMENT + SEND_CUT_IMAGE, admin_view);
                mg_printf_data(conn, "<hr>");
            }

            if(t) delete t;
        }
        clock_t endc = clock();
        //site_footer(conn, (float)(endc-startc)/CLOCKS_PER_SEC);
        templates.invoke("site_footer").var("TIME", (int)((endc-startc) * 1000 / CLOCKS_PER_SEC)).pipe_to(conn).destory();

        if(r) delete r;
    }else{
        templates.invoke("site_header").toggle("my_post_page").pipe_to(conn).destory();
        templates.invoke("site_footer").var("TIME", 0).pipe_to(conn).destory();
    }
}