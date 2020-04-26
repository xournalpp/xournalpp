//
// Created by julius on 26.04.20.
//

#pragma once

#include "Action.h"

class ActionListener {
public:
    virtual ~ActionListener();
    virtual auto onAction(const Action& action) -> void = 0;
};
