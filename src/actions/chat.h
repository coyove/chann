#ifndef CHANN_ACTIONS_CHAT_HEADER_INCLUDED
#define CHANN_ACTIONS_CHAT_HEADER_INCLUDED
#include <string>
#include <deque>

extern "C" {
#include "../../lib/unqlite/unqlite.h"
#include "../../lib/mongoose/mongoose.h"
}

#include "../general.h"
#include "../helper.h"
#include "../config.h"
#include "../tags.h"

struct ChatData {
    int roomID;
    char ssid[64];
};

struct History{
    char ssid[64];   
    char message[1024] = {0};
    time_t timestamp;
};


namespace websocket{
    // void broadcast(mg_connection *conn);
    void poll(mg_connection *conn);
    int connect(mg_connection *conn);
    void disconnect(mg_connection *conn);
}


#endif