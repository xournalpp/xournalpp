INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = xournalpp
xournalpp_SOURCES = \
	model/Page.cpp \
	model/Document.cpp \
	model/Layer.cpp \
	model/Text.cpp \
	model/Image.cpp \
	model/Element.cpp \
	model/Stroke.cpp \
	model/String.cpp \
	model/StringBuffer.cpp \
	model/Font.cpp \
	model/LinkDestination.cpp \
	model/FormatDefinitions.cpp \
	control/Settings.cpp \
	control/LoadHandler.cpp \
	control/SaveHandler.cpp \
	control/ZoomControl.cpp \
	control/Cursor.cpp \
	control/ev-metadata-manager.cpp \
	control/ShapeRecognizer.cpp \
	control/BackgroundThreadHandler.cpp \
	control/Selection.cpp \
	control/EditSelection.cpp \
	control/SearchControl.cpp \
	control/ClipboardHandler.cpp \
	control/PrintHandler.cpp \
	gui/GladeGui.cpp \
	gui/AboutDialog.cpp \
	gui/PdfPagesDialog.cpp \
	gui/ImagesDialog.cpp \
	gui/ExportDialog.cpp \
	gui/FormatDialog.cpp \
	gui/widgets/ZoomCallib.cpp \
	gui/widgets/SelectColor.cpp \
	gui/widgets/gtkmenutooltogglebutton.cpp \
	gui/MainWindow.cpp \
	gui/SettingsDialog.cpp \
	gui/toolbarMenubar/ToolMenuHandler.cpp \
	gui/XournalWidget.cpp \
	gui/PageView.cpp \
	gui/TextEditor.cpp \
	gui/sidebar/Sidebar.cpp \
	gui/sidebar/SidebarPreview.cpp \
	gui/SearchBar.cpp \
	gui/Shadow.cpp \
	gui/SelectBackgroundColorDialog.cpp \
	view/TextView.cpp \
	view/DocumentView.cpp \
	control/Actions.cpp \
	control/Tool.cpp \
	control/ToolHandler.cpp \
	control/Control.cpp \
	control/RecentManager.cpp \
	control/UndoRedoHandler.cpp \
	util/Util.cpp \
	util/OutputStream.cpp \
	util/ObjectStream.cpp \
	util/CrashHandler.cpp \
	util/pixbuf-utils.cpp \
	pdf/PdfExport.cpp \
	pdf/popplerGlibExt/PopplerGlibExtension.cpp \
	Xournalpp.cpp

xournalpp_CXXFLAGS = -g -rdynamic -DXOURNAL_FLAGS

xournalpp_LDADD = @PACKAGE_LIBS@ $(INTLLIBS)

