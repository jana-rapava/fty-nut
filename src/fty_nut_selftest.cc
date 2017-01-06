/*  =========================================================================
    fty_nut_selftest.c - run selftests

    Runs all selftests.

    -------------------------------------------------------------------------
    Copyright (C) 2014 - 2017 Eaton                                        
                                                                           
    This program is free software; you can redistribute it and/or modify   
    it under the terms of the GNU General Public License as published by   
    the Free Software Foundation; either version 2 of the License, or      
    (at your option) any later version.                                    
                                                                           
    This program is distributed in the hope that it will be useful,        
    but WITHOUT ANY WARRANTY; without even the implied warranty of         
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          
    GNU General Public License for more details.                           
                                                                           
    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.            

################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################
    =========================================================================
*/

#include "fty_nut_classes.h"

typedef struct {
    const char *testname;
    void (*test) (bool);
} test_item_t;

static test_item_t
all_tests [] = {
// Tests for private classes:
// TODO: Per https://github.com/zeromq/zproject/commit/d44194965e5ccd61fa9d9d9a636a62581fa67adb
//       and https://github.com/zeromq/zproject/commit/e63ccee20f910f0656eda75fc69800b749549b84
//       such tests should not be exposed and rather be called
//       from the public class's self-test routine
#ifdef EXPOSE_PRIVATE_SELFTESTS
    { "logger", logger_test },
    { "fsutils", fsutils_test },
    { "cidr", cidr_test },
    { "nutscan", nutscan_test },
    { "subprocess", subprocess_test },
    { "actor_commands", actor_commands_test },
    { "ups_status", ups_status_test },
    { "nut_device", nut_device_test },
    { "nut_agent", nut_agent_test },
    { "nut_configurator", nut_configurator_test },
    { "alert_device", alert_device_test },
    { "alert_device_list", alert_device_list_test },
    { "nut", nut_test },
    { "stream", stream_test },
    { "sensor_device", sensor_device_test },
    { "sensor_list", sensor_list_test },
#endif
// Tests for stable public classes:
    { "fty_nut_server", fty_nut_server_test },
    { "fty_nut_configurator_server", fty_nut_configurator_server_test },
    { "alert_actor", alert_actor_test },
    { "sensor_actor", sensor_actor_test },
    {0, 0}          //  Sentinel
};

//  -------------------------------------------------------------------------
//  Test whether a test is available.
//  Return a pointer to a test_item_t if available, NULL otherwise.
//

test_item_t *
test_available (const char *testname)
{
    test_item_t *item;
    for (item = all_tests; item->test; item++) {
        if (streq (testname, item->testname))
            return item;
    }
    return NULL;
}

//  -------------------------------------------------------------------------
//  Run all tests.
//

static void
test_runall (bool verbose)
{
    test_item_t *item;
    printf ("Running fty-nut selftests...\n");
    for (item = all_tests; item->test; item++)
        item->test (verbose);

    printf ("Tests passed OK\n");
}

int
main (int argc, char **argv)
{
    bool verbose = false;
    test_item_t *test = 0;
    int argn;
    for (argn = 1; argn < argc; argn++) {
        if (streq (argv [argn], "--help")
        ||  streq (argv [argn], "-h")) {
            puts ("fty_nut_selftest.c [options] ...");
            puts ("  --verbose / -v         verbose test output");
            puts ("  --number / -n          report number of tests");
            puts ("  --list / -l            list all tests");
            puts ("  --test / -t [name]     run only test 'name'");
            puts ("  --continue / -c        continue on exception (on Windows)");
            return 0;
        }
        if (streq (argv [argn], "--verbose")
        ||  streq (argv [argn], "-v"))
            verbose = true;
        else
        if (streq (argv [argn], "--number")
        ||  streq (argv [argn], "-n")) {
            puts ("20");
            return 0;
        }
        else
        if (streq (argv [argn], "--list")
        ||  streq (argv [argn], "-l")) {
            puts ("Available tests (note that only stable classes are exposed to test):");
            puts ("    logger\t- stable private API; selftest is implemented");
            puts ("    fsutils\t- stable private API; selftest is implemented");
            puts ("    cidr\t- stable private API; selftest is implemented");
            puts ("    nutscan\t- stable private API; selftest is implemented");
            puts ("    subprocess\t- stable private API; selftest is implemented");
            puts ("    actor_commands\t- stable private API; selftest is implemented");
            puts ("    ups_status\t- stable private API; selftest is implemented");
            puts ("    nut_device\t- stable private API; selftest is implemented");
            puts ("    nut_agent\t- stable private API; selftest is implemented");
            puts ("    nut_configurator\t- stable private API; selftest is implemented");
            puts ("    alert_device\t- stable private API; selftest is implemented");
            puts ("    alert_device_list\t- stable private API; selftest is implemented");
            puts ("    nut\t- stable private API; selftest is implemented");
            puts ("    stream\t- stable private API; selftest is implemented");
            puts ("    sensor_device\t- stable private API; selftest is implemented");
            puts ("    sensor_list\t- stable private API; selftest is implemented");
            puts ("    fty_nut_server\t- stable public  API; selftest is implemented");
            puts ("    fty_nut_configurator_server\t- stable public  API; selftest is implemented");
            puts ("    alert_actor\t- stable public  API; selftest is implemented");
            puts ("    sensor_actor\t- stable public  API; selftest is implemented");
            return 0;
        }
        else
        if (streq (argv [argn], "--test")
        ||  streq (argv [argn], "-t")) {
            argn++;
            if (argn >= argc) {
                fprintf (stderr, "--test needs an argument\n");
                return 1;
            }
            test = test_available (argv [argn]);
            if (!test) {
                fprintf (stderr, "%s not valid, use --list to show tests\n", argv [argn]);
                return 1;
            }
        }
        else
        if (streq (argv [argn], "--continue")
        ||  streq (argv [argn], "-c")) {
#ifdef _MSC_VER
            //  When receiving an abort signal, only print to stderr (no dialog)
            _set_abort_behavior (0, _WRITE_ABORT_MSG);
#endif
        }
        else {
            printf ("Unknown option: %s\n", argv [argn]);
            return 1;
        }
    }
    if (test) {
        printf ("Running fty-nut test '%s'...\n", test->testname);
        test->test (verbose);
    }
    else
        test_runall (verbose);

    return 0;
}
/*
################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################
*/