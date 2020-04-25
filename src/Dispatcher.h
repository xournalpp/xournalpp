//
// Created by julius on 25.04.20.
//

#pragma once
#include "Action.h"

class Dispatcher {
public:
    static Dispatcher& getInstance() {
        static Dispatcher dispatcher;
        return dispatcher;
    }

    Dispatcher(Dispatcher const&) = delete;
    void operator=(Dispatcher const&) = delete;

public:
    auto dispatch(Action action) -> void;
    auto registerStorage(Storage storage, std::function<bool(Action)> filter) -> void;

private:
    Dispatcher(){};
};
