//
// Created by julius on 26.04.20.
//

#pragma once


#include <memory>

#include <control/tools/EditSelection.h>
#include <model/Storage.h>

class SelectionEvent {};

class Selections: public Storage<SelectionEvent> {
public:
    auto getSelection() -> std::shared_ptr<EditSelection>;

    auto onAction(const Action& action) -> void override;
};
