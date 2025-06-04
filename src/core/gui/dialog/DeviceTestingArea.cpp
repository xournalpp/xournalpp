#include "DeviceTestingArea.h"

#include <bitset>
#include <sstream>

#include <config-git.h>
#include <config.h>
#include <zip.h>

#include "control/settings/Settings.h"
#include "gui/Builder.h"
#include "gui/dialog/XojSaveDlg.h"
#include "gui/inputdevices/InputContext.h"
#include "gui/inputdevices/InputEvents.h"
#include "gui/inputdevices/PrintEvent.h"
#include "model/Point.h"
#include "util/CircularBuffer.h"
#include "util/EnumIndexedArray.h"
#include "util/PathUtil.h"
#include "util/PopupWindowWrapper.h"
#include "util/VersionInfo.h"
#include "util/glib_casts.h"
#include "util/gtk4_helper.h"  // for gtk_box_append, ...
#include "util/i18n.h"
#include "util/raii/GtkWindowUPtr.h"

#include "DeviceClassConfigGui.h"
#include "DeviceListDialog.h"
#include "filesystem.h"

constexpr auto UI_FILE_NAME = "deviceTestingArea.glade";
constexpr auto UI_MAIN_ID = "mainBox";

static constexpr double PRESSURE_FACTOR = 20;
static constexpr size_t CONTACT_BUTTON_INDEX = 1;  ///< Corresponds to button == 1 in GdkEvent
static constexpr double PRESSURELESS_INDICATOR_HALF_SIZE = 3;

static constexpr size_t MAX_NB_BUTTONS = 4;
static constexpr std::bitset<MAX_NB_BUTTONS> CONSIDERED_FOR_TIP_EMULATION(0b1100);
static constexpr std::bitset<MAX_NB_BUTTONS> MOUSE_BUTTONS_MASK(0b1110);

static constexpr unsigned int IN_USE_RESET_DELAY = 100;  ///< in ms

struct Ev {
    Point point;
    std::bitset<MAX_NB_BUTTONS> pressedButtons;
    DeviceTestingArea::HandlerType type;
};


enum class DevCategory : size_t {
    MOUSE = 0,
    STYLUS = 1,
    ERASER = 2,
    TOUCHSCREEN = 3,
    ENUMERATOR_COUNT = 4  ///< Used as bound
};  /// Used as indices for buttonsStatus

struct DeviceTestingArea::Data {
    CircularBuffer<Ev> lastEvs{20};
    EnumIndexedArray<std::bitset<MAX_NB_BUTTONS>, DevCategory> buttonsStatus;
    size_t nbTouchSequences;
    std::stringstream log;            ///< all the events as received (after conversion) by the handlers
    std::stringstream ellipsizedLog;  ///< Same as log, but sequences of MOTION_EVENT's are ellipsized
    std::stringstream allGdkEvents;   ///< all the events, as emitted by GDK
    size_t nbOfConsecutiveMotionEvents = 0;
    GdkDevice* lastMotionDevice = nullptr;
    guint32 timeReference = 0;  ///< timestamp of DeviceTestingArea's creation
};

static void draw(GtkDrawingArea*, cairo_t* cr, int width, int height, gpointer lastEvs) {
    cairo_set_source_rgb(cr, 1., 1., 1.);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0., 0., 0.);
    cairo_set_line_width(cr, 2);

    // Draw a frame - using CSS seems not to work for GtkDrawingArea's
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
    cairo_line_to(cr, 0., 0.);
    cairo_line_to(cr, width, 0.);
    cairo_line_to(cr, width, height);
    cairo_line_to(cr, 0., height);
    cairo_close_path(cr);
    cairo_stroke(cr);

    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    for (auto&& e: *static_cast<CircularBuffer<Ev>*>(lastEvs)) {
        if (e.pressedButtons[CONTACT_BUTTON_INDEX] ||
            (e.type == DeviceTestingArea::MOUSE && (e.pressedButtons & MOUSE_BUTTONS_MASK).any())) {
            // Contact
            if (e.point.z == Point::NO_PRESSURE) {
                cairo_set_source_rgb(cr, 1., 0., 0.);
                cairo_move_to(cr, e.point.x, e.point.y);
                cairo_rel_move_to(cr, -PRESSURELESS_INDICATOR_HALF_SIZE, -PRESSURELESS_INDICATOR_HALF_SIZE);
                cairo_rel_line_to(cr, 2 * PRESSURELESS_INDICATOR_HALF_SIZE, 2 * PRESSURELESS_INDICATOR_HALF_SIZE);
                cairo_rel_move_to(cr, -2 * PRESSURELESS_INDICATOR_HALF_SIZE, 0);
                cairo_rel_line_to(cr, 2 * PRESSURELESS_INDICATOR_HALF_SIZE, -2 * PRESSURELESS_INDICATOR_HALF_SIZE);
            } else {
                cairo_set_source_rgb(cr, .4, 1., 0.);
                cairo_arc(cr, e.point.x, e.point.y, PRESSURE_FACTOR * e.point.z, 0, 2. * M_PI);
            }
        } else {
            // No contact: hovering
            cairo_set_source_rgb(cr, .5, .5, .5);
            cairo_move_to(cr, e.point.x, e.point.y);
            cairo_rel_line_to(cr, 0, 0);
        }
        cairo_stroke(cr);
    }
}

