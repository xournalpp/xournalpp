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

class ToolEraser(ToolTest):
	def __init__(self, xoj):
		ToolTest.__init__(self, xoj)

	def runTest(self):
		self.xoj.setSelectedTool(self.xoj.TOOL_ERASER)
		self.testWhiteout()
		self.testStandard()
		self.testDeleteStroke()

	def testWhiteout(self):
		path = os.path.realpath(__file__ + '/../source.xoj')
		self.xoj.openFile(path)

		path = os.path.realpath(__file__ + '/../resultWhiteout.xoj')
		self.checkContents(path)

	def testStandard(self):
		# test with normal stroke, with pressure, with ruler
		pass

	def testDeleteStroke(self):
		pass

