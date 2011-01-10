/*
 * Xournal++
 *
 * Handles Undo and Redo
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __UNDOREDOHANDLER_H__
#define __UNDOREDOHANDLER_H__

#include "../model/String.h"
#include "../model/Layer.h"
#include "../model/Page.h"
#include "../gui/PageView.h"
#include "../gui/TextEditor.h"
#include "../util/Util.h"

class Control;

class UndoRedoListener {
public:
	virtual void undoRedoChanged() = 0;
	virtual void undoRedoPageChanged(XojPage * page) = 0;
};

class UndoAction {
public:
	UndoAction();

	virtual bool undo(Control * control) = 0;
	virtual bool redo(Control * control) = 0;

	virtual String getText() = 0;

	/**
	 * Get the affected pages, the Array is terminated with NULL and should be freed with delete[]
	 */
	virtual XojPage ** getPages();
protected:
	XojPage * page;
	bool undone;
};

class InsertUndoAction: public UndoAction {
public:
	InsertUndoAction(XojPage * page, Layer * layer, Element * element, PageView * view);
	~InsertUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	virtual String getText();
private:
	Layer * layer;
	Element * element;
	PageView * view;
};

class TextUndoAction: public UndoAction {
public:
	TextUndoAction(XojPage * page, Layer * layer, Text * text, String lastText, PageView * view,
			TextEditor * textEditor);
	~TextUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	virtual String getText();

	String getUndoText();

	void textEditFinished();
private:
	Layer * layer;
	Text * text;
	String lastText;
	String newText;
	PageView * view;

	TextEditor * textEditor;
};

class DeleteUndoAction: public UndoAction {
public:
	DeleteUndoAction(XojPage * page, PageView * view, bool eraser);
	~DeleteUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	void addElement(Layer * layer, Element * e, int pos);

	virtual String getText();
private:
	GList * elements;
	PageView * view;
	bool eraser;
};

class RecognizerUndoAction: public UndoAction {
public:
	RecognizerUndoAction(XojPage * page, PageView * view, Layer * layer, Element * original, Element * recognized);
	~RecognizerUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	virtual String getText();
private:
	PageView * view;
	Layer * layer;
	Element * original;
	Element * recognized;
};

class EraseUndoAction: public UndoAction {
public:
	EraseUndoAction(XojPage * page, PageView * view);
	~EraseUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	void addOriginal(Layer * layer, Stroke * element, int pos);
	void addEdited(Layer * layer, Stroke * element, int pos);

	void cleanup();

	virtual String getText();
private:
	PageView * view;

	GList * edited;
	GList * original;
};

class InsertLayerUndoAction: public UndoAction {
public:
	InsertLayerUndoAction(XojPage * page, Layer * layer);
	~InsertLayerUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	virtual String getText();
private:
	Layer * layer;
};

class RemoveLayerUndoAction: public UndoAction {
public:
	RemoveLayerUndoAction(XojPage * page, Layer * layer, int layerPos);
	~RemoveLayerUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	virtual String getText();
private:
	Layer * layer;
	int layerPos;
};

class InsertDeletePageUndoAction: public UndoAction {
public:
	InsertDeletePageUndoAction(XojPage * page, int pagePos, bool inserted);
	~InsertDeletePageUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	virtual String getText();
private:
	bool insertPage(Control * control);
	bool deletePage(Control * control);

private:
	bool inserted;
	int pagePos;
};

class PageBackgroundChangedUndoAction: public UndoAction {
public:
	PageBackgroundChangedUndoAction(XojPage * page, BackgroundType origType, int origPdfPage,
			BackgroundImage origBackgroundImage, double origW, double origH);
	~PageBackgroundChangedUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	virtual String getText();

private:
	BackgroundType origType;
	int origPdfPage;
	BackgroundImage origBackgroundImage;
	double origW;
	double origH;

	BackgroundType newType;
	int newPdfPage;
	BackgroundImage newBackgroundImage;
	double newW;
	double newH;
};

class UndoRedoHandler : public MemoryCheckObject {
public:
	UndoRedoHandler(Control * control);
	virtual ~UndoRedoHandler();

	void undo();
	void redo();

	bool canUndo();
	bool canRedo();

	void addUndoAction(UndoAction * action);
	void addUndoActionBefore(UndoAction * action, UndoAction * before);
	bool removeUndoAction(UndoAction * action);

	String undoDescription();
	String redoDescription();

	void clearContents();

	void fireUpdateUndoRedoButtons(XojPage ** pages);
	void addUndoRedoListener(UndoRedoListener * listener);

	bool isChanged();
private:
	void clearRedo();

private:
	GList * undoList;
	GList * redoList;

	GList * listener;

	Control * control;
};

#endif /* __UNDOREDOHANDLER_H__ */
