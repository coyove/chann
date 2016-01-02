#ifndef CCHAN_VIEWS_SINGLE_HEADER
#define CCHAN_VIEWS_SINGLE_HEADER

#include <string>
#include <time.h>
#include <map>
#include <queue>

extern "C" {
#include "../../lib/mongoose/mongoose.h"
}

#include "../helper.h"
#include "../config.h"
#include "../tags.h"
#include "../data.h"

#define SEND_IS_REPLY           1
#define SEND_SHOW_REPLY_LINK    2
#define SEND_CUT_LONG_COMMENT   4
#define SEND_CUT_IMAGE          8
#define SEND_CUT_REPLY_COUNT    16

extern TemplateManager templates;

namespace views{

void each_thread(mg_connection* conn, struct Thread* r, char display_state, bool admin_view = false, 
    const char* iid = "",
    const char* uid = "");

void each_thread(mg_connection* conn, _Thread & r, char display_state, bool admin_view = false, 
    std::string iid = "",
    std::string uid = "");
}


#endif
