//
// Created by julius on 25.04.20.
//

#pragma once

#include <functional>
#include <memory>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graph_traits.hpp>

#include "ActionListener.h"

struct VertexData {
    std::shared_ptr<ActionListener> listener;
    std::function<bool(Action)> filter;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexData> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor VertexDescriptor;
typedef boost::graph_traits<Graph>::edge_descriptor EdgeDescriptor;

class BfsVisitor: public boost::default_bfs_visitor {
public:
    BfsVisitor(std::function<void(VertexDescriptor, Graph)> visit);
    auto examine_vertex(VertexDescriptor v, const Graph& g) const -> void;
    std::function<void(VertexDescriptor, Graph)> visit;
};

class DfsVisitor: public boost::default_dfs_visitor {
public:
    auto back_edge(EdgeDescriptor e, const Graph& g) -> void;
    bool hasCycle = false;
};

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
    auto waitFor(const std::shared_ptr<ActionListener>& listener, const std::shared_ptr<ActionListener>& dependency)
            -> void;
    auto registerListener(std::shared_ptr<ActionListener> listener, std::function<bool(Action)> filter) -> void;
    // auto unregisterListener(const std::shared_ptr<ActionListener>& listener) -> void;

private:
    Dispatcher() = default;

    Graph listenerGraph{};
    std::vector<VertexDescriptor> componentStartNodes{};

private:
    auto findVertex(const std::shared_ptr<ActionListener>& listener) -> VertexDescriptor;
    auto recalculateParts() -> void;
};
