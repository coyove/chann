#ifndef CCHAN_VIEWS_LIST_HEADER
#define CCHAN_VIEWS_LIST_HEADER

#include <string>
#include <time.h>

extern "C" {
#include "../../../lib/unqlite/unqlite.h"
#include "../../../lib/mongoose/mongoose.h"
}

#include "../../general.h"
#include "../../helper.h"
#include "../../config.h"
#include "../../tags.h"


extern TemplateManager templates;
extern unqlite *pDb;

ConfigManager configs;

#endif