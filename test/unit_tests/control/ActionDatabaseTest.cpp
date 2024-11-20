/*
 * Xournal++
 *
 * This file is part of the Xournal UnitTests
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <string>

#include <config-test.h>
#include <gtest/gtest.h>
#include <gtk/gtk.h>

#include "control/actions/ActionProperties.h"
#include "gui/menus/menubar/Menubar.h"
#include "util/EnumIndexedArray.h"
#include "util/raii/GObjectSPtr.h"
#include "util/raii/GVariantSPtr.h"
#include "util/safe_casts.h"  // for to_underlying Todo(cpp20) replace with <utility>

#include "filesystem.h"

/**
 * Test if the GActions referred to in MENU_XML_FILE have a counterpart in enums/Action.h, and checks if the (optional)
 * action target value has the same type as set in control/actions/ActionProperties.h
 */

constexpr auto MENU_XML_FILE = "mainmenubar.ui";
constexpr auto MENU_ID = "menubar";

namespace {
template <Action a, class U = void>
struct helper {
    static void setup(EnumIndexedArray<const GVariantType*, Action>& expectedTypes) { expectedTypes[a] = nullptr; };
};
template <Action a>
struct helper<a, std::void_t<typename ActionProperties<a>::parameter_type>> {
    static void setup(EnumIndexedArray<const GVariantType*, Action>& expectedTypes) {
        expectedTypes[a] = gVariantType<typename ActionProperties<a>::parameter_type>();
    }
};

template <size_t... As>
static auto setupImpl(std::index_sequence<As...>) {
    EnumIndexedArray<const GVariantType*, Action> expectedTypes;
    ((helper<static_cast<Action>(As)>::setup(expectedTypes)), ...);
    return expectedTypes;
}

static const auto expectedTypes = setupImpl(std::make_index_sequence<xoj::to_underlying(Action::ENUMERATOR_COUNT)>());

void exploreMenu(GMenuModel* m, int lvl = 1) {
    int n = g_menu_model_get_n_items(m);
    for (int i = 0; i < n; i++) {
        xoj::util::GVariantSPtr val(g_menu_model_get_item_attribute_value(m, i, "action", nullptr), xoj::util::adopt);

        if (val) {
            std::string value = g_variant_get_string(val.get(), nullptr);
            auto pos = value.find('.');
            EXPECT_TRUE(pos != std::string::npos);
            EXPECT_TRUE(pos != 0);
            std::string prefix = value.substr(0, pos);
            std::string action = value.substr(pos + 1);
            std::cout << std::setw(2 * lvl) << lvl << "  " << value << std::endl;
            Action a = Action_fromString(action);

            xoj::util::GVariantSPtr target(g_menu_model_get_item_attribute_value(m, i, "target", nullptr),
                                           xoj::util::adopt);
            if (target) {
                EXPECT_TRUE(g_variant_type_equal(g_variant_get_type(target.get()), expectedTypes[a]));
            } else {
                EXPECT_TRUE(expectedTypes[a] == nullptr);
            }
        }

        {
            xoj::util::GObjectSPtr<GMenuLinkIter> it(g_menu_model_iterate_item_links(m, i), xoj::util::adopt);
            while (g_menu_link_iter_next(it.get())) {
                std::cout << std::setw(2 * lvl) << lvl << "  " << g_menu_link_iter_get_name(it.get()) << std::endl;
                xoj::util::GObjectSPtr<GMenuModel> subm(g_menu_link_iter_get_value(it.get()), xoj::util::adopt);
                exploreMenu(subm.get(), lvl + 1);
            }
        }
    }
}
};  // namespace

TEST(ActionDatabaseTest, testActionTargetMatch) {
    xoj::util::GObjectSPtr<GtkBuilder> builder(gtk_builder_new(), xoj::util::adopt);

    GError* error = nullptr;
    auto filepath = fs::path(GET_UI_FOLDER) / MENU_XML_FILE;

    if (!gtk_builder_add_from_file(builder.get(), filepath.u8string().c_str(), &error)) {
        std::string msg = "Error loading menubar XML file ";
        msg += filepath.u8string();

        if (error != nullptr) {
            msg += "\n";
            msg += error->message;
            g_error_free(error);
        }
        FAIL() << msg;
        return;
    }

    GMenuModel* menu = G_MENU_MODEL(gtk_builder_get_object(builder.get(), MENU_ID));
    ASSERT_TRUE(menu);

    exploreMenu(menu);
}
