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

# Start with some helpers.
class OutcomeCache:
    # This class holds test outcomes for configurations.  This avoids
    # running the same test twice.

    # The outcome cache is implemented as a tree.  Each node points
    # to the outcome of the remaining list.
    #
    # Example: ([1, 2, 3], PASS), ([1, 2], FAIL), ([1, 4, 5], FAIL):
    #
    #      (2, FAIL)--(3, PASS)
    #     /
    # (1, None)
    #     \
    #      (4, None)--(5, FAIL)

    def __init__(self):
        self.tail = {}                  # Points to outcome of tail
        self.result = None              # Result so far

    def add(self, c, result):
        """Add (C, RESULT) to the cache.  C must be a list of scalars."""
        cs = c[:]
        cs.sort()

        p = self
        for start in range(len(c)):
            if not c[start] in p.tail:
                p.tail[c[start]] = OutcomeCache()
            p = p.tail[c[start]]

        p.result = result

    def lookup(self, c):
        """Return RESULT if (C, RESULT) is in the cache; None, otherwise."""
        p = self
        for start in range(len(c)):
            if not c[start] in p.tail:
                return None
            p = p.tail[c[start]]

        return p.result

    def lookup_superset(self, c, start = 0):
        """Return RESULT if there is some (C', RESULT) in the cache with
        C' being a superset of C or equal to C.  Otherwise, return None."""

        # FIXME: Make this non-recursive!
        if start >= len(c):
            if self.result:
                return self.result
            elif self.tail != {}:
                # Select some superset
                superset = self.tail[self.tail.keys()[0]]
                return superset.lookup_superset(c, start + 1)
            else:
                return None

        if c[start] in self.tail:
            return self.tail[c[start]].lookup_superset(c, start + 1)

        # Let K0 be the largest element in TAIL such that K0 <= C[START]
        k0 = None
        for k in self.tail.keys():
            if (k0 == None or k > k0) and k <= c[start]:
                k0 = k

        if k0 != None:
            return self.tail[k0].lookup_superset(c, start)

        return None

    def lookup_subset(self, c):
        """Return RESULT if there is some (C', RESULT) in the cache with
        C' being a subset of C or equal to C.  Otherwise, return None."""
        p = self
        for start in range(len(c)):
            if c[start] in p.tail:
                p = p.tail[c[start]]

        return p.result




