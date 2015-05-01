/*
 * Xournal++
 *
 * This file is part of the Xournal UnitTests
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <CrashHandler.h>
#include <Stacktrace.h>

#include <gtk/gtk.h>

/**
 * Main Entry point for CppUnit Tests
 */
int main(int argc, char* argv[])
{

	gtk_init(&argc, &argv);

	// init crash handler
	installCrashHandlers();
	if (argc)
	{
		// Filename is needed to get backtracke with filenumbers
		Stacktrace::setExename(argv[0]);
	}

#ifdef XOJ_CALL_LOG_ENABLED
	Log::initlog();
#endif

#ifdef XOJ_MEMORY_LEAK_CHECK_ENABLED
	xoj_type_initMutex();
#endif

	// Init GTK Display
	gdk_display_open_default_libgtk_only();

	// Get the top level suite from the registry
	CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

	// Adds the test to the list of test to run
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(suite);

	// Change the default outputter to a compiler error format outputter
	runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

	// Run the tests.
	bool wasSucessful = runner.run();

	printf("CppUnit result: %s\n", wasSucessful ? "succeeded" : "FAILED");

#ifdef XOJ_MEMORY_LEAK_CHECK_ENABLED
	xoj_momoryleak_printRemainingObjects();
#endif

#ifdef XOJ_CALL_LOG_ENABLED
	Log::closelog();
#endif

	// Return error code 1 if the one of test failed.
	return wasSucessful ? 0 : 1;
}
