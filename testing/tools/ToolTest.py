# Xournal++
#
# Textclass for Undo / Redohandler
#
# @author Xournal Team
# http://xournal.sf.net
#
# @license GPL

from XournalTest import XournalTest

class ToolTest(XournalTest):
	def __init__(self, xoj):
		XournalTest.__init__(self, xoj)

	def tearUp(self):
		self.xoj.newFile(True)
		self.xoj.setToolColor(0xff0000)
		self.xoj.setRulerEnabled(False)
		self.xoj.setShapeRecognizerEnabled(False)
		self.xoj.setCurrentPageBackground(self.xoj.BACKGROUND_TYPE_RULED)

	def tearDown(self):
		print 'ToolTest.tearDown'


