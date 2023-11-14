/*
 * Xournal++
 *
 * A page reference, should only allocated on the stack
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <memory>

class XojPage;

using PageRef = std::shared_ptr<XojPage>;
using ConstPageRef = std::shared_ptr<const XojPage>;
