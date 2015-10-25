#ifndef CCHAN_VIEWS_GALLERY_HEADER
#define CCHAN_VIEWS_GALLERY_HEADER

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

namespace views{
	namespace gallery{
		void render(mg_connection* conn);
	}

}

#endif