static void saveLog(const std::stringstream& fullLog, const std::stringstream& ellipsizedLog,
                    const std::stringstream& gdkeventslog, GtkWidget* parent, Settings* settings) {

    auto pathValidation = [](fs::path& p, const char* filterName) {
        Util::clearExtensions(p, ".zip");
        p += ".zip";
        return true;
    };

    auto callback = [fullLog = fullLog.str(), ellipsizedLog = ellipsizedLog.str(), gdkeventslog = gdkeventslog.str(),
                     settings = settings->getSettingsFile()](std::optional<fs::path> p) {
        if (p && !p->empty()) {
            int errorCode = 0;
            auto* zip = zip_open(p->u8string().c_str(), ZIP_CREATE | ZIP_TRUNCATE, &errorCode);
            if (!zip) {
                // TODO handle error
                // errorCode contains the error
                return;
            }
            auto addSource = [&](const char* name, zip_source_t* source) {
                if (!source) {
                    // TODO handle error
                    // zip_strerror(zip) returns the error string
                } else if (zip_file_add(zip, name, source, ZIP_FL_ENC_UTF_8) < 0) {
                    // TODO handler error
                    // zip_strerror(zip) returns the error string
                    zip_source_free(source);
                }
            };
            addSource("ellipsized-logs.txt", zip_source_buffer(zip, ellipsizedLog.data(), ellipsizedLog.length(), 0));
            addSource("full-logs.txt", zip_source_buffer(zip, fullLog.data(), fullLog.length(), 0));
            addSource("input-gdk_events.txt", zip_source_buffer(zip, gdkeventslog.data(), gdkeventslog.length(), 0));
#ifdef ZIP_LENGTH_TO_END  // Only introduced in libzip 1.10.1 - ubuntu 22 has v1.7.3
            addSource("settings.xml", zip_source_file(zip, settings.u8string().c_str(), 0, ZIP_LENGTH_TO_END));
#else
            addSource("settings.xml", zip_source_file(zip, settings.u8string().c_str(), 0, -1));
#endif
            auto version = xoj::util::getVersionInfo();
            addSource("version.txt", zip_source_buffer(zip, version.data(), version.length(), 0));

            errorCode = zip_close(zip);
            if (errorCode) {
                // TODO handle error
            }
        }
    };

    auto popup = xoj::popup::PopupWindowWrapper<xoj::SaveExportDialog>(
            nullptr, fs::path(), _("Export Logs"), _("Export"), std::move(pathValidation), std::move(callback));

    auto* fc = GTK_FILE_CHOOSER(popup.getPopup()->getWindow());

    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "ZIP archive");
    gtk_file_filter_add_mime_type(filter, "application/zip");
    gtk_file_chooser_add_filter(fc, filter);

    popup.show(GTK_WINDOW(gtk_widget_get_ancestor(parent, GTK_TYPE_WINDOW)));
}

