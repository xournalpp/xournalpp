//
// Created by julius on 25.04.20.
//

#pragma once

#include <functional>

#include "ActionListener.h"

class Dispatcher {
public:
    static Dispatcher& getMainStage() {
        static Dispatcher dispatcher;
        return dispatcher;
    }

    static Dispatcher& getPreStage() {
        static Dispatcher dispatcher;
        return dispatcher;
    }

    Dispatcher(Dispatcher const&) = delete;
    void operator=(Dispatcher const&) = delete;

public:
    auto dispatch(const Action& action) -> void;
    auto registerListener(const ActionListener& storage, std::function<bool(Action)> filter) -> void;

private:
    Dispatcher(){};
};
