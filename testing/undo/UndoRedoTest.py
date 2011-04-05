# Xournal++
#
# Textclass for Undo / Redohandler
#
# @author Xournal Team
# http://xournal.sf.net
#
# @license GPL

from XournalTest import XournalTest

class UndoRedoTest(XournalTest):
	def __init__(self, xoj):
		XournalTest.__init__(self, xoj)

		# TODO debug
		print xoj.getUndoRedoHandler()

	def tearUp(self, xoj):
		print 'UndoRedoTest.tearUp'

	def tearDown(self, xoj):
		print 'UndoRedoTest.tearDown'


