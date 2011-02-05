INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = xournalpp
xournalpp_SOURCES = \
	control/settings/ev-metadata-manager.cpp \
	control/settings/Settings.cpp \
	control/tools/EditSelection.cpp \
	control/tools/EraseHandler.cpp \
	control/tools/Selection.cpp \
	control/tools/ShapeRecognizer.cpp \
	control/tools/VerticalToolHandler.cpp \
	control/Actions.cpp \
	control/BackgroundThreadHandler.cpp \
	control/ClipboardHandler.cpp \
	control/Control.cpp \
	control/LoadHandler.cpp \
	control/PrintHandler.cpp \
	control/RecentManager.cpp \
	control/SaveHandler.cpp \
	control/SearchControl.cpp \
	control/Tool.cpp \
	control/ToolHandler.cpp \
	control/ZoomControl.cpp \
	gui/dialog/AboutDialog.cpp \
	gui/dialog/ExportDialog.cpp \
	gui/dialog/FormatDialog.cpp \
	gui/dialog/ImagesDialog.cpp \
	gui/dialog/PdfPagesDialog.cpp \
	gui/dialog/SelectBackgroundColorDialog.cpp \
	gui/dialog/SettingsDialog.cpp \
	gui/sidebar/Sidebar.cpp \
	gui/sidebar/SidebarPreview.cpp \
	gui/toolbarMenubar/ToolMenuHandler.cpp \
	gui/widgets/gtkmenutooltogglebutton.cpp \
	gui/widgets/SelectColor.cpp \
	gui/widgets/ZoomCallib.cpp \
	gui/Cursor.cpp \
	gui/GladeGui.cpp \
	gui/MainWindow.cpp \
	gui/PageView.cpp \
	gui/SearchBar.cpp \
	gui/Shadow.cpp \
	gui/TextEditor.cpp \
	gui/XournalWidget.cpp \
	model/Document.cpp \
	model/Element.cpp \
	model/Font.cpp \
	model/FormatDefinitions.cpp \
	model/Image.cpp \
	model/Layer.cpp \
	model/LinkDestination.cpp \
	model/Page.cpp \
	model/Point.cpp \
	model/Stroke.cpp \
	model/Text.cpp \
	pdf/cairo/CairoPdf.cpp \
	pdf/poppler/XojPopplerAction.cpp \
	pdf/poppler/XojPopplerDocument.cpp \
	pdf/poppler/XojPopplerIter.cpp \
	pdf/poppler/XojPopplerPage.cpp \
	pdf/PdfExport.cpp \
	undo/ColorUndoAction.cpp \
	undo/DeleteUndoAction.cpp \
	undo/EraseUndoAction.cpp \
	undo/FontUndoAction.cpp \
	undo/InsertDeletePageUndoAction.cpp \
	undo/InsertLayerUndoAction.cpp \
	undo/InsertUndoAction.cpp \
	undo/MoveUndoAction.cpp \
	undo/PageBackgroundChangedUndoAction.cpp \
	undo/RecognizerUndoAction.cpp \
	undo/RemoveLayerUndoAction.cpp \
	undo/ScaleUndoAction.cpp \
	undo/SizeUndoAction.cpp \
	undo/TextUndoAction.cpp \
	undo/UndoAction.cpp \
	undo/UndoRedoHandler.cpp \
	util/CrashHandler.cpp \
	util/GzHelper.cpp \
	util/ObjectStream.cpp \
	util/OutputStream.cpp \
	util/pixbuf-utils.cpp \
	util/Range.cpp \
	util/String.cpp \
	util/StringBuffer.cpp \
	util/Util.cpp \
	view/DocumentView.cpp \
	view/TextView.cpp \
	Xournalpp.cpp

xournalpp_CXXFLAGS = -g -rdynamic -DXOURNAL_FLAGS

xournalpp_LDADD = @PACKAGE_LIBS@ $(INTLLIBS)

