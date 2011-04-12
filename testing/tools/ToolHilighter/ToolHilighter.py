# Xournal++
#
# Testclass for Undo / Redohandler
#
# @author Xournal Team
# http://xournal.sf.net
#
# @license GPL

from tools.ToolTest import ToolTest
import os

class ToolHilighter(ToolTest):
	def __init__(self, xoj):
		ToolTest.__init__(self, xoj)

	def runTest(self):
		self.xoj.setSelectedTool(self.xoj.TOOL_HILIGHTER)
		self.xoj.setToolSize(self.xoj.TOOL_SIZE_MEDIUM)
		self.initTool()
		self.doTestInput()

		path = os.path.realpath(__file__ + '/../result.xoj')
		self.checkContents(path)

		undo = self.xoj.getUndoRedoHandler()
		print undo.getUndoStackTopTypeName()