# Test the outcome cache
def oc_test():
    oc = OutcomeCache()

    assert oc.lookup([1, 2, 3]) == None
    oc.add([1, 2, 3], 4)
    assert oc.lookup([1, 2, 3]) == 4
    assert oc.lookup([1, 2, 3, 4]) == None

    assert oc.lookup([5, 6, 7]) == None
    oc.add([5, 6, 7], 8)
    assert oc.lookup([5, 6, 7]) == 8

    assert oc.lookup([]) == None
    oc.add([], 0)
    assert oc.lookup([]) == 0

    assert oc.lookup([1, 2]) == None
    oc.add([1, 2], 3)
    assert oc.lookup([1, 2]) == 3
    assert oc.lookup([1, 2, 3]) == 4

    assert oc.lookup_superset([1]) == 3 or oc.lookup_superset([1]) == 4
    assert oc.lookup_superset([1, 2]) == 3 or oc.lookup_superset([1, 2]) == 4
    assert oc.lookup_superset([5]) == 8
    assert oc.lookup_superset([5, 6]) == 8
    assert oc.lookup_superset([6, 7]) == 8
    assert oc.lookup_superset([7]) == 8
    assert oc.lookup_superset([]) != None

    assert oc.lookup_superset([9]) == None
    assert oc.lookup_superset([7, 9]) == None
    assert oc.lookup_superset([-5, 1]) == None
    assert oc.lookup_superset([1, 2, 3, 9]) == None
    assert oc.lookup_superset([4, 5, 6, 7]) == None

    assert oc.lookup_subset([]) == 0
    assert oc.lookup_subset([1, 2, 3]) == 4
    assert oc.lookup_subset([1, 2, 3, 4]) == 4
    assert oc.lookup_subset([1, 3]) == None
    assert oc.lookup_subset([1, 2]) == 3

    assert oc.lookup_subset([-5, 1]) == None
    assert oc.lookup_subset([-5, 1, 2]) == 3
    assert oc.lookup_subset([-5]) == 0


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

    # Resolving directions.
    ADD    = "ADD"			# Add deltas to resolve
    REMOVE = "REMOVE"			# Remove deltas to resolve

    # Debugging output (set to 1 to enable)
    debug_test      = 0
    debug_dd        = 0
    debug_split     = 0
    debug_resolve   = 0

    def __init__(self):
        self.__resolving = 0
        self.__last_reported_length = 0
        self.monotony = 0
        self.outcome_cache  = OutcomeCache()
        self.cache_outcomes = 1
        self.minimize = 1
        self.maximize = 1
        self.assume_axioms_hold = 1

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

    # Testing
    def test(self, c):
        """Test the configuration C.  Return PASS, FAIL, or UNRESOLVED"""
        #c.sort()

        # If we had this test before, return its result
        if self.cache_outcomes:
            cached_result = self.outcome_cache.lookup(c)
            if cached_result != None:
                return cached_result

        if self.monotony:
            # Check whether we had a passing superset of this test before
            cached_result = self.outcome_cache.lookup_superset(c)
            if cached_result == self.PASS:
                return self.PASS

            cached_result = self.outcome_cache.lookup_subset(c)
            if cached_result == self.FAIL:
                return self.FAIL

        if self.debug_test:
            print()
            print("test(" + self.coerce(c) + ")...")

        outcome = self._test(c)

        if self.debug_test:
            print("test(" + self.coerce(c) + ") = " + repr(outcome))

        if self.cache_outcomes:
            self.outcome_cache.add(c, outcome)

        return outcome

    def _test(self, c):
        """Stub to overload in subclasses"""
        return self.UNRESOLVED		# Placeholder


    # Splitting
    def split(self, c, n):
        """Split C into [C_1, C_2, ..., C_n]."""
        if self.debug_split:
            print("split(" + self.coerce(c) + ", " + repr(n) + ")...")

        outcome = self._split(c, n)

        if self.debug_split:
            print( "split(" + self.coerce(c) + ", " + repr(n) + ") = " + repr(outcome))

        return outcome

    def _split(self, c, n):
        """Stub to overload in subclasses"""
        subsets = []
        start = 0
        for i in range(n):
            subset = c[start:start + (len(c) - start) // (n - i)]
            subsets.append(subset)
            start = start + len(subset)
        return subsets


    # Resolving
    def resolve(self, csub, c, direction):
        """If direction == ADD, resolve inconsistency by adding deltas
        to CSUB.  Otherwise, resolve by removing deltas from CSUB."""

        if self.debug_resolve:
            print("resolve(" + repr(csub) + ", " + self.coerce(c) + ", " + \
                repr(direction) + ")...")

        outcome = self._resolve(csub, c, direction)

        if self.debug_resolve:
            print("resolve(" + repr(csub) + ", " + self.coerce(c) + ", " + \
                  repr(direction) + ") = " + repr(outcome))

        return outcome


    def _resolve(self, csub, c, direction):
        """Stub to overload in subclasses."""
        # By default, no way to resolve
        return None


    # Test with fixes
    def test_and_resolve(self, csub, r, c, direction):
        """Repeat testing CSUB + R while unresolved."""

        initial_csub = csub[:]
        c2 = self.__listunion(r, c)

        csubr = self.__listunion(csub, r)
        t = self.test(csubr)

        # necessary to use more resolving mechanisms which can reverse each
        # other, can (but needn't) be used in subclasses
        self._resolve_type = 0

        while t == self.UNRESOLVED:
            self.__resolving = 1
            csubr = self.resolve(csubr, c, direction)

            if csubr == None:
                # Nothing left to resolve
                break

            if len(csubr) >= len(c2):
                # Added everything: csub == c2. ("Upper" Baseline)
                # This has already been tested.
                csubr = None
                break

            if len(csubr) <= len(r):
                # Removed everything: csub == r. (Baseline)
                # This has already been tested.
                csubr = None
                break

            t = self.test(csubr)

        self.__resolving = 0
        if csubr == None:
            return self.UNRESOLVED, initial_csub

        # assert t == self.PASS or t == self.FAIL
        csub = self.__listminus(csubr, r)
        return t, csub

    # Inquiries
    def resolving(self):
        """Return 1 while resolving."""
        return self.__resolving


    # Logging
    def report_progress(self, c, title):
        if len(c) != self.__last_reported_length:
            print()
            print(title + ": " + repr(len(c)) + " deltas left:", self.coerce(c))
            self.__last_reported_length = len(c)


    # Delta Debugging (old ESEC/FSE version)
    def old_dd(self, c, r = [], n = 2):
        """Return the failure-inducing subset of C"""

        assert self.test([]) == dd.PASS
        assert self.test(c)  == dd.FAIL

        if self.debug_dd:
            print ("dd(" + self.pretty(c) + ", " + repr(r) + ", " + repr(n) + ")...")

        outcome = self._old_dd(c, r, n)

        if self.debug_dd:
            print ("dd(" + self.pretty(c) + ", " + repr(r) + ", " + repr(n) +
                   ") = " + repr(outcome))

        return outcome



    def test_mix(self, csub, c, direction):
        if self.minimize:
            (t, csub) = self.test_and_resolve(csub, [], c, direction)
            if t == self.FAIL:
                return (t, csub)

        if self.maximize:
            csubbar = self.__listminus(self.CC, csub)
            cbar    = self.__listminus(self.CC, c)
            if direction == self.ADD:
                directionbar = self.REMOVE
            else:
                directionbar = self.ADD

            (tbar, csubbar) = self.test_and_resolve(csubbar, [], cbar,
                                                    directionbar)

            csub = self.__listminus(self.CC, csubbar)

            if tbar == self.PASS:
                t = self.FAIL
            elif tbar == self.FAIL:
                t = self.PASS
            else:
                t = self.UNRESOLVED

        return (t, csub)



    # Delta Debugging (new ISSTA version)
    def ddgen(self, c, minimize, maximize):
        """Return a 1-minimal failing subset of C"""

        self.minimize = minimize
        self.maximize = maximize

        n = 2
        self.CC = c

        if self.debug_dd:
            print ("dd(" + self.pretty(c) + ", " + repr(n) + ")...")

        outcome = self._dd(c, n)

        if self.debug_dd:
            print ("dd(" + self.pretty(c) + ", " + repr(n) + ") = " + repr(outcome))

        return outcome

    def _dd(self, c, n):
        """Stub to overload in subclasses"""

        testNoDelta=self.test([])
        if testNoDelta!=self.PASS:
            self.internalError("_dd", "ERROR: test([]) == FAILED")

#        assert self.test([]) == self.PASS

        run = 1
        cbar_offset = 0

        # We replace the tail recursion from the paper by a loop
        while 1:
            tc = self._test(c)
            if tc != self.FAIL and tc != self.UNRESOLVED:
                self.internalError("_dd","test([all deltas]) == PASS")


            if n > len(c):
                # No further minimizing
                print ("dd: done")
                return c

            self.report_progress(c, "dd")

            cs = self.split(c, n)

            print ()
            print ("dd (run #" + repr(run) + "): trying", "+".join([repr(len(cs[i])) for i in range(n)] ) )

            c_failed    = 0
            cbar_failed = 0

            next_c = c[:]
            next_n = n

            # Check subsets
            for i in range(n):
                if self.debug_dd:
                    print ("dd: trying", self.pretty(cs[i]))

                (t, cs[i]) = self.test_mix(cs[i], c, self.REMOVE)

                if t == self.FAIL:
                    # Found
                    if self.debug_dd:
                        print ("dd: found", len(cs[i]), "deltas:",)
                        print (self.pretty(cs[i]))

                    c_failed = 1
                    next_c = cs[i]
                    next_n = 2
                    cbar_offset = 0
                    self.report_progress(next_c, "dd")
                    break

            if not c_failed:
                # Check complements
                cbars = n * [self.UNRESOLVED]

                # print "cbar_offset =", cbar_offset

                for j in range(n):
                    i = (j + cbar_offset) % n
                    cbars[i] = self.__listminus(c, cs[i])
                    t, cbars[i] = self.test_mix(cbars[i], c, self.ADD)

                    doubled = self.__listintersect(cbars[i], cs[i])
                    if doubled != []:
                        cs[i] = self.__listminus(cs[i], doubled)

                    if t == self.FAIL:
                        if self.debug_dd:
                            print ("dd: reduced to", len(cbars[i]),)
                            print ("deltas:", end="")
                            print (self.pretty(cbars[i]))

                        cbar_failed = 1
                        next_c = self.__listintersect(next_c, cbars[i])
                        next_n = next_n - 1
                        self.report_progress(next_c, "dd")

                        # In next run, start removing the following subset
                        cbar_offset = i
                        break

            if not c_failed and not cbar_failed:
                if n >= len(c):
                    # No further minimizing
                    print ("dd: done")
                    return c

                next_n = min(len(c), n * 2)
                print ("dd: increase granularity to", next_n)
                cbar_offset = (cbar_offset * next_n) // n

            c = next_c
            n = next_n
            run = run + 1

    def verrou_dd_max(self, c):
        """Stub to overload in subclasses"""
        self.maximize=1
        self.minimize=0
        n = 2
        self.CC = c
        algo_name="dd_max"

        testNoDelta=self.test([])
        if testNoDelta!=self.PASS:
            self.internalError("verrou_dd_max","ERROR: test([]) == FAILED")

        run = 1
        cbar_offset = 0

        # We replace the tail recursion from the paper by a loop
        while 1:
            tc = self.test(c)
            if tc != self.FAIL and tc != self.UNRESOLVED:
                self.internalError("verrou_dd_max","test([all deltas]) == PASS")


            if n > len(c):
                # No further minimizing
                print (algo_name+": done")
                return c

            self.report_progress(c, algo_name)

            cs = self.split(c, n)

            print ()
            print (algo_name+" (run #" + repr(run) + "): trying", "+".join([repr(len(cs[i])) for i in range(n)] ) )

            c_failed    = 0
            cbar_failed = 0

            next_c = c[:]
            next_n = n


            if not c_failed:
                # Check complements
                cbars = n * [self.UNRESOLVED]

                # print "cbar_offset =", cbar_offset

                for j in range(n):
                    i = (j + cbar_offset) % n
                    cbars[i] = self.__listminus(c, cs[i])
                    t, cbars[i] = self.test_mix(cbars[i], c, self.ADD)

                    doubled = self.__listintersect(cbars[i], cs[i])
                    if doubled != []:
                        cs[i] = self.__listminus(cs[i], doubled)

                    if t == self.FAIL:
                        if self.debug_dd:
                            print (algo_name+": reduced to", len(cbars[i]),)
                            print ("deltas:", end="")
                            print (self.pretty(cbars[i]))

                        cbar_failed = 1
                        next_c = self.__listintersect(next_c, cbars[i])
                        next_n = next_n - 1
                        self.report_progress(next_c, algo_name)

                        # In next run, start removing the following subset
                        cbar_offset = i
                        break

            if not c_failed and not cbar_failed:
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

        testNoDelta=self._test([],nbRun)
        if testNoDelta!=self.PASS:
            self.internalError("verrou_dd_min","ERROR: test([]) == FAILED")

        run = 1
        cbar_offset = 0

        # We replace the tail recursion from the paper by a loop
        while 1:
            tc = self._test(c ,nbRun)
            if tc != self.FAIL and tc != self.UNRESOLVED:
                self.internalError("verrou_dd_min","ERROR: test([all deltas]) == PASS")


            if n > len(c):
                # No further minimizing
                print (algo_name+": done")
                return c

            self.report_progress(c, algo_name)

            cs = self.split(c, n)

            print ()
            print (algo_name+" (run #" + repr(run) + "): trying", "+".join([repr(len(cs[i])) for i in range(n)] ) )

            c_failed    = False
            cbar_failed = False

            next_c = c[:]
            next_n = n

            # Check subsets
            for i in range(n):
                if self.debug_dd:
                    print (algo_name+": trying", self.pretty(cs[i]))

                t = self._test(cs[i],nbRun)

                if t == self.FAIL:
                    # Found
                    if self.debug_dd:
                        print (algo_name+": found", len(cs[i]), "deltas:",)
                        print (self.pretty(cs[i]))

                    c_failed = True
                    next_c = cs[i]
                    next_n = 2
                    cbar_offset = 0
                    self.report_progress(next_c, algo_name)
                    break

            if not c_failed:
                # Check complements
                cbars = n * [self.UNRESOLVED]

                # print "cbar_offset =", cbar_offset

                for j in range(n):
                    i = (j + cbar_offset) % n
                    cbars[i] = self.__listminus(c, cs[i])
                    t = self._test(cbars[i],nbRun)

                    if t == self.FAIL:
                        if self.debug_dd:
                            print (algo_name+": reduced to", len(cbars[i]),)
                            print ("deltas:", end="")
                            print (self.pretty(cbars[i]))

                        cbar_failed = True
                        next_c = cbars[i]
                        next_n = next_n - 1
                        self.report_progress(next_c, algo_name)

                        # In next run, start removing the following subset
                        cbar_offset = i
                        break

            if not c_failed and not cbar_failed:
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

    def verrou_dd_min_par(self, c , nbRun):
        """Stub to overload in subclasses"""
        n = 2
        algo_name="ddmin//"

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
            tTab=self._testTab(cs, [nbRun]*len(cs), earlyExit=True, firstConfFail=True)

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
                tTab = self._testTab(cbars,[nbRun]*n,  earlyExit=True, firstConfFail=True)
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

    def ddmin(self, c):
        return self.ddgen(c, 1, 0)

    def ddmax(self, c):
        return self.ddgen(c, 0, 1)

    def ddmix(self, c):
        return self.ddgen(c, 1, 1)

    def internalError(self, func, msg):
        raise AssertionError(func +"\t"+ msg)

    # General delta debugging (new TSE version)
    def dddiff(self, c):
        n = 2

        if self.debug_dd:
            print ("dddiff(" + self.pretty(c) + ", " + repr(n) + ")...")

        outcome = self._dddiff([], c, n)

        if self.debug_dd:
            print ("dddiff(" + self.pretty(c) + ", " + repr(n) + ") = " +
                   repr(outcome))

        return outcome

    def _dddiff(self, c1, c2, n):
        run = 1
        cbar_offset = 0

        # We replace the tail recursion from the paper by a loop
        while 1:
            if self.debug_dd:
                print ("dd: c1 =", self.pretty(c1))
                print ("dd: c2 =", self.pretty(c2))

            if self.assume_axioms_hold:
                t1 = self.PASS
                t2 = self.FAIL
            else:
                t1 = self.test(c1)
                t2 = self.test(c2)

            assert t1 == self.PASS
            assert t2 == self.FAIL
            assert self.__listsubseteq(c1, c2)

            c = self.__listminus(c2, c1)

            if self.debug_dd:
                print ("dd: c2 - c1 =", self.pretty(c))

            if n > len(c):
                # No further minimizing
                print ("dd: done")
                return (c, c1, c2)

            self.report_progress(c, "dd")

            cs = self.split(c, n)

            print ()
            print ("dd (run #" + repr(run) + "): trying",)
            for i in range(n):
                if i > 0:
                    print ("+",)
                print (len(cs[i]),)
            print ()

            progress = 0

            next_c1 = c1[:]
            next_c2 = c2[:]
            next_n = n

            # Check subsets
            for j in range(n):
                i = (j + cbar_offset) % n

                if self.debug_dd:
                    print ("dd: trying", self.pretty(cs[i]))

                (t, csub) = self.test_and_resolve(cs[i], c1, c, self.REMOVE)
                csub = self.__listunion(c1, csub)

                if t == self.FAIL and t1 == self.PASS:
                    # Found
                    progress    = 1
                    next_c2     = csub
                    next_n      = 2
                    cbar_offset = 0

                    if self.debug_dd:
                        print ("dd: reduce c2 to", len(next_c2), "deltas:",)
                        print (self.pretty(next_c2))
                    break

                if t == self.PASS and t2 == self.FAIL:
                    # Reduce to complement
                    progress    = 1
                    next_c1     = csub
                    next_n      = max(next_n - 1, 2)
                    cbar_offset = i

                    if self.debug_dd:
                        print ("dd: increase c1 to", len(next_c1), "deltas:",)
                        print (self.pretty(next_c1))
                    break


                csub = self.__listminus(c, cs[i])
                (t, csub) = self.test_and_resolve(csub, c1, c, self.ADD)
                csub = self.__listunion(c1, csub)

                if t == self.PASS and t2 == self.FAIL:
                    # Found
                    progress    = 1
                    next_c1     = csub
                    next_n      = 2
                    cbar_offset = 0

                    if self.debug_dd:
                        print ("dd: increase c1 to", len(next_c1), "deltas:",)
                        print (self.pretty(next_c1))
                    break

                if t == self.FAIL and t1 == self.PASS:
                    # Increase
                    progress    = 1
                    next_c2     = csub
                    next_n      = max(next_n - 1, 2)
                    cbar_offset = i

                    if self.debug_dd:
                        print ("dd: reduce c2 to", len(next_c2), "deltas:",)
                        print (self.pretty(next_c2))
                    break

            if progress:
                self.report_progress(self.__listminus(next_c2, next_c1), "dd")
            else:
                if n >= len(c):
                    # No further minimizing
                    print ("dd: done")
                    return (c, c1, c2)

                next_n = min(len(c), n * 2)
                print ("dd: increase granularity to", next_n)
                cbar_offset = (cbar_offset * next_n) // n

            c1  = next_c1
            c2  = next_c2
            n   = next_n
            run = run + 1

    def dd(self, c):
        return self.dddiff(c)           # Backwards compatibility




if __name__ == '__main__':
    # Test the outcome cache
    oc_test()

    # Define our own DD class, with its own test method
    class MyDD(DD):
        def _test_a(self, c):
            "Test the configuration C.  Return PASS, FAIL, or UNRESOLVED."

            # Just a sample
            # if 2 in c and not 3 in c:
            #	return self.UNRESOLVED
            # if 3 in c and not 7 in c:
            #   return self.UNRESOLVED
            if 7 in c and not 2 in c:
                return self.UNRESOLVED
            if 5 in c and 8 in c:
                return self.FAIL
            return self.PASS

        def _test_b(self, c):
            if c == []:
                return self.PASS
            if 1 in c and 2 in c and 3 in c and 4 in c and \
               5 in c and 6 in c and 7 in c and 8 in c:
                return self.FAIL
            return self.UNRESOLVED

        def _test_c(self, c):
            if 1 in c and 2 in c and 3 in c and 4 in c and \
               6 in c and 8 in c:
                if 5 in c and 7 in c:
                    return self.UNRESOLVED
                else:
                    return self.FAIL
            if 1 in c or 2 in c or 3 in c or 4 in c or \
               6 in c or 8 in c:
                return self.UNRESOLVED
            return self.PASS

        def __init__(self):
            self._test = self._test_c
            DD.__init__(self)


    print ("WYNOT - a tool for delta debugging.")
    mydd = MyDD()
    # mydd.debug_test     = 1			# Enable debugging output
    # mydd.debug_dd       = 1			# Enable debugging output
    # mydd.debug_split    = 1			# Enable debugging output
    # mydd.debug_resolve  = 1			# Enable debugging output

    # mydd.cache_outcomes = 0
    # mydd.monotony = 0

    print ("Minimizing failure-inducing input...")
    c = mydd.ddmin([1, 2, 3, 4, 5, 6, 7, 8])  # Invoke DDMIN
    print ("The 1-minimal failure-inducing input is", c)
    print ("Removing any element will make the failure go away.")
    print ()

    print ("Computing the failure-inducing difference...")
    (c, c1, c2) = mydd.dd([1, 2, 3, 4, 5, 6, 7, 8])	# Invoke DD
    print ("The 1-minimal failure-inducing difference is", c)
    print (c1, "passes,", c2, "fails")



# Local Variables:
# mode: python
# End:
