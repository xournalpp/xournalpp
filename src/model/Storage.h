//
// Created by julius on 26.04.20.
//

#pragma once

#include <functional>
#include <memory>

#include <control/ActionListener.h>

template <class E>
class Storage: public ActionListener {
public:
    virtual ~Storage();

public:
    auto registerListener(std::function<void(E)> callback) -> int;
    auto unregisterListener(int index) -> void;

protected:
    auto emit(const E& event) -> void;

private:
    std::vector<std::function<void(E)>> listeners{};
};
