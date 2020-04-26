//
// Created by julius on 26.04.20.
//

#pragma once

#include <functional>
#include <memory>

#include <control/ActionListener.h>
#include <gui/ControllerView.h>

template <class E>
class Storage: public ActionListener {
public:
    virtual ~Storage();

public:
    auto registerListener(const ControllerView<E>& listener, std::function<bool(E)> filter) -> void;
    auto unregisterListener(const ControllerView<E>& listener) -> void;

protected:
    auto emit(const E& event) -> void;

private:
    std::vector<std::weak_ptr<ControllerView<E>>> listeners{};
};
