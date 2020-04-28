//
// Created by julius on 25.04.20.
//

#include "Dispatcher.h"

#include <utility>

#include <boost/graph/breadth_first_search.hpp>

auto Dispatcher::dispatch(const Action& action) -> void {
    boost::breadth_first_search(listenerGraph, initialNode,
                                boost::visitor(Visitor{[&](auto v, auto g) { g[v].listener->onAction(action); }}));
}
auto Dispatcher::registerListener(std::shared_ptr<ActionListener> listener, std::function<bool(Action)> filter)
        -> void {
    VertexData vertexData{std::move(listener), std::move(filter)};
    if (listenerGraph.vertex_set().empty()) {
        auto descriptor = boost::add_vertex(vertexData, listenerGraph);
        initialNode = descriptor;
    } else
        boost::add_vertex(vertexData, listenerGraph);
}
auto Dispatcher::waitFor(const std::shared_ptr<ActionListener>& listener,
                         const std::shared_ptr<ActionListener>& dependency) -> void {
    if (listenerGraph.vertex_set().empty())
        return;
    auto resultMap = findVertices(std::vector<std::shared_ptr<ActionListener>>{listener, dependency});
    if (resultMap.size() != 2)
        return;
    boost::add_edge(resultMap[1], resultMap[0], listenerGraph);
}

auto Dispatcher::findVertices(const std::vector<std::shared_ptr<ActionListener>>& listeners)
        -> std::vector<VertexDescriptor> {
    std::vector<VertexDescriptor> result{};
    boost::breadth_first_search(listenerGraph, initialNode, boost::visitor(Visitor{[&](auto v, auto g) {
                                    for (int i = 0; i < listeners.size(); i++)
                                        if (g[v].listener == listeners[i])
                                            result[i] = v;
                                }}));
    return result;
}

auto Visitor::examine_vertex(VertexDescriptor v, const Graph& g) const -> void { visit(v, g); }
Visitor::Visitor(std::function<void(VertexDescriptor, Graph)> visit): visit(std::move(visit)) {}
