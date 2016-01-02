#ifndef CHANN_VIEWS_SUCCESS_HEADER_INCLUDED
#define CHANN_VIEWS_SUCCESS_HEADER_INCLUDED

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
#include "info.h"

extern TemplateManager templates;

namespace views{
	namespace success{
		void render(mg_connection* conn, std::string cookie, int id);
	}
}

#endif
