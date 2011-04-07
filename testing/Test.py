import xournal
import undo.UndoRedoTest
import os
import sys
import gtk


class XournalTestRunner:
	def __init__(self):
		self.failedTests = 0
		self.successfullyTests = 0
		self.xoj = xournal.Xournal()
		self.faileTest = []

	def start(self):
		print '=== Xournal Testsuit ===\n'

		self.xournalRunTestInSubfolder('tools');
		self.xournalRunTestInSubfolder('undo');

		print '\n=== End Xournal Testsuit ===\n'

		if self.failedTests == 0:
			md = gtk.MessageDialog(None, 0, gtk.MESSAGE_INFO, gtk.BUTTONS_CLOSE, 'All test passed')
			md.run()
			md.destroy()
		else:
			tests = '';
			for test in self.faileTest:
				tests += u'\n- ' + test

			md = gtk.MessageDialog(None, 0, gtk.MESSAGE_WARNING, gtk.BUTTONS_CLOSE, '')
			md.set_markup('<b>%i Test failed:</b>\n%s' % (self.failedTests, tests))
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
					self.successfullyTests = self.successfullyTests + 1
				except (AssertionError) as e:
					self.failedTests = self.failedTests + 1

					self.faileTest.append(subfolder + '/' + name)

					print >> sys.stderr, 'Test %s failed!' % name
					print >> sys.stderr, type(e)     # the exception instance
					print >> sys.stderr, e.args      # arguments stored in .args
					print >> sys.stderr, e
	#			except (Exception) as e:
	#				print "Test %s Unexpected error:" % name, type(e)
	#				print e.args      # arguments stored in .args
	#				print e



		print '\n\n\n==================================================='
		print 'Testresult: %i successfully, %i failed!' % (self.successfullyTests, self.failedTests)
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


