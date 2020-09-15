/*
 * Xournal++
 *
 * Xournal main entry, commandline parser
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include <config.h>

#include "XournalType.h"
#include "filesystem.h"

class GladeSearchpath;
class Control;

namespace XournalMain {
auto run(int argc, char** argv) -> int;
}
