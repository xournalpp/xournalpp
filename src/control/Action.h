//
// Created by julius on 25.04.20.
//

#pragma once

#include <variant>

#include <model/softstorage/Viewport.h>

using Action = std::variant<Scroll, Resize, Scale>;