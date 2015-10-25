#ifndef CCHAN_ROUTER_HEADER
#define CCHAN_ROUTER_HEADER

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include "helper.h"

namespace route{
    enum{
        THREAD      = 1,
        DAERHT      = 2,
        IMAGE       = 3,
        SUCCESS     = 4,
        TIMELINE    = 100,
        GALLERY     = 101,
        LINEAR      = 102,
        LINEAR_IP   = 103,
        LINEAR_ID   = 104,
        ADMIN_PANEL = 200,

        SELF_DELETE = 501,
        API_THREAD  = 502,
        ADMIN_ACTION= 503,
        POST_THREAD = 504,
        POST_REPLY  = 505
    };

    struct Tree{
        std::map<std::string, struct Tree*> to;
        std::string catcher;

        int ending;
    };

    struct Tree* root;

    void init(){
        root = new Tree();
    }

    void add(const std::string pattern, int no){
        // "/page/:num"
        std::vector<std::string> v = cc_split(pattern, "/");
        struct Tree *top = root;

        for(auto i: v)
            if(!i.empty()){
                if(i[0] == ':'){
                    top->catcher = i.substr(1);
                }else{
                    if(!top->to[i]) top->to[i] = new Tree();
                    top = top->to[i];
                }
            }

        top->ending = no;
    }

    std::pair<int, std::map<std::string, std::string>>
    route(const std::string& uri){
        std::string tmp = "";
        int last_pos = 1;
        struct Tree *top = root;
        std::map<std::string, std::string> ret;
        if(uri == "/" || uri.empty()) return make_pair(0, ret);

        std::string has_catcher = "";

        for(auto i = 1; i < uri.size(); ++i){
            if(uri[i] == '/'){
                tmp = uri.substr(last_pos, i - last_pos);
                last_pos = i + 1;

                if(!has_catcher.empty()){
                    ret[has_catcher] = tmp;
                    has_catcher = "";
                    continue;
                }

                if(top->to[tmp]){
                    top = top->to[tmp];

                    if(!top->catcher.empty()){
                        has_catcher = top->catcher;
                    }
                    continue;
                }
                else{
                    return make_pair(-1, ret);
                }
            }
            if(i == uri.size() - 1){
                tmp = uri.substr(last_pos, i - last_pos + 1);
                // std::cout << tmp << ",";
                if(!has_catcher.empty()){
                    ret[has_catcher] = tmp;
                    break;
                }

                if(top->to[tmp]) 
                    top = top->to[tmp];
                else
                    return make_pair(-1, ret);
            }
        }

        return make_pair(top->ending, ret);
    }
}

#endif