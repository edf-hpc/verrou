#!/usr/bin/env python3

# $Id: DD.py,v 1.2 2001/11/05 19:53:33 zeller Exp $
# Enhanced Delta Debugging class
# Copyright (c) 1999, 2000, 2001 Andreas Zeller.

# This module (written in Python) implements the base delta debugging
# algorithms and is at the core of all our experiments.  This should
# easily run on any platform and any Python version since 1.6.
#
# To plug this into your system, all you have to do is to create a
# subclass with a dedicated `test()' method.  Basically, you would
# invoke the DD test case minimization algorithm (= the `ddmin()'
# method) with a list of characters; the `test()' method would combine
# them to a document and run the test.  This should be easy to realize
# and give you some good starting results; the file includes a simple
# sample application.
#
# This file is in the public domain; feel free to copy, modify, use
# and distribute this software as you wish - with one exception.
# Passau University has filed a patent for the use of delta debugging
# on program states (A. Zeller: `Isolating cause-effect chains',
# Saarland University, 2001).  The fact that this file is publicly
# available does not imply that I or anyone else grants you any rights
# related to this patent.
#
# The use of Delta Debugging to isolate failure-inducing code changes
# (A. Zeller: `Yesterday, my program worked', ESEC/FSE 1999) or to
# simplify failure-inducing input (R. Hildebrandt, A. Zeller:
# `Simplifying failure-inducing input', ISSTA 2000) is, as far as I
# know, not covered by any patent, nor will it ever be.  If you use
# this software in any way, I'd appreciate if you include a citation
# such as `This software uses the delta debugging algorithm as
# described in (insert one of the papers above)'.
#
# All about Delta Debugging is found at the delta debugging web site,
#
#               http://www.st.cs.uni-sb.de/dd/
#
# Happy debugging,
#
# Andreas Zeller

import sys
import os


