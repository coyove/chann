#ifndef CCHAN_VIEWS_SUCCESS_HEADER
#define CCHAN_VIEWS_SUCCESS_HEADER

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
#include "info.h"

extern TemplateManager templates;
extern unqlite *pDb;

namespace views{
	namespace success{
		void render(mg_connection* conn, std::string cookie, int id);
	}
}

#endif