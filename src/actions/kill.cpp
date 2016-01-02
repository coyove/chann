#include "kill.h"

using namespace std;

namespace actions{
    namespace kill{
        ConfigManager configs;

        bool call(mg_connection * conn, uint32_t tid, bool admin){
            string username = cck_verify_ssid(conn);
            bool flag = false;

            if (admin || !username.empty())
            {
                //struct Thread* t = unq_read_thread(pDb, tid);
                _Thread th = _Thread::read(tid);

                if (th.author == username || admin)
                {
                    if (th.state & _Thread::THREAD)
                        _Thread::unlink(tid);
                    else
                    {
                        th.state |= _Thread::NORMAL;
                        th.state -= _Thread::NORMAL;
                        _Thread::write(th);
                    }
                    logLog("%s has deleted No.%d", username.c_str(), tid);
                    flag = true;
                }
                else
                    flag = false;
            }
            else
                flag = false;

            return flag;
        }
    }
}
