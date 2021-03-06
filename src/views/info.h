#ifndef CCHAN_VIEWS_INFO_HEADER
#define CCHAN_VIEWS_INFO_HEADER

#include <string>
#include <time.h>
#include <map>
#include <queue>

extern "C" {
#include "../../lib/unqlite/unqlite.h"
#include "../../lib/mongoose/mongoose.h"
}

#include "../general.h"
#include "../helper.h"
#include "../config.h"
#include "../tags.h"

extern TemplateManager templates;
extern unqlite *pDb;

namespace views{
	namespace info{
		void render(mg_connection* conn, const char* msg);
	}
}

#endif