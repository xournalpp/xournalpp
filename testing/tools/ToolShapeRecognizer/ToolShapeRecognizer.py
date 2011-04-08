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

# TODO: Test circle, Test rectangel (with one stroke, and with 4 stroke) etc...

class ToolShapeRecognizer(ToolTest):
	def __init__(self, xoj):
		ToolTest.__init__(self, xoj)

	def runTest(self):
		self.xoj.setSelectedTool(self.xoj.TOOL_PEN)
		self.xoj.setToolSize(self.xoj.TOOL_SIZE_FINE)
		self.initTool()
		self.xoj.setShapeRecognizerEnabled(True)
		self.doTestInput()

		path = os.path.realpath(__file__ + '/../result.xoj')
#		self.xoj.saveFile(path)
		self.checkContents(path)


