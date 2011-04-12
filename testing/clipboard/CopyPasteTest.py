# Xournal++
#
# Textclass for Undo / Redohandler
#
# @author Xournal Team
# http://xournal.sf.net
#
# @license GPL

from XournalTest import XournalTest
import os

class CopyPasteTest(XournalTest):
	def __init__(self, xoj):
		XournalTest.__init__(self, xoj)

	def tearUp(self):
		self.xoj.newFile(True)
		self.xoj.setCurrentPageBackground(self.xoj.BACKGROUND_TYPE_RULED)
		path = os.path.realpath(__file__ + '/../source.xoj')
		self.xoj.openFile(path)

	def select(self):
		self.xoj.setSelectedTool(self.xoj.TOOL_SELECT_RECT)

		points = [[100, 100]]
		points.append([300, 300]);
		self.mouseInput(points, 300);


	def copy(self):
		self.select()
		assert(self.xoj.copy() == True)

	def cut(self):
		self.select()
		assert(self.xoj.cut() == True)


	def tearDown(self):
		pass


