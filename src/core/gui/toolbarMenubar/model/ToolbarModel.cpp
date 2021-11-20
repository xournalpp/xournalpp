#include "ToolbarModel.h"

#include <fstream>

#include "util/XojMsgBox.h"
#include "util/i18n.h"

#include "ToolbarData.h"
#include "filesystem.h"

using std::string;

ToolbarModel::ToolbarModel() = default;

ToolbarModel::~ToolbarModel() {
    for (ToolbarData* data: this->toolbars) { delete data; }
    this->toolbars.clear();
}

auto ToolbarModel::getToolbars() -> std::vector<ToolbarData*>* { return &this->toolbars; }

void ToolbarModel::parseGroup(GKeyFile* config, const char* group, bool predefined) {
    auto* data = new ToolbarData(predefined);

    data->name = (predefined ? "predef_" : "custom_");
    data->id = group;

    data->load(config, group);

    add(data);
}

void ToolbarModel::remove(ToolbarData* data) {
    for (size_t i = 0; i < this->toolbars.size(); i++) {
        if (this->toolbars[i] == data) {
            this->toolbars.erase(this->toolbars.begin() + i);
            break;
        }
    }
}

void ToolbarModel::add(ToolbarData* data) { this->toolbars.push_back(data); }

auto ToolbarModel::parse(fs::path const& filepath, bool predefined) -> bool {
    GKeyFile* config = g_key_file_new();
    g_key_file_set_list_separator(config, ',');
    if (!g_key_file_load_from_file(config, filepath.u8string().c_str(), G_KEY_FILE_NONE, nullptr)) {
        g_key_file_free(config);
        return false;
    }

    gsize length = 0;
    gchar** groups = g_key_file_get_groups(config, &length);

    for (gsize i = 0; i < length; i++) { parseGroup(config, groups[i], predefined); }

    g_strfreev(groups);
    g_key_file_free(config);
    return true;
}

void ToolbarModel::initCopyNameId(ToolbarData* data) {
    for (int i = 0; i < 100; i++) {
        string id = data->getId() + " Copy";

        if (i != 0) {
            id += " ";
            id += std::to_string(i);
        }

        if (!existsId(id)) {
            if (i != 0) {
                string filename = data->getName();
                filename += " ";
                filename += _("Copy");
                filename += " ";
                filename += std::to_string(i);

                data->setName(filename);
            } else {
                data->setName(data->getName() + " " + _("Copy"));
            }
            data->setId(id);
            break;
        }
    }
}

auto ToolbarModel::existsId(const string& id) -> bool {
    for (ToolbarData* data: this->toolbars) {
        if (data->getId() == id) {
            return true;
        }
    }
    return false;
}


const char* TOOLBAR_INI_HEADER =
        "# Xournal++ Toolbar configuration\n"
        "# Here you can customize the Toolbars\n"
        " Delete this file to generate a new config file with default values\n"
        "\n"
        " Available buttons:\n"
        " File: NEW,SAVE,OPEN,PRINT,SAVEPDF\n"
        "\n"
        " Edit: CUT,COPY,PASTE,SEARCH,UNDO,REDO,INSERT_NEW_PAGE\n"
        "\n"
        " Navigation: GOTO_FIRST,GOTO_BACK,GOTO_NEXT,GOTO_LAST,GOTO_NEXT_ANNOTATED_PAGE\n"
        "\n"
        " Zoom: ZOOM_OUT,ZOOM_IN,ZOOM_FIT,ZOOM_100,FULLSCREEN,PAIRED_PAGES\n"
        "\n"
        " Color: "
        "COLOR(0),COLOR(1),COLOR(2),COLOR(3),COLOR(4),COLOR(5),COLOR(6),COLOR(7),COLOR(8),COLOR(9),COLOR(10),COLOR_"
        "SELECT\n"
        "  Notice: The colors reference the respective color in the palette.gpl file.\n"
        "          For backwards compatibility the hex representation, e.g. COLOR(0xff8000), is still permitted. "
        "However, the hex value\n"
        "          will be IGNORED in favour of the \"next\""
        " palette color. They will be automatically migrated when the user customizes\n"
        "          the respective toolbar configuration.\n"
        "\n"
        " Tools: ERASER,PEN,HIGHLIGHTER,IMAGE,TEXT,IMAGE,SELECT,SELECT_REGION,SELECT_RECTANGLE,VERTICAL_SPACE,HAND\n"
        "  Notice: ERASER also has a drop down menu to select the eraser type, SELECT are all selection tools, with "
        "drop down menu\n"
        "\n"
        " Tool settings: SHAPE_RECOGNIZER,RULER,FINE,MEDIUM,THICK,SELECT_FONT\n"
        "\n"
        " Components: PAGE_SPIN,ZOOM_SLIDER,LAYER\n"
        "  PAGE_SPIN: The page spiner, incl. current page label\n"
        "  ZOOM_SLIDER: The zoom slider\n"
        "  LAYER: The layer dropdown menu\n"
        "\n";

void ToolbarModel::save(fs::path const& filepath) {
    GKeyFile* config = g_key_file_new();
    g_key_file_set_list_separator(config, ',');

    g_key_file_set_comment(config, nullptr, nullptr, TOOLBAR_INI_HEADER, nullptr);

    for (ToolbarData* data: this->toolbars) {
        if (!data->isPredefined()) {
            data->saveToKeyFile(config);
        }
    }

    gsize len = 0;
    char* data = g_key_file_to_data(config, &len, nullptr);

    GError* error = nullptr;
    if (!g_file_set_contents(filepath.u8string().c_str(), data, len, &error)) {
        XojMsgBox::showErrorToUser(nullptr, error->message);
        g_error_free(error);
    }

    g_free(data);
}
