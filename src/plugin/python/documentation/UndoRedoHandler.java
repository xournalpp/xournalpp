
public interface UndoRedoHandler {
	/**
	 * If there is something to undo
	 * @return true if yes
	 */
	boolean canUndo();

	/**
	 * If there is something to redo
	 * @return true if yes
	 */
	boolean canRedo();

	/**
	 * Undo the last operation
	 * @return false on fail
	 */
	boolean undo();
	
	/**
	 * Redo the last operation
	 * @return false on fail
	 */
	boolean redo();

}
