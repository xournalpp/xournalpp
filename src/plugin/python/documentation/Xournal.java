public interface Xournal {
	int TOOL_PEN = 1;
	int TOOL_ERASER = 2;
	int TOOL_HILIGHTER = 3;
	int TOOL_TEXT = 3;
	int TOOL_IMAGE = 4;
	int TOOL_SELECT_RECT = 5;
	int TOOL_SELECT_REGION = 6;
	int TOOL_SELECT_OBJECT = 7;
	int TOOL_VERTICAL_SPACE = 8;
	int TOOL_HAND = 9;
	
	
	int TOOL_SIZE_VERY_FINE = 0;
	int TOOL_SIZE_FINE = 1;
	int TOOL_SIZE_MEDIUM = 2;
	int TOOL_SIZE_THICK = 3;
	int TOOL_SIZE_VERY_THICK = 4;

	int BACKGROUND_TYPE_NONE = 1;
	int BACKGROUND_TYPE_PDF = 2;
	int BACKGROUND_TYPE_IMAGE = 3;
	int BACKGROUND_TYPE_LINED = 4;
	int BACKGROUND_TYPE_RULED = 5;
	int BACKGROUND_TYPE_GRAPH = 6;
	
	int ERASER_TYPE_DEFAULT = 1;
	int ERASER_TYPE_WHITEOUT = 2;
	int ERASER_TYPE_DELETE_STROKE = 3;
	
	/**
	 * Sets the current tool
	 * @param tool A constant defined above
	 */
	void setSelectedTool(int tool);
	
	/**
	 * Return the selected tool
	 * @return (See constants TOOL_*)
	 */
	int getSelectedTool();
	
	/**
	 * Sets the current tool size
	 * @param tool A constant defined above
	 */
	void setToolSize(int tool);
	
	/**
	 * Return the selected tool size
	 * @return (See constants TOOL_SIZE_*)
	 */
	int getToolSize();
	
	/**
	 * Sets the tool Color in RGB
	 */
	void setToolColor(int color);
	
	/**
	 * Returns the tool color in RGB
	 */
	int getToolColor();

	/**
	 * Enables the ruler
	 */
	void setRulerEnabled(boolean enabled);

	/**
	 * Return true if the ruler is enabled
	 */
	boolean isRulerEnabled();

	/**
	 * Enables the shape recognizer
	 */
	void setShapeRecognizerEnabled(boolean enabled);
	
	/**
	 * Return true if the shape recognizer is enabled
	 */
	boolean isShapeRecognizerEnabled();

	/**
	 * Set the eraser type (ERASER_TYPE_*)
	 */
	void setEraserType(int eraserType);
	
	/**
	 * Returns the eraser type (ERASER_TYPE_*)
	 */
	int getEraserType();
	
	/**
	 * Return the background type of the current page (see BACKGROUND_TYPE_*)
	 */
	int getCurrentPageBackground();
	
	/**
	 * Set the background type of the current page (see BACKGROUND_TYPE_*)
	 * Don't use BACKGROUND_TYPE_PDF or BACKGROUND_TYPE_PDF
	 */
	void setCurrentPageBackground(int backgroundType);
	
	/**
	 * Creates a new file
	 * 
	 * @param [optional] force force creation of a new file, loose your unsaved contents
	 * 
	 * @return if a new file was created, false if the user canceled the operation
	 */
	boolean newFile(boolean force);
	
	/**
	 * Save the current document
	 * @param path a complete filename, e.g. /home/myname/test.xoj
	 * @return true if successfully, false if failed
	 */
	boolean saveFile(String path);
	
	/**
	 * Open a xournal file
	 * @param path a complete filename, e.g. /home/myname/test.xoj
	 * @param [optional] scrollToPage The page to scroll or -1 to not scroll
	 * @return true if successfully, false if failed
	 */
	boolean openFile(String path, int scrollToPage);
	
	/**
	 * Press the mouse on the selected View
	 * @param x The X Coordinate
	 * @param y The Y Coordinate
	 */
	void mousePressed(double x, double y);

	/**
	 * Press the mouse on the selected View
	 * @param x The X Coordinate
	 * @param y The Y Coordinate
	 */
	void mouseMoved(double x, double y);

	/**
	 * Press the mouse on the selected View
	 */
	void mouseReleased();
	
	/**
	 * The Undo- / Redohandler
	 */
	UndoRedoHandler getUndoRedoHandler();

	/**
	 * Return the Xournal Document
	 */
	Document getDocument();
	
	/**
	 * Gets the selected page ID (first Page: 0)
	 */
	int getSelectedPage();
	
	/**
	 * Sets the selected page (first Page: 0)
	 */
	void selectPage(int page);
}
