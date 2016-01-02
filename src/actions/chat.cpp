#include "chat.h"

using namespace std;

extern string admin_cookie;
extern struct mg_server *server;

namespace websocket{
    deque<struct History *> history;
    ConfigManager configs;

    void broadcast(mg_connection *conn){
        struct ChatData *d = (struct ChatData *) conn->connection_param;
        struct mg_connection *c;

        time_t rawtime;
        time(&rawtime);

        struct History *h = new History();
        strcpy(h->ssid, d->ssid);

        int len = conn->content_len - 4;
        len = len > 1023 ? 1023 : len;
        strncpy(h->message, conn->content + 4, len);
        h->message[len] = 0;

        h->timestamp = rawtime;

        history.push_front(h);
        while(history.size() > configs.global().get<int>("chat::max_histories")){
            struct History *h2 = history.back();
            delete h2;
            history.pop_back();
        }

        for (c = mg_next(server, NULL); c != NULL; c = mg_next(server, c)) {
            struct ChatData *d2 = (struct ChatData *) c->connection_param;
            if (!c->is_websocket || d2->roomID != d->roomID) continue;

            if(strcmp(d->ssid, admin_cookie.c_str()) == 0)
                mg_websocket_printf(c, WEBSOCKET_OPCODE_TEXT, "msg %c Admin %d %.*s",
                          (char) d->roomID, rawtime, conn->content_len - 4, conn->content + 4);
            else
                mg_websocket_printf(c, WEBSOCKET_OPCODE_TEXT, "msg %c %.9s %d %.*s",
                          (char) d->roomID, d->ssid, rawtime, len, h->message);
        }

        //delete [] bufMessage;
    }

    void poll(mg_connection *conn) {
        if(!conn->connection_param) return;

        struct ChatData *d = (struct ChatData *) conn->connection_param;

        string username = cck_verify_ssid(d->ssid);
        if(username.empty()) return;

        struct mg_connection *c;

        if (conn->content_len > 5 && !memcmp(conn->content, "join ", 5)) {
        // Client joined new room
            d->roomID = conn->content[5];
        } else if (conn->content_len > 4 && !memcmp(conn->content, "msg ", 4) &&
                 d->roomID != 0 && d->roomID != '?') {
            broadcast(conn);
        }
    }

    int connect(mg_connection *conn) {
        string username, ssid;
        username = cck_verify_ssid(conn);
        ssid = cck_extract_ssid(conn);

        conn->connection_param = new ChatData();
        strcpy(((struct ChatData *)conn->connection_param)->ssid, ssid.c_str());

        if(!username.empty()){
            mg_websocket_printf(conn, WEBSOCKET_OPCODE_TEXT, "id %s", username.c_str());               
        }else
            mg_websocket_printf(conn, WEBSOCKET_OPCODE_TEXT, "id (null)");
        
        if(history.size() > 0){
                // for(auto i = chatHistory.size() - 1; i >= 0; i--){
            for(auto i = 0; i < history.size(); ++i){
                if(strcmp(history[i]->ssid, admin_cookie.c_str()) == 0)
                    mg_websocket_printf(conn, WEBSOCKET_OPCODE_TEXT, "history %d Admin %s", 
                    history[i]->timestamp, history[i]->message);
                else
                    mg_websocket_printf(conn, WEBSOCKET_OPCODE_TEXT, "history %d %.9s %s", 
                        history[i]->timestamp, history[i]->ssid, history[i]->message);
            }
        }

        mg_websocket_printf(conn, WEBSOCKET_OPCODE_TEXT, "end history");
    }

    void disconnect(mg_connection *conn){
        if (conn->connection_param)
            delete (struct ChatData*)conn->connection_param;
    }
}
