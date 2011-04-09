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

# TODO: remove if implemented
from TestNotImplementedException import TestNotImplementedException

class VerticalTool(ToolTest):
	def __init__(self, xoj):
		ToolTest.__init__(self, xoj)

	def runTest(self):
		raise TestNotImplementedException()



		self.xoj.setSelectedTool(self.xoj.TOOL_PEN)
		self.xoj.newFile(True)
		path = os.path.realpath(__file__ + '/../source.xoj')
		self.xoj.openFile(path)

		points = [[100, 40]]
		points.append([100, 100]);
		self.mouseInput(points, 1);

		self.xoj.setSelectedTool(self.xoj.TOOL_VERTICAL_SPACE)

		points = [[100, 40]]
		points.append([100, 100]);
		self.mouseInput(points, 1);

		path = os.path.realpath(__file__ + '/../result.xoj')
		self.xoj.saveFile(path)
#		self.checkContents(path)

#		raise KeyboardInterrupt

