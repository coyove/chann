#include "success.h"

using namespace std;

namespace views{
    namespace success{
        void render(mg_connection* conn, string cookie, int id){
      //   	string url(conn->uri);
		    // vector<string> tmp = cc_split(url, "/");

		    // if(tmp.size() != 3){
		    //     views::info::render(conn, "Error");
		    //     return;
		    // }
		    // cclong id  = atol(tmp[tmp.size() - 1].c_str());

		    cck_send_ssid(conn, cookie);
		    mg_send_header(conn, "charset", "utf-8");
		    mg_send_header(conn, "Content-Type", "text/html");
		    mg_send_header(conn, "cache-control", "private, max-age=0");
		    
		    templates.invoke("site_header").toggle("success_post_page").pipe_to(conn).destory();
		    templates.invoke("info_page").var("THREAD_NO", id).toggle("return_page").pipe_to(conn).destory();
		    templates.invoke("site_footer").var("TIME", 0).pipe_to(conn).destory();
        }
    }
}