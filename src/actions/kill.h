#ifndef CHANN_ACTIONS_KILL_HEADER_INCLUDED
#define CHANN_ACTIONS_KILL_HEADER_INCLUDED
#include <string>
#include <deque>
#include <unordered_set>

extern "C" {
#include "../../lib/unqlite/unqlite.h"
#include "../../lib/mongoose/mongoose.h"
}

#include "../general.h"
#include "../helper.h"
#include "../config.h"
#include "../tags.h"

#include "../views/info.h"

namespace actions{
    namespace kill{
        bool call(mg_connection *conn, int tid, bool admin = false);
    }
}

#endif