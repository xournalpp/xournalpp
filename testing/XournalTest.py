# Xournal++
#
# Testclass "Template"
#
# @author Xournal Team
# http://xournal.sf.net
#
# @license GPL

class XournalTest:
	def __init__(self, xoj):
		self.xoj = xoj

	def test(self, xoj):
		self.tearUp(xoj)
		self.runTest(xoj)
		self.tearDown(xoj)

	def runTest(self, xoj):
		pass

	def tearUp(self, xoj):
		pass

	def tearDown(self, xoj):
		pass

