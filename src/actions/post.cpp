#include "post.h"

using namespace std;

extern unordered_set<string> IDBanList;

namespace actions{
    namespace post{
        ConfigManager configs;

        static size_t curl_callback(void *contents, size_t size, size_t nmemb, void *userp){ 
            ((std::string*)userp)->append((char*)contents, size * nmemb);
            return size * nmemb;
        }

        void call(mg_connection* conn, int post_to){
            bool sage = false;
            bool fileAttached = false;

            const char *data;
            int data_len, ofs = 0, mofs = 0;
            char var_name[128], file_name[128];

            bool admin_ctrl = is_admin(conn);
            
            string dump_name;
            if(configs.global().get<bool>("post::dump::raw"))
            {
                dump_name = cc_random_chars(11);
                cc_write_binary((configs.global().get("post::dump::to") + dump_name).c_str(), conn->content, conn->content_len);
            }

            if(conn->content_len <= 0){
                views::info::render(conn, "Error");
                return;
            }

            if(post_to == 0 && configs.global().get<bool>("user::can_only_reply") && !admin_ctrl){
                views::info::render(conn, templates.invoke("misc").toggle("can_only_reply").build_destory().c_str());
                return;
            }

            char p_title[64] = {0}, p_options[64] = {'\0'}, p_image[16] = {'\0'}, g_data[1024] = {0};
            // char p_content[4096] = {'\0'};
            int max_len = configs.global().get<int>(string("post::max_size::") + (admin_ctrl ? "admin" : "user"));
            if(max_len == 0) max_len = 4096;

            auto_ptr<char> p_content(new char[max_len]());

            #define GET_VAR(x, l, v) \
                if (strcmp(var_name, x) == 0) { \
                    data_len = data_len > (l) ? (l) : data_len; \
                    strncpy(v, data, data_len); \
                    v[data_len] = 0; }
            while ((mofs = mg_parse_multipart(conn->content + ofs, conn->content_len - ofs,
                var_name, sizeof(var_name),
                file_name, sizeof(file_name),
                &data, &data_len)) > 0) {
                ofs += mofs;

                GET_VAR("input_name", 63, p_title);
                GET_VAR("input_email", 63, p_options);
                GET_VAR("input_content", max_len - 1, p_content.get());

                #ifdef GOOGLE_RECAPTCHA
                GET_VAR("g-recaptcha-response", 1023, g_data);
                #endif

                if (strcmp(var_name, "input_file") == 0 && strcmp(file_name, "") != 0) {
                    int ilimit = configs.global().get<int>("image::max_size");
                    if (data_len > 1024 * ilimit){
                        views::info::render(conn, templates.invoke("misc").toggle("too_big_image").var("MAX", ilimit).build_destory().c_str());
                        return;
                    }

                    string original(file_name), new_name = cc_random_chars(11);
                    std::transform(original.begin(), original.end(), original.begin(), ::tolower);

                    string ext = cc_valid_image_ext(original);
                    if(ext.empty()){
                        if (admin_ctrl) 
                            ext = original.substr(original.size() - 4);
                        else{
                            views::info::render(conn, templates.invoke("misc").toggle("invalid_image").build_destory().c_str());
                            return;
                        }
                    }

                    fileAttached = true;
                    string fpath = "images/" + new_name + ext;

                    cc_write_binary(fpath.c_str(), data, data_len);

                    strcpy(p_image, (new_name + ext).c_str());
                    logLog("Image uploaded: %s", p_image);
                }
            }

            #ifdef GOOGLE_RECAPTCHA
            if(configs.global().get<bool>("security::captcha")){
                CURL *curl;
                CURLcode res;
                curl = curl_easy_init();
                if(curl) {
                    curl_easy_setopt(curl, CURLOPT_URL, "https://www.google.com/recaptcha/api/siteverify");
                    string read_buffer = ""; 
                    string post_data = "secret=" + configs.global().get("security::captcha::private_key") + "&response=" + string(g_data);

                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
                    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
                    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
                    
                    res = curl_easy_perform(curl);
                    /* Check for errors */ 
                    if(res != CURLE_OK || read_buffer.find("true") == string::npos){
                        views::info::render(conn, templates.invoke("misc").toggle("captcha_failed").build_destory().c_str());
                        return;
                    }
                    /* always cleanup */ 
                    curl_easy_cleanup(curl);
                    
                }
            }
            #endif

            #undef GET_VAR

            string tmpcontent(p_content.get());

            if (strcmp(p_title, "") == 0) strcpy(p_title, "untitled");
            if (strstr(p_options, "sage") && configs.global().get<bool>("post::allow_self::sage")) sage = true;

            if (strstr(p_options, "url"))
            {
                templates.invoke("site_header").toggle("upload_image_page").pipe_to(conn).destory();
                templates.invoke("info_page").toggle("upload_image_page").var("IMAGE", p_image).pipe_to(conn).destory();
                templates.invoke("site_footer").var("TIME", 0).pipe_to(conn).destory();

                logLog("Image uploaded and won't be posted");
                return;
            }
            //admin/assist trying to update a thread/reply
            if (strstr(p_options, "update") && 
                (admin_ctrl || configs.global().get<bool>("security::assist::" + is_assist(conn) + "::update"))){
                uint32_t id = cc_extract_uri_num(conn);
                _Thread th = _Thread::read(id);

                if(!strstr(p_options, "html"))
                    th.content = cc_htmlify(tmpcontent, false);
                else
                    th.content = tmpcontent;

                _Thread::write(th);

                views::info::render(conn, "Thread updated successfully");
                logLog("Admin (%s) has edited No.%d", is_assist(conn).c_str(), th.no);

                return;
            }

                //image or comment or both
            if (strcmp(p_content.get(), "") == 0 && !fileAttached) {
                views::info::render(conn, templates.invoke("misc").toggle("cannot_post_null").build_destory().c_str());
                return;
            }else if (strcmp(p_content.get(), "") == 0 && fileAttached){
                strcpy(p_content.get(), templates.invoke("misc").toggle("null_comment").build_destory().c_str());
            }
            

            string username = cck_verify_ssid(conn);
            string ssid = cck_extract_ssid(conn);

            if (!username.empty()){  //passed
                string findID = username;
                // if(strcmp(ssid, adminCookie) != 0)
                if (IDBanList.find(findID) != IDBanList.end()){
                    //this id is banned, so we destory it
                    cck_destory_ssid(conn);
                    logLog("Banned cookie destoryed: '%s'", ssid.c_str());
                    views::info::render(conn, templates.invoke("misc").toggle("id_is_banned").build_destory().c_str());
                    return;
                }
            }else{
                if (configs.global().get<bool>("cookie::stop")){
                    views::info::render(conn, templates.invoke("misc").toggle("cookie_is_stopped").build_destory().c_str());
                    return;
                }
                else{
                    username = cc_random_username();
                    ssid = cck_create_ssid(username);
                }
            }

            if (admin_ctrl) username = "Admin";
            string ass = is_assist(conn);
            if(!ass.empty()) username = ass.substr(0, 9);

            if(admin_ctrl)
                tmpcontent = cc_htmlify(tmpcontent, false);
            else
                tmpcontent = cc_htmlify(tmpcontent, configs.global().get<bool>("user::strict"));

            string tmpname(p_title);
            cc_clean_string(tmpname);
            strncpy(p_title, tmpname.c_str(), 64);

            const char * cip = cc_get_client_ip(conn);
            logLog("New thread %s (%s) posted by %s as '%s' dumped to '%s'", p_title, p_image, cip, p_options, dump_name.c_str());

            if (post_to != 0) {
                int id = post_to;
                _Thread reply_to = _Thread::read(id);

                if(reply_to.error)
                {
                    views::info::render(conn, templates.invoke("misc").toggle("invalid_thread_no").build_destory().c_str());
                    return;
                }

                //if(t->state & TOOMANY_REPLIES){
                if(reply_to.state & _Thread::FULL)
                {
                    views::info::render(conn, templates.invoke("misc").toggle("cannot_reply_toomany") \
                            .var("MAX", configs.global().get<int>("user::max_replies")).build_destory().c_str());
                    return;
                }

                //if(t->childThread)
                if(reply_to.child)
                {
                    //if(tc->childCount >= configs.global().get<int>("user::max_replies") - 1){
                    if(reply_to.children_count >= (uint32_t)configs.global().get<int>("user::max_replies") - 1)
                    {
                        //changeState(t, TOOMANY_REPLIES, true);
                        reply_to.state &= _Thread::FULL;
                        //unq_write_thread(pDb, t->threadID, t, true);
                        _Thread::write(reply_to);
                    }
                }

                //if(t->state & LOCKED_THREAD)
                if(reply_to.state & _Thread::LOCK)
                    views::info::render(conn, templates.invoke("misc").toggle("cannot_reply_locked").build_destory().c_str());
                else
                {
                    //unq_new_reply(pDb, id, tmpcontent.c_str(), p_title, cip, username.c_str(), p_image, sage);
                    _Thread reply = _Thread::make();
                    reply.content = tmpcontent;
                    reply.title = tmpname;
                    reply.author = username;
                    reply.ip = std::string(cip);
                    reply.img = std::string(p_image);
                    if(sage) reply.state &= _Thread::SAGE;

                    _Thread::append_child(reply_to, reply);
                    mg_printf(conn, "HTTP/1.1 302 Moved Temporarily\r\nLocation: /success/%s/to/%d\r\n\r\n", ssid.c_str(), id);
                }
            }
            else
            {
                _Thread new_thread = _Thread::make();
                new_thread.content = tmpcontent;
                new_thread.title = tmpname;
                new_thread.author = username;
                new_thread.ip = std::string(cip);
                new_thread.img = std::string(p_image);
                if(sage) new_thread.state += _Thread::SAGE;
                    
                _Thread::insert_after(_Thread::read(0), new_thread);
                //unq_new_thread(pDb, tmpcontent.c_str(), p_title, cip, username.c_str(), p_image, sage);
                mg_printf(conn, "HTTP/1.1 302 Moved Temporarily\r\nLocation: /success/%s/to/0\r\n\r\n", ssid.c_str());
            }
        }
    }
}
