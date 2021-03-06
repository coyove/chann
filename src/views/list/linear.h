#ifndef CCHAN_VIEWS_LIST_HEADER
#define CCHAN_VIEWS_LIST_HEADER

#include <string>
#include <time.h>
#include <map>
#include <queue>

extern "C" {
#include "../../../lib/unqlite/unqlite.h"
#include "../../../lib/mongoose/mongoose.h"
}

#include "../../general.h"
#include "../../helper.h"
#include "../../config.h"
#include "../../tags.h"
#include "../single.h"

extern TemplateManager templates;
extern unqlite *pDb;

#define LINEAR_ADMIN_VIEW 	1
#define LINEAR_IP_BASED		2
#define LINEAR_ID_BASED		4

namespace views{
	namespace linear{
		void render(mg_connection* conn, 
		    bool ip_based = false,
		    bool id_based = false,
		    const char* needle = "");
	}

}

#endif