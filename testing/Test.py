import xournal
import undo.UndoRedoTest
from TestNotImplementedException import TestNotImplementedException
import os
import sys
import gtk


class XournalTestRunner:
	def __init__(self):
		self.failedTests = 0
		self.notImplementedTests = 0
		self.successfullyTests = 0
		self.xoj = xournal.Xournal()
		self.faileTest = []

	def start(self):
		print '=== Xournal Testsuit ===\n'

		subfolders = ['clipboard', 'document', 'tools', 'undo']
		for folder in subfolders:
			self.xournalRunTestInSubfolder(folder);

		print '\n=== End Xournal Testsuit ===\n'

		if self.failedTests == 0:
			msg = 'All test passed'

			if self.notImplementedTests != 0:
				msg += ' (%i not implemented)' % self.notImplementedTests

			md = gtk.MessageDialog(None, 0, gtk.MESSAGE_INFO, gtk.BUTTONS_CLOSE, msg)
			md.run()
			md.destroy()
		else:
			tests = '';
			for test in self.faileTest:
				tests += u'\n- ' + test

			notImplemented = ''
			if self.notImplementedTests != 0:
				notImplemented = ' (%i not implemented)' % self.notImplementedTests


			md = gtk.MessageDialog(None, 0, gtk.MESSAGE_WARNING, gtk.BUTTONS_CLOSE, '')
			md.set_markup('<b>%i Test failed%s:</b>\n%s' % (self.failedTests, notImplemented, tests))
			md.run()
			md.destroy()




	def xournalRunTestInSubfolder(self, subfolder):
		path = os.path.realpath(__file__)
		path = os.path.dirname(path)
		folder = os.path.join(path, subfolder)

		print 'Running scripts in %s' % folder

		for name in os.listdir(folder):
			dirfile = os.path.join(folder, name)
		
			if os.path.isdir(dirfile) and not name.startswith('.') and os.path.exists(os.path.join(dirfile, name + '.py')):
				print 'Run test in %s' % dirfile
	#			print 'Debug: import %s from %s' % (name, subfolder + '.' + name + '.' + name)

				try:
					sFrom = subfolder + '.' + name + '.' + name
					moduleObject = __import__(sFrom, globals(), locals(), [name], -1)
					classObject = getattr(moduleObject, name)
					obj = classObject(self.xoj)
					obj.test()
					self.successfullyTests += 1
				except (AssertionError, IOError) as e:
					self.failedTests += 1

					self.faileTest.append(subfolder + '/' + name)

					print >> sys.stderr, 'Test %s failed!' % name
					print >> sys.stderr, type(e)     # the exception instance
					print >> sys.stderr, e.args      # arguments stored in .args
					print >> sys.stderr, e
				except (TestNotImplementedException) as e:
					self.notImplementedTests += 1
					
	#			except (Exception) as e:
	#				print "Test %s Unexpected error:" % name, type(e)
	#				print e.args      # arguments stored in .args
	#				print e



		print '\n\n\n==================================================='
		print 'Testresult: %i successfully, %i not implemented, %i failed!' % (self.successfullyTests, self.notImplementedTests, self.failedTests)
		print '===================================================\n\n'


def xournalTest(args = ''):
	print 'Xournal testsuit started...'

	tester = XournalTestRunner()
	tester.start()
	del tester


	

#	print xoj.openFile('/home/andreas/tmp/Notiz-10-03-2011-16-57.xoj')

"""	xoj.mousePressed(10, 10)
	xoj.mouseMoved(20, 20)
	xoj.mouseReleased()

	ud = ColorUndoAction()
"""


