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
		self.xoj.setEraserType(self.xoj.ERASER_TYPE_WHITEOUT)

		self.doTestInput()

		path = os.path.realpath(__file__ + '/../resultWhiteout.xoj')
		self.checkContents(path)

	def testStandard(self):
		self.xoj.newFile(True)
		path = os.path.realpath(__file__ + '/../source.xoj')
		self.xoj.openFile(path)

		self.xoj.setEraserType(self.xoj.ERASER_TYPE_DEFAULT)

		points = [[100, 40]]
		points.append([150, 300]);
		self.mouseInput(points, 1);

		path = os.path.realpath(__file__ + '/../resultStandard.xoj')
		self.checkContents(path)

	def testDeleteStroke(self):
		self.xoj.newFile(True)
		path = os.path.realpath(__file__ + '/../source.xoj')
		self.xoj.openFile(path)

		self.xoj.setEraserType(self.xoj.ERASER_TYPE_DELETE_STROKE)

		points = [[100, 40]]
		points.append([150, 300]);
		self.mouseInput(points, 1);

		path = os.path.realpath(__file__ + '/../resultDelete.xoj')
		self.checkContents(path)

