#ifndef CCHAN_VIEWS_GALLERY_HEADER
#define CCHAN_VIEWS_GALLERY_HEADER

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

extern TemplateManager templates;

namespace views{
	namespace gallery{
		void render(mg_connection* conn);
	}

}

#endif
