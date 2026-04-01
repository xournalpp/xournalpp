/*
 * Xournal++
 *
 * A helper class to statically assert the action namespace is correct
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string_view>

#include "gtk/gtk.h"

class MainWindow;

/**
 * @brief Returns true if the provided GAction namespace matches the provided type
 * (i.e. "win" for GtkApplicationWindow and "app" for GtkApplication)
 */
template <class action_map_type>
constexpr bool is_action_namespace_match(const char* nspace) {
    return false;
}

template <>
constexpr bool is_action_namespace_match<GtkWindow*>(const char* nspace) {
    return std::string_view(nspace) == std::string_view("win.");
}

template <>
constexpr bool is_action_namespace_match<GtkApplicationWindow*>(const char* nspace) {
    return std::string_view(nspace) == std::string_view("win.");
}

template <>
constexpr bool is_action_namespace_match<MainWindow*>(const char* nspace) {
    return std::string_view(nspace) == std::string_view("win.");
}

template <>
constexpr bool is_action_namespace_match<GtkApplication*>(const char* nspace) {
    return std::string_view(nspace) == std::string_view("app.");
}

template <>
constexpr bool is_action_namespace_match<GApplication*>(const char* nspace) {
    return std::string_view(nspace) == std::string_view("app");
}
