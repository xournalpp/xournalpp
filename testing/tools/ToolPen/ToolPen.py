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

class ToolPen(ToolTest):
	def __init__(self, xoj):
		ToolTest.__init__(self, xoj)

	def runTest(self):
		self.xoj.setSelectedTool(self.xoj.TOOL_PEN)
		self.xoj.setToolSize(self.xoj.TOOL_SIZE_FINE)
		self.doTestInput()

		path = os.path.realpath(__file__ + '/../result.xoj')
		self.checkContents(path)

