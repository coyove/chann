#ifndef CHANN_ACTIONS_POST_HEADER_INCLUDED
#define CHANN_ACTIONS_POST_HEADER_INCLUDED
#include <string>
#include <deque>
#include <unordered_set>
#include <memory>

extern "C" {
#include "../../lib/unqlite/unqlite.h"
#include "../../lib/mongoose/mongoose.h"
}

#include "../general.h"
#include "../helper.h"
#include "../config.h"
#include "../tags.h"

#include "../views/info.h"

#ifdef GOOGLE_RECAPTCHA
#include <curl/curl.h>
#endif

namespace actions{
    namespace post{
        void call(mg_connection *conn, int post_to);
    }
}

#endif