DeviceTestingArea::DeviceTestingArea(GladeSearchpath* gladeSearchPath, GtkBox* parent, Settings* settings):
        settings(settings),
        gladeSearchPath(gladeSearchPath),
        emulateTipContactOnButtonPress(settings->getInputSystemTPCButtonEnabled()),
        inputContext(std::make_unique<InputContext>(settings, *this)),
        ancestorScrolledWindow(gtk_widget_get_ancestor(GTK_WIDGET(parent), GTK_TYPE_SCROLLED_WINDOW), xoj::util::ref),
        data(std::make_unique<Data>()) {
    data->log << std::setprecision(6);
    data->log.imbue(std::locale::classic());
    data->allGdkEvents << std::setprecision(6);
    data->allGdkEvents.imbue(std::locale::classic());
    data->ellipsizedLog << std::setprecision(6);
    data->ellipsizedLog.imbue(std::locale::classic());

    data->timeReference = strict_cast<guint32>(as_unsigned(g_get_monotonic_time()) / 1000);

    Builder builder(gladeSearchPath, UI_FILE_NAME);
    gtk_box_append(parent, builder.get(UI_MAIN_ID));

    struct D {
        std::stringstream* log;
        GtkTextBuffer* view;
    };
    g_signal_connect_data(builder.get("logpopover"), "show", G_CALLBACK(+[](GtkWidget*, gpointer d) {
                              auto* dd = static_cast<D*>(d);
                              gtk_text_buffer_set_text(dd->view, dd->log->str().c_str(), -1);
                          }),
                          new D{&data->ellipsizedLog, GTK_TEXT_BUFFER(builder.get<GObject>("logbuffer"))},
                          xoj::util::closure_notify_cb<D>, GConnectFlags(0));

    auto* cbEmulateTipContactOnButtonPress = builder.get("cbEmulateTipContactOnButtonPress");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(cbEmulateTipContactOnButtonPress), emulateTipContactOnButtonPress);
    g_signal_connect(cbEmulateTipContactOnButtonPress, "toggled",
                     G_CALLBACK(+[]
#if GTK_MAJOR_VERSION == 3
                                (GtkToggleButton* btn, gpointer d) {
                                    static_cast<DeviceTestingArea*>(d)->emulateTipContactOnButtonPress =
                                            gtk_toggle_button_get_active(btn);
                                }),
#else
                                (GtkCheckButton* btn,, gpointer d) {
                                    static_cast<DeviceTestingArea*>(d)->emulateTipContactOnButtonPress =
                                            gtk_check_button_get_active(btn);
                                }),
#endif
                     this);

    g_signal_connect(builder.get("btnExportLogs"), "clicked", G_CALLBACK(+[](GtkButton* btn, gpointer d) {
                         auto* self = static_cast<DeviceTestingArea*>(d);
                         saveLog(self->data->log, self->data->ellipsizedLog, self->data->allGdkEvents, GTK_WIDGET(btn),
                                 self->settings);
                     }),
                     this);

    g_signal_connect(builder.get("btnAllDevices"), "clicked", G_CALLBACK(+[](GtkButton* btn, gpointer d) {
                         auto* self = static_cast<DeviceTestingArea*>(d);
                         auto dlg = xoj::popup::PopupWindowWrapper<DeviceListDialog>(self->gladeSearchPath,
                                                                                     self->settings);
                         dlg.show(GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(btn), GTK_TYPE_WINDOW)));
                     }),
                     this);


    lastDeviceClassConfig = std::make_unique<DeviceClassConfigGui>(
            gladeSearchPath, GTK_BOX(builder.get("boxLastDevice")), settings, true);
    drawingArea.reset(builder.get("testArea"), xoj::util::ref);

    mouseIndicators[0].reset(builder.get("mouse-in-use"), xoj::util::ref);
    mouseIndicators[GDK_BUTTON_PRIMARY].reset(builder.get("mouse-left"), xoj::util::ref);
    mouseIndicators[GDK_BUTTON_MIDDLE].reset(builder.get("mouse-middle"), xoj::util::ref);
    mouseIndicators[GDK_BUTTON_SECONDARY].reset(builder.get("mouse-right"), xoj::util::ref);
    stylusIndicators[0].reset(builder.get("stylus-hover"), xoj::util::ref);
    stylusIndicators[1].reset(builder.get("stylus-tip"), xoj::util::ref);
    stylusIndicators[2].reset(builder.get("stylus-1"), xoj::util::ref);
    stylusIndicators[3].reset(builder.get("stylus-2"), xoj::util::ref);
    eraserIndicators[0].reset(builder.get("eraser-hover"), xoj::util::ref);
    eraserIndicators[1].reset(builder.get("eraser-tip"), xoj::util::ref);
    touchIndicators[0].reset(builder.get("touch-1"), xoj::util::ref);
    touchIndicators[1].reset(builder.get("touch-2"), xoj::util::ref);
    touchIndicators[2].reset(builder.get("touch-3"), xoj::util::ref);
    touchIndicators[3].reset(builder.get("touch-4"), xoj::util::ref);
    touchIndicators[4].reset(builder.get("touch-5"), xoj::util::ref);

    inputContext->connect(drawingArea.get(), /* connectKeyboardHandler */ false, [data = data.get()](GdkEvent* e) {
        xoj::input::printGdkEvent(data->allGdkEvents, e, data->timeReference);
    });

    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawingArea.get()), draw, &data->lastEvs, nullptr);
}

