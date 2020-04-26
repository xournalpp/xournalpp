//
// Created by julius on 26.04.20.
//

#pragma once


#include <model/StorageEvent.h>
template <class E>
class ControllerView {
public:
    virtual ~ControllerView();

public:
    virtual auto eventCallback(const E& event) -> void = 0;
};
