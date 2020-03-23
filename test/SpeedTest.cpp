/*
 * Xournal++
 *
 * This file is part of the Xournal UnitTests
 * It's helper class for tests speed testing
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <ctime>

#include <StringUtils.h>
using std::clock;
#include <functional>
using std::function;
#include <iostream>
using std::cout;
using std::endl;
#include <string>

#include <zconf.h>

using std::string;

class SpeedTest {

public:
    void startTest(string target) {
        cout << endl << "== Speed test of " << target << " ==" << endl;
        this->target = target;
        begin = clock();
    }

    void endTest() {
        clock_t end = clock();

        printMemory();

        double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
        cout << "Time to " << target << ": " << std::to_string(elapsed_secs) << endl;
    }

private:
    clock_t begin;
    string target;

    static void printMemory() {
        string cmd = "bash -c \"cat /proc/";
        cmd += std::to_string(::getpid()) + "/status | grep Vm\"";
        if (system(cmd.c_str()) != 0) {
            cout << "Error executing " << cmd.c_str() << endl;
        };
    }
};