DeviceTestingArea::~DeviceTestingArea() = default;


static constexpr const char* handlerTypeToString[] = {"MOUSE", "STYLUS", "TOUCH"};


bool DeviceTestingArea::handle(const InputEvent& e, HandlerType handlerType) {
    if (e.deviceClass == INPUT_DEVICE_TOUCHSCREEN) {
        /*
         * kinetic scrolling will capture touch event if the sequence looks like a scroll.
         * We do not want to scroll when testing touch devices in the testing area, so we disable it
         */
        if (e.type == ENTER_EVENT) {
            gtk_scrolled_window_set_kinetic_scrolling(GTK_SCROLLED_WINDOW(ancestorScrolledWindow.get()), false);
        } else if (e.type == LEAVE_EVENT) {
            gtk_scrolled_window_set_kinetic_scrolling(GTK_SCROLLED_WINDOW(ancestorScrolledWindow.get()), true);
        }
    }
    data->log << "Handler " << handlerTypeToString[handlerType] << " received:\n";
    xoj::input::printEvent(data->log, e, data->timeReference);

    if (e.type != MOTION_EVENT || e.device != data->lastMotionDevice) {
        if (data->nbOfConsecutiveMotionEvents > 1) {
            data->ellipsizedLog << "   ⦙\n  " << data->nbOfConsecutiveMotionEvents - 1
                                << " motion events omitted\n   ⦙\n";
        }
        if (e.type == MOTION_EVENT) {
            data->nbOfConsecutiveMotionEvents = 1;
            data->lastMotionDevice = e.device;
        } else {
            data->nbOfConsecutiveMotionEvents = 0;
            data->lastMotionDevice = nullptr;
        }
        data->ellipsizedLog << "Handler " << handlerTypeToString[handlerType] << " received:\n";
        xoj::input::printEvent(data->ellipsizedLog, e, data->timeReference);
    } else {
        if (data->nbOfConsecutiveMotionEvents == 0) {
            data->ellipsizedLog << "Handler " << handlerTypeToString[handlerType] << " received:\n";
            xoj::input::printEvent(data->ellipsizedLog, e, data->timeReference);
            data->lastMotionDevice = e.device;
        }
        data->nbOfConsecutiveMotionEvents++;
    }

    auto queueEvent = [&]() {
        auto category = [&]() {
            switch (e.deviceClass) {
                case INPUT_DEVICE_MOUSE:
                    return DevCategory::MOUSE;
                case INPUT_DEVICE_PEN:
                    return DevCategory::STYLUS;
                case INPUT_DEVICE_ERASER:
                    return DevCategory::ERASER;
                case INPUT_DEVICE_TOUCHSCREEN:
                    return DevCategory::TOUCHSCREEN;
                default:
                    return DevCategory::MOUSE;
            }
        }();
        data->lastEvs.push_front(
                Ev{Point(e.relative.x, e.relative.y, e.pressure), data->buttonsStatus[category], handlerType});
        gtk_widget_queue_draw(drawingArea.get());
    };

    // The mouse rarely issues LEAVE_EVENT. We turn off the "in-use" indicator after a short time of inactivity
    auto resetInUseTimer = [](xoj::util::GSourceURef& timer, GtkWidget* indicator) {
        if (!timer) {
            gtk_widget_add_css_class(indicator, "pressed");
        }
        struct cbData {
            GtkWidget* indicator;
            xoj::util::GSourceURef* timer;
        };
        timer = g_timeout_add_full(
                G_PRIORITY_DEFAULT, IN_USE_RESET_DELAY,
                +[](gpointer d) {
                    auto* data = static_cast<cbData*>(d);
                    gtk_widget_remove_css_class(data->indicator, "pressed");
                    data->timer->consume();
                    return G_SOURCE_REMOVE;
                },
                new cbData{indicator, &timer}, xoj::util::destroy_cb<cbData>);
    };

    if (e.type == BUTTON_PRESS_EVENT) {
        lastDeviceClassConfig->setDevice(InputDevice(e.device));
        if (handlerType == MOUSE && e.button < mouseIndicators.size()) {
            data->buttonsStatus[DevCategory::MOUSE].set(e.button);
            gtk_widget_add_css_class(mouseIndicators[e.button].get(), "pressed");
            resetInUseTimer(mouseInUseTimer, mouseIndicators[0].get());
        } else if (handlerType == STYLUS) {
            if (e.deviceClass == INPUT_DEVICE_PEN && e.button < stylusIndicators.size()) {
                data->buttonsStatus[DevCategory::STYLUS].set(e.button);
                gtk_widget_add_css_class(stylusIndicators[e.button].get(), "pressed");
                resetInUseTimer(stylusInUseTimer, stylusIndicators[0].get());
                if (e.button != CONTACT_BUTTON_INDEX && emulateTipContactOnButtonPress &&
                    CONSIDERED_FOR_TIP_EMULATION[e.button]) {
                    data->buttonsStatus[DevCategory::STYLUS].set(CONTACT_BUTTON_INDEX);
                    gtk_widget_add_css_class(stylusIndicators[CONTACT_BUTTON_INDEX].get(), "pressed");
                }
            } else if (e.deviceClass == INPUT_DEVICE_ERASER && e.button < eraserIndicators.size()) {
                data->buttonsStatus[DevCategory::ERASER].set(e.button);
                gtk_widget_add_css_class(eraserIndicators[e.button].get(), "pressed");
                resetInUseTimer(eraserInUseTimer, eraserIndicators[0].get());
            }
        } else if (handlerType == TOUCH) {
            data->nbTouchSequences++;
            if (data->nbTouchSequences <= 5) {
                gtk_widget_add_css_class(touchIndicators[data->nbTouchSequences - 1].get(), "pressed");
            }
            data->buttonsStatus[DevCategory::TOUCHSCREEN].set(CONTACT_BUTTON_INDEX);
        }
        queueEvent();
    } else if (e.type == BUTTON_RELEASE_EVENT) {
        if (handlerType == MOUSE && e.button < mouseIndicators.size()) {
            data->buttonsStatus[DevCategory::MOUSE].reset(e.button);
            gtk_widget_remove_css_class(mouseIndicators[e.button].get(), "pressed");
            resetInUseTimer(mouseInUseTimer, mouseIndicators[0].get());
        } else if (handlerType == TOUCH) {
            if (data->nbTouchSequences == 0) {
                data->log << "ERROR: Terminating more touch sequences than had been started" << std::endl;
            } else if (data->nbTouchSequences <= 5) {
                gtk_widget_remove_css_class(touchIndicators[data->nbTouchSequences - 1].get(), "pressed");
                if (--data->nbTouchSequences == 0) {
                    data->buttonsStatus[DevCategory::TOUCHSCREEN].reset(0);
                }
            }
        } else {
            if (e.deviceClass == INPUT_DEVICE_PEN && e.button < stylusIndicators.size()) {
                data->buttonsStatus[DevCategory::STYLUS].reset(e.button);
                gtk_widget_remove_css_class(stylusIndicators[e.button].get(), "pressed");
                resetInUseTimer(stylusInUseTimer, stylusIndicators[0].get());
                if (e.button != CONTACT_BUTTON_INDEX && emulateTipContactOnButtonPress) {
                    data->buttonsStatus[DevCategory::STYLUS].reset(CONTACT_BUTTON_INDEX);
                    gtk_widget_remove_css_class(stylusIndicators[CONTACT_BUTTON_INDEX].get(), "pressed");
                }
            } else if (e.deviceClass == INPUT_DEVICE_ERASER && e.button < eraserIndicators.size()) {
                data->buttonsStatus[DevCategory::ERASER].reset(e.button);
                gtk_widget_remove_css_class(eraserIndicators[e.button].get(), "pressed");
                resetInUseTimer(eraserInUseTimer, eraserIndicators[0].get());
            }
        }
        queueEvent();
    } else if (e.type == MOTION_EVENT) {
        if (handlerType == MOUSE) {
            resetInUseTimer(mouseInUseTimer, mouseIndicators[0].get());
        } else if (handlerType == STYLUS) {
            if (e.deviceClass == INPUT_DEVICE_PEN) {
                resetInUseTimer(stylusInUseTimer, stylusIndicators[0].get());
            } else {
                resetInUseTimer(eraserInUseTimer, eraserIndicators[0].get());
            }
        }
        queueEvent();
    }
    return true;
}

void DeviceTestingArea::saveSettings() const {
    this->settings->setInputSystemTPCButtonEnabled(this->emulateTipContactOnButtonPress);
    this->lastDeviceClassConfig->saveSettings();
}
