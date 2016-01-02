#ifndef CCHAN_VIEWS_EXPAND_HEADER
#define CCHAN_VIEWS_EXPAND_HEADER

#include <string>
#include <time.h>
#include <map>
#include <queue>

extern "C" {
#include "../../../lib/mongoose/mongoose.h"
}

#include "../../helper.h"
#include "../../config.h"
#include "../../tags.h"
#include "../../data.h"
#include "../single.h"
#include "../info.h"

extern TemplateManager templates;

namespace views{
	namespace expand{
		void render(mg_connection* conn, int id, bool reverse);
	}
}

#endif
