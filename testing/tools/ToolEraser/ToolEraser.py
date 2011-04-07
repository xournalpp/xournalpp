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


class ToolEraser(ToolTest):
	def __init__(self, xoj):
		ToolTest.__init__(self, xoj)

	def runTest(self):
		self.testWhiteout()
		self.testStandard()
		self.testDeleteStroke()
		raise TestNotImplementedException()

	def testWhiteout(self):
		pass

	def testStandard(self):
		# test with normal stroke, with pressure, with ruler
		pass

	def testDeleteStroke(self):
		pass

