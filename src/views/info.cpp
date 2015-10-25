#include "info.h"

using namespace std;

namespace views{
    namespace info{
        void render(mg_connection* conn, const char* msg){
        	// cout << msg << ";";
            templates.invoke("site_header").pipe_to(conn).destory();
            templates.invoke("info_page").var("CONTENT", msg).toggle("info_page").pipe_to(conn).destory();
            // cout << msg << ";";
            templates.invoke("site_footer").var("TIME", 0).pipe_to(conn).destory();
        }
    }
}