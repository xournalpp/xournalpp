#include <array>
#include <clocale>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <string>

#include <libintl.h>

#include "control/Control.h"
#include "control/XournalMain.h"
#include "control/settings/Settings.h"
#include "gui/GladeSearchpath.h"
#include "gui/MainWindow.h"
#include "gui/dialog/SettingsDialog.h"
#include "util/PathUtil.h"

#include "ScribbleGtkTest.h"
#include "config-test.h"
#include "config.h"

namespace {
auto environment(const char* name) -> std::optional<std::string> {
    const char* value = g_getenv(name);
    return value ? std::optional<std::string>(value) : std::nullopt;
}

void restoreEnvironment(const char* name, const std::optional<std::string>& value) {
#ifdef _WIN32
    // Use the same CRT environment API as Settings::load on Windows.
    _putenv_s(name, value ? value->c_str() : "");
#else
    if (value) {
        setenv(name, value->c_str(), 1);
    } else {
        unsetenv(name);
    }
#endif
}

GtkWidget* findWidget(GtkWidget* parent, const char* id) {
    const char* name = gtk_buildable_get_name(GTK_BUILDABLE(parent));
    if (name && std::strcmp(name, id) == 0) {
        return parent;
    }
    if (!GTK_IS_CONTAINER(parent)) {
        return nullptr;
    }
    auto* children = gtk_container_get_children(GTK_CONTAINER(parent));
    GtkWidget* found = nullptr;
    for (auto* child = children; child && !found; child = child->next) {
        found = findWidget(GTK_WIDGET(child->data), id);
    }
    g_list_free(children);
    return found;
}

class ScribbleTranslationTest: public ScribbleGtkTest {
    std::optional<std::string> originalLanguage;
    std::optional<std::string> originalDomainDirectory;
    std::optional<std::string> originalLocaleEnvironment;
    std::optional<std::string> originalLang;
    std::string originalLocale;
    bool started = false;
    std::string language;

    void SetUp() override {
        originalLanguage = environment("LANGUAGE");
        originalDomainDirectory = environment("TEXTDOMAINDIR");
        originalLocaleEnvironment = environment("LC_ALL");
        originalLang = environment("LANG");
        originalLocale = setlocale(LC_ALL, nullptr);
        const auto requested = environment("XOURNALPP_TEST_LANGUAGE").value_or("pt_BR");
        ASSERT_FALSE(requested.empty());
        ASSERT_EQ(requested.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"),
                  std::string::npos);
        language = requested;
        restoreEnvironment("LANGUAGE", requested);
        restoreEnvironment("TEXTDOMAINDIR", environment("XOURNALPP_TEST_LOCALE_DIR").value_or(GET_TEST_LOCALE_FOLDER));
        const char* testLocale = nullptr;
        for (const char* candidate: {language == "pt_BR" ? "pt_BR.UTF-8" : "en_US.UTF-8", "en_US.UTF-8", "C.UTF-8"}) {
            if (setlocale(LC_ALL, candidate)) {
                testLocale = candidate;
                break;
            }
        }
        ASSERT_NE(testLocale, nullptr);
        restoreEnvironment("LC_ALL", testLocale);
        restoreEnvironment("LANG", testLocale);
        // Refresh gettext through its public locale API between test fixtures.
        setlocale(LC_ALL, "C");
        ASSERT_NE(setlocale(LC_ALL, ""), nullptr);
        started = true;
        ScribbleGtkTest::SetUp();
    }

    void TearDown() override {
        if (started) {
            ScribbleGtkTest::TearDown();
        }
        restoreEnvironment("LANGUAGE", originalLanguage);
        restoreEnvironment("TEXTDOMAINDIR", originalDomainDirectory);
        restoreEnvironment("LC_ALL", originalLocaleEnvironment);
        restoreEnvironment("LANG", originalLang);
        setlocale(LC_ALL, "C");
        setlocale(LC_ALL, originalLocale.c_str());
        XournalMain::initLocalisation();
    }