# Main Delta Debugging algorithm.
class DD:
    # Delta debugging base class.  To use this class for a particular
    # setting, create a subclass with an overloaded `test()' method.
    #
    # Main entry points are:
    # - `ddmin()' which computes a minimal failure-inducing configuration, and
    # - `dd()' which computes a minimal failure-inducing difference.
    #
    # See also the usage sample at the end of this file.
    #
    # For further fine-tuning, you can implement an own `resolve()'
    # method (tries to add or remove configuration elements in case of
    # inconsistencies), or implement an own `split()' method, which
    # allows you to split configurations according to your own
    # criteria.
    # 
    # The class includes other previous delta debugging alorithms,
    # which are obsolete now; they are only included for comparison
    # purposes.

    # Test outcomes.
    PASS       = "PASS"
    FAIL       = "FAIL"
    UNRESOLVED = "UNRESOLVED"


    # Debugging output (set to 1 to enable)
    debug_test      = 0
    debug_dd        = 0

    def __init__(self):
        pass


    # Helpers
    def __listminus(self, c1, c2):
        """Return a list of all elements of C1 that are not in C2."""
        s2 = {}
        for delta in c2:
            s2[delta] = 1

        c = []
        for delta in c1:
            if not delta in s2:
                c.append(delta)

        return c

    def __listintersect(self, c1, c2):
        """Return the common elements of C1 and C2."""
        s2 = {}
        for delta in c2:
            s2[delta] = 1

        c = []
        for delta in c1:
            if delta in s2:
                c.append(delta)

        return c

    def __listunion(self, c1, c2):
        """Return the union of C1 and C2."""
        s1 = {}
        for delta in c1:
            s1[delta] = 1

        c = c1[:]
        for delta in c2:
            if not delta in s1:
                c.append(delta)

        return c

    def __listsubseteq(self, c1, c2):
        """Return 1 if C1 is a subset or equal to C2."""
        s2 = {}
        for delta in c2:
            s2[delta] = 1

        for delta in c1:
            if not delta in s2:
                return 0

        return 1

    # Output
    def coerce(self, c):
        """Return the configuration C as a compact string"""
        # Default: use printable representation
        return repr(c)

    def pretty(self, c):
        """Like coerce(), but sort beforehand"""
        sorted_c = c[:]
        sorted_c.sort()
        return self.coerce(sorted_c)


    def _test(self, c):
        """Stub to overload in subclasses"""
        return self.UNRESOLVED		# Placeholder

    def split(self, c, n):
        """Stub to overload in subclasses"""
        subsets = []
        start = 0
        for i in range(n):
            subset = c[start:start + (len(c) - start) // (n - i)]
            subsets.append(subset)
            start = start + len(subset)
        return subsets


    # Logging
    def report_progress(self, c, title):
        if len(c) != self.__last_reported_length:
            print()
            print(title + ": " + repr(len(c)) + " deltas left:", self.coerce(c))
            self.__last_reported_length = len(c)


    def verrou_dd_max(self, c, nbRun):
        """Stub to overload in subclasses"""
        n = 2
        self.CC = c
        algo_name="dd_max"

        testNoDelta=self._test([], nbRun)
        if testNoDelta!=self.PASS:
            self.internalError(algo_name,"ERROR: test([]) == FAILED")

        run = 1
        cbar_offset = 0

        # We replace the tail recursion from the paper by a loop
        while True:#1:
            tc = self._test(c,nbRun)
            if tc == self.PASS:
                self.internalError("verrou_dd_max","test([all deltas]) == PASS")

            if n > len(c):
                # No further minimizing
                print (algo_name+": done")
                return c

            self.report_progress(c, algo_name)

            cs = self.split(c, n)

            print ()
            print (algo_name+" (run #" + repr(run) + "): trying", "+".join([repr(len(cs[i])) for i in range(n)] ) )

            cbar_pass = False

            next_c = c[:]
            next_n = n

            # Check complements
            cbars = n * [None]
            cbarsTestTab=n*[None]

            # print "cbar_offset =", cbar_offset

            for j in range(n):
                i = (j + cbar_offset) % n
                cbars[i] = self.__listminus(c, cs[i])
                cbarsTestTab[i]=self.__listminus(self.CC,cbars[i])

            tTab=self._testTab(cbarsTestTab, [nbRun]*len(cs), earlyExit=True, firstConfFail=False, firstConfPass=True, sortOrder="triangle")

            for j in range(n):
                i = (j + cbar_offset) % n
#                cbars[i] = self.__listminus(c, cs[i])

                t=tTab[i]

                if t == self.PASS:
                    if self.debug_dd:
                        print (algo_name+": reduced to", len(cbars[i]),)
                        print ("deltas:", end="")
                        print (self.pretty(cbars[i]))

                    cbar_pass = True
                    next_c = self.__listintersect(next_c, cbars[i])
                    next_n = next_n - 1
                    self.report_progress(next_c, algo_name)

                    # In next run, start removing the following subset
                    cbar_offset = i
                    break

            if not cbar_pass:
                if n >= len(c):
                    # No further minimizing
                    print (algo_name+": done")
                    return c

                next_n = min(len(c), n * 2)
                print (algo_name+": increase granularity to", next_n)
                cbar_offset = (cbar_offset * next_n) // n

            c = next_c
            n = next_n
            run = run + 1


    def verrou_dd_min(self, c , nbRun):
        """Stub to overload in subclasses"""
        n = 2
        algo_name="ddmin"
        sortOrder="outerConfInnerSample"
        if self.config_.get_use_dd_min_par():
            algo_name="ddmin//"
            sortOrder="outerSampleInnerConf"

        testNoDelta=self._test([],nbRun)
        if testNoDelta!=self.PASS:
            self.internalError("verrou_dd_min_par","ERROR: test([]) == FAILED")

        run = 1
        cbar_offset = 0

        failMemory=[]

        # We replace the tail recursion from the paper by a loop
        while 1:
            tc = self._test(c ,nbRun)
            if tc != self.FAIL and tc != self.UNRESOLVED:
                self.internalError("verrou_dd_min","ERROR: test([all deltas]) == PASS")


            if n > len(c):
                # No further minimizing
                print (algo_name+": done")
                return (c,failMemory)

            self.report_progress(c, algo_name)

            cs = self.split(c, n)

            print ()
            print (algo_name+" (run #" + repr(run) + "): trying", "+".join([repr(len(cs[i])) for i in range(n)] ) )

            c_failed    = False
            cbar_failed = False

            next_c = c[:]
            next_n = n

            # // resolution
            tTab=self._testTab(cs, [nbRun]*len(cs), earlyExit=True, firstConfFail=True, sortOrder=sortOrder)

            for i in range(n):
                if self.debug_dd:
                    print (algo_name+": trying", self.pretty(cs[i]))

                t = tTab[i]

                if t == self.FAIL:
                    # Found
                    if self.debug_dd:
                        print (algo_name+": found", len(cs[i]), "deltas:",)
                        print (self.pretty(cs[i]))

                    c_failed = True
                    next_c = cs[i]
                    next_n = 2
                    cbar_offset = 0
                    for j in range(i+1,n):
                        t = tTab[j]
                        if t == self.FAIL:
                            failMemory+=[cs[j]]
                    self.report_progress(next_c, algo_name)
                    break

            if not c_failed and n!=2:
                # Check complements
                cbars = n * [self.UNRESOLVED]

                # print "cbar_offset =", cbar_offset

                for j in range(n):
                    i = (j + cbar_offset) % n
                    cbars[i] = self.__listminus(c, cs[i])
                tTab = self._testTab(cbars,[nbRun]*n, earlyExit=True, firstConfFail=True, sortOrder=sortOrder)
                for i in range(n):
                    t=tTab[i]
                    if t == self.FAIL:
                        if self.debug_dd:
                            print (algo_name+": reduced to", len(cbars[i]),)
                            print ("deltas:", end="")
                            print (self.pretty(cbars[i]))

                        cbar_failed = True
                        next_c = cbars[i]
                        next_n = next_n - 1

                        for j in range(i+1,n):
                            t = tTab[j]
                            if t == self.FAIL:
                                failMemory+=[cbars[j]]

                        self.report_progress(next_c, algo_name)

                        # In next run, start removing the following subset
                        cbar_offset = i
                        break

            if not c_failed and not cbar_failed:
                if n >= len(c):
                    # No further minimizing
                    print (algo_name+": done")
                    return (c,failMemory)

                next_n = min(len(c), n * 2)
                print (algo_name+": increase granularity to", next_n)
                cbar_offset = (cbar_offset * next_n) // n

            c = next_c
            n = next_n
            run = run + 1


# Local Variables:
# mode: python
# End:
