SUBDIRS = \
	collab \
	util \
	xoj-preview-extractor

bin_PROGRAMS = xournalpp

xournalpp_SOURCES = \
	control/jobs/AutosaveJob.cpp \
	control/jobs/BlockingJob.cpp \
	control/jobs/ExportJob.cpp \
	control/jobs/Job.cpp \
	control/jobs/PdfExportJob.cpp \
	control/jobs/PreviewJob.cpp \
	control/jobs/RenderJob.cpp \
	control/jobs/SaveJob.cpp \
	control/jobs/Scheduler.cpp \
	control/jobs/SynchronizedProgressListener.cpp \
	control/jobs/XournalScheduler.cpp \
	control/settings/ButtonConfig.cpp \
	control/settings/MetadataManager.cpp \
	control/settings/Settings.cpp \
	control/shaperecognizer/CircleRecognizer.cpp \
	control/shaperecognizer/Inertia.cpp \
	control/shaperecognizer/RecoSegment.cpp \
	control/shaperecognizer/ShapeRecognizer.cpp \
	control/shaperecognizer/ShapeRecognizerResult.cpp \
	control/stockdlg/ImageOpenDlg.cpp \
	control/stockdlg/XojOpenDlg.cpp \
	control/tools/EditSelection.cpp \
	control/tools/EditSelectionContents.cpp \
	control/tools/EraseHandler.cpp \
	control/tools/ImageHandler.cpp \
	control/tools/InputHandler.cpp \
	control/tools/Selection.cpp \
	control/tools/VerticalToolHandler.cpp \
	control/xml/Attribute.cpp \
	control/xml/DoubleArrayAttribute.cpp \
	control/xml/DoubleAttribute.cpp \
	control/xml/IntAttribute.cpp \
	control/xml/TextAttribute.cpp \
	control/xml/XmlImageNode.cpp \
	control/xml/XmlNode.cpp \
	control/xml/XmlPointNode.cpp \
	control/xml/XmlStrokeNode.cpp \
	control/xml/XmlTextNode.cpp \
	control/Actions.cpp \
	control/ClipboardHandler.cpp \
	control/ExportHandler.cpp \
	control/Control.cpp \
	control/LoadHandler.cpp \
	control/PdfCache.cpp \
	control/PrintHandler.cpp \
	control/RecentManager.cpp \
	control/SaveHandler.cpp \
	control/ScrollHandler.cpp \
	control/SearchControl.cpp \
	control/Tool.cpp \
	control/ToolHandler.cpp \
	control/ZoomControl.cpp \
	control/XournalMain.cpp \
	gui/dialog/AboutDialog.cpp \
	gui/dialog/ButtonConfigGui.cpp \
	gui/dialog/ExportDialog.cpp \
	gui/dialog/FormatDialog.cpp \
	gui/dialog/GotoDialog.cpp \
	gui/dialog/toolbarCustomize/CustomizeableColorList.cpp \
	gui/dialog/toolbarCustomize/ToolbarCustomizeDialog.cpp \
	gui/dialog/toolbarCustomize/ToolbarDragDropHandler.cpp \
	gui/dialog/toolbarCustomize/ToolbarDragDropHelper.cpp \
	gui/dialog/toolbarCustomize/ToolItemDragCurrentData.cpp \
	gui/dialog/ToolbarManageDialog.cpp \
	gui/dialog/ImagesDialog.cpp \
	gui/dialog/PdfPagesDialog.cpp \
	gui/dialog/SelectBackgroundColorDialog.cpp \
	gui/dialog/SettingsDialog.cpp \
	gui/pageposition/PagePosition.cpp \
	gui/pageposition/PagePositionCache.cpp \
	gui/pageposition/PagePositionHandler.cpp \
	gui/pageposition/PageViewIndex.cpp \
	gui/sidebar/AbstractSidebarPage.cpp \
	gui/sidebar/Sidebar.cpp \
	gui/sidebar/indextree/SidebarIndexPage.cpp \
	gui/sidebar/previews/SidebarLayout.cpp \
	gui/sidebar/previews/SidebarPreviewPage.cpp \
	gui/sidebar/previews/SidebarPreviews.cpp \
	gui/sidebar/previews/SidebarToolbar.cpp \
	gui/toolbarMenubar/model/ToolbarColorNames.cpp \
	gui/toolbarMenubar/model/ToolbarData.cpp \
	gui/toolbarMenubar/model/ToolbarEntry.cpp \
	gui/toolbarMenubar/model/ToolbarItem.cpp \
	gui/toolbarMenubar/model/ToolbarModel.cpp \
	gui/toolbarMenubar/AbstractItem.cpp \
	gui/toolbarMenubar/AbstractToolItem.cpp \
	gui/toolbarMenubar/ColorToolItem.cpp \
	gui/toolbarMenubar/FontButton.cpp \
	gui/toolbarMenubar/MenuItem.cpp \
	gui/toolbarMenubar/ToolButton.cpp \
	gui/toolbarMenubar/ToolMenuHandler.cpp \
	gui/toolbarMenubar/ToolPageLayer.cpp \
	gui/toolbarMenubar/ToolPageSpinner.cpp \
	gui/toolbarMenubar/ToolSelectCombocontrol.cpp \
	gui/toolbarMenubar/ToolZoomSlider.cpp \
	gui/widgets/gtkmenutooltogglebutton.cpp \
	gui/widgets/Scrollbar.cpp \
	gui/widgets/SelectColor.cpp \
	gui/widgets/SpinPageAdapter.cpp \
	gui/widgets/XournalWidget.cpp \
	gui/widgets/ZoomCallib.cpp \
	gui/Cursor.cpp \
	gui/CursorTheme.cpp \
	gui/GladeGui.cpp \
	gui/GladeSearchpath.cpp \
	gui/Layout.cpp \
	gui/LayoutData.cpp \
	gui/MainWindow.cpp \
	gui/PageView.cpp \
	gui/Redrawable.cpp \
	gui/RepaintHandler.cpp \
	gui/SearchBar.cpp \
	gui/Shadow.cpp \
	gui/TextEditor.cpp \
	gui/ToolitemDragDrop.cpp \
	gui/XojColor.cpp \
	gui/XournalView.cpp \
	model/eraser/EraseableStroke.cpp \
	model/eraser/EraseableStrokePart.cpp \
	model/eraser/PartList.cpp \
	model/BackgroundImage.cpp \
	model/BackgroundImageContents.cpp \
	model/Document.cpp \
	model/DocumentHandler.cpp \
	model/DocumentListener.cpp \
	model/Element.cpp \
	model/Font.cpp \
	model/FormatDefinitions.cpp \
	model/Image.cpp \
	model/Layer.cpp \
	model/LayerListener.cpp \
	model/LinkDestination.cpp \
	model/PageRef.cpp \
	model/Point.cpp \
	model/Stroke.cpp \
	model/Text.cpp \
	model/XojPage.cpp \
	pdf/cairo/CairoPdf.cpp \
	pdf/popplerdirect/poppler/XojPopplerAction.cpp \
	pdf/popplerdirect/poppler/XojPopplerDocument.cpp \
	pdf/popplerdirect/poppler/XojPopplerIter.cpp \
	pdf/popplerdirect/poppler/XojPopplerPage.cpp \
	pdf/popplerdirect/PdfBookmark.cpp \
	pdf/popplerdirect/PdfExport.cpp \
	pdf/popplerdirect/PdfObjectWriter.cpp \
	pdf/popplerdirect/PdfRefEntry.cpp \
	pdf/popplerdirect/PdfRefList.cpp \
	pdf/popplerdirect/PdfUtil.cpp \
	pdf/popplerdirect/PdfWriter.cpp \
	pdf/popplerdirect/PdfXRef.cpp \
	pdf/popplerdirect/UpdateRef.cpp \
	pdf/popplerdirect/UpdateRefKey.cpp \
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
	view/DocumentView.cpp \
	view/PdfView.cpp \
	view/TextView.cpp \
	Xournalpp.cpp

if ENABLE_PYTHON
xournalpp_SOURCES += \
	plugin/python/bindings/PyDocument.cpp \
	plugin/python/bindings/PySelection.cpp \
	plugin/python/bindings/PyUndoRedoHandler.cpp \
	plugin/python/bindings/PyXournal.cpp \
	plugin/python/PythonRunner.cpp
endif

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

xournalpp_CXXFLAGS = \
	-I$(top_srcdir)/src/util \
	-g -rdynamic \
	-Wreturn-type -Wuninitialized -Wunused-value -Wunused-variable   

if ENABLE_OS
xournalpp_CXXFLAGS += \
	-DENABLE_OS
endif

xournalpp_LDADD = \
	@PACKAGE_LIBS@ $(INTLLIBS) \
	$(top_builddir)/src/collab/libcollab.la \
	$(top_builddir)/src/util/libutil.la

	