    void runTest(GtkApplication*) override {
        XournalMain::initLocalisation();
        // Settings::load selects the application language, overriding LANGUAGE.
        Settings configured(Util::getConfigFile("settings.xml"));
        configured.load();
        configured.setPreferredLocale(language);
        configured.save();
        auto& control = createControl();
        setlocale(LC_ALL, "C");
        ASSERT_NE(setlocale(LC_ALL, ""), nullptr);
        XournalMain::initLocalisation();
        setlocale(LC_NUMERIC, "C");
        createWindow();
        control.enableAutosave(false);
        auto* settings = control.getSettings();
        settings->latexSettings.globalTemplatePath = fs::path(PROJECT_SOURCE_DIR) / "resources/default_template.tex";
        SCOPED_TRACE(testing::Message() << "LANGUAGE=" << g_getenv("LANGUAGE")
                                        << " locale=" << setlocale(LC_ALL, nullptr));
        SettingsDialog dialog(&resources, settings, &control, {}, []() {});
        enum class TextKind { LABEL, BUTTON, TOOLTIP, EXPANDER };
        struct Message {
            const char* widget;
            TextKind kind;
            const char* source;
        };
        const std::array<Message, 9> messages{{
                {"labelScribbleToErase", TextKind::LABEL, "Scribble to Erase"},
                {"cbScribbleToEraseEnabled", TextKind::BUTTON, "Enable scribble to erase"},
                {"cbScribbleToEraseEnabled", TextKind::TOOLTIP,
                 "Erase entire strokes on the current layer by scribbling over them with the Pen tool. Undo restores "
                 "the erased strokes."},
                {"exScribbleAdvanced", TextKind::EXPANDER, "Advanced settings"},
                {"lbScribbleSensitivity", TextKind::LABEL, "Recognition sensitivity (0-100)"},
                {"lbScribbleSensitivityHelp", TextKind::LABEL,
                 "Higher values accept less regular scribbles, but can erase ordinary drawing by mistake. Default: "
                 "50."},
                {"lbScribbleMinimumPasses", TextKind::LABEL, "Minimum passes"},
                {"lbScribbleMinimumPassesHelp", TextKind::LABEL,
                 "Each movement in one direction counts as one pass. Lower values need fewer movements, but can erase "
                 "ordinary drawing by mistake. Moving across several targets may need extra passes. Default: 4."},
                {"btScribbleDefaults", TextKind::BUTTON, "Restore defaults"},
        }};
        const bool sourceEnglish = language == "en" || language == "en_GB";
        for (const auto& message: messages) {
            SCOPED_TRACE(message.source);
            auto* widget = findWidget(GTK_WIDGET(dialog.getWindow()), message.widget);
            ASSERT_NE(widget, nullptr);
            std::string actual;
            switch (message.kind) {
                case TextKind::LABEL:
                    ASSERT_TRUE(GTK_IS_LABEL(widget));
                    actual = gtk_label_get_text(GTK_LABEL(widget));
                    break;
                case TextKind::BUTTON:
                    ASSERT_TRUE(GTK_IS_BUTTON(widget));
                    actual = gtk_button_get_label(GTK_BUTTON(widget));
                    break;
                case TextKind::TOOLTIP: {
                    gchar* tooltip = gtk_widget_get_tooltip_text(widget);
                    ASSERT_NE(tooltip, nullptr);
                    actual = tooltip;
                    g_free(tooltip);
                    break;
                }
                case TextKind::EXPANDER:
                    ASSERT_TRUE(GTK_IS_EXPANDER(widget));
                    actual = gtk_expander_get_label(GTK_EXPANDER(widget));
                    break;
            }
            const char* expected = sourceEnglish ? message.source : dgettext(GETTEXT_PACKAGE, message.source);
            EXPECT_STREQ(actual.c_str(), expected);
            EXPECT_TRUE(g_utf8_validate(actual.c_str(), -1, nullptr));
            if (language == "pt_BR" && !environment("XOURNALPP_TEST_LOCALE_DIR")) {
                EXPECT_STRNE(expected, message.source) << "Missing translation or wrong application language";
            }
        }
        RecordProperty("language", language);
        RecordProperty("message_count", static_cast<int>(messages.size()));
    }
};
}  // namespace

TEST_F(ScribbleTranslationTest, usesApplicationLanguageForLabelsAndTooltip) {}
