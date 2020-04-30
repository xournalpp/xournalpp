//
// Created by julius on 25.04.20.
//

#include "Dispatcher.h"

#include <utility>

#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/topological_sort.hpp>

typedef

        auto
        Dispatcher::dispatch(const Action& action) -> void {
    // TODO make parallel
    for (auto& initialNode: componentStartNodes)
        boost::breadth_first_search(
                listenerGraph, initialNode,
                boost::visitor(BfsVisitor{[&](auto v, auto g) { g[v].listener->onAction(action); }}));
}
auto Dispatcher::registerListener(std::shared_ptr<ActionListener> listener, std::function<bool(Action)> filter)
        -> void {
    VertexData vertexData{std::move(listener), std::move(filter)};
    boost::add_vertex(vertexData, listenerGraph);
    recalculateParts();
}
auto Dispatcher::waitFor(const std::shared_ptr<ActionListener>& listener,
                         const std::shared_ptr<ActionListener>& dependency) -> void {
    if (listenerGraph.vertex_set().empty())
        return;
    auto listenerV = findVertex(listener);
    auto dependencyV = findVertex(dependency);
    boost::add_edge(dependencyV, listenerV, listenerGraph);
    recalculateParts();
}

auto Dispatcher::findVertex(const std::shared_ptr<ActionListener>& listener) -> VertexDescriptor {
    for (auto v: listenerGraph.vertex_set()) {
        if (listenerGraph[v].listener == listener)
            return v;
    }
    throw std::invalid_argument("Specified listener not found");
}
auto Dispatcher::recalculateParts() -> void {
    DfsVisitor visitor{};
    boost::depth_first_search(listenerGraph, boost::visitor(visitor));
    if (visitor.hasCycle)
        throw std::invalid_argument("Dependency cycle detected, must be removed");
    // Partition graph into connected components
    std::map<VertexDescriptor, int> componentMap{};
    int components = boost::connected_components(listenerGraph, boost::make_assoc_property_map(componentMap));
    for (int i = 0; i < components; i++) {
    }
    // Sort graph topological to traverse dependencies
    std::vector<VertexDescriptor> topologicalSorted{};
    boost::topological_sort(listenerGraph, std::back_inserter(topologicalSorted));
    // Save start nodes of each component
    std::vector<int> traversedComponents{};
    for (auto rit = topologicalSorted.rbegin(); rit != topologicalSorted.rend(); ++rit) {
        auto component = componentMap[*rit];
        if (!std::binary_search(traversedComponents.begin(), traversedComponents.end(), component)) {
            traversedComponents.push_back(component);
            componentStartNodes.push_back(*rit);
        }
    }
}

auto BfsVisitor::examine_vertex(VertexDescriptor v, const Graph& g) const -> void { visit(v, g); }
BfsVisitor::BfsVisitor(std::function<void(VertexDescriptor, Graph)> visit): visit(std::move(visit)) {}
auto DfsVisitor::back_edge(EdgeDescriptor e, const Graph& g) -> void { hasCycle = true; }
