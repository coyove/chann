#include "kill.h"

using namespace std;

namespace actions{
    namespace kill{
        ConfigManager configs;

        bool call(mg_connection* conn, int tid, bool admin){
            string username = cck_verify_ssid(conn);
            bool flag = false;

            if (!username.empty()){
                struct Thread* t = unq_read_thread(pDb, tid);

                if (strcmp(t->ssid, username.c_str()) == 0 || admin){
                    unq_delete_thread(pDb, tid);
                    logLog("%s has deleted No.%d", username.c_str(), tid);
                    flag = true;
                }
                else{
                    flag = false;
                }

                if(t) delete t;
            }else
                flag = false;

            return flag;
        }
    }
}