
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Test Suite                                                            *
 *                                                                        *
 *  Copyright (c) 1999-2013, Ben Burton                                   *
 *  For further details contact Ben Burton (bab@debian.org).              *
 *                                                                        *
 *  This program is free software; you can redistribute it and/or         *
 *  modify it under the terms of the GNU General Public License as        *
 *  published by the Free Software Foundation; either version 2 of the    *
 *  License, or (at your option) any later version.                       *
 *                                                                        *
 *  As an exception, when this program is distributed through (i) the     *
 *  App Store by Apple Inc.; (ii) the Mac App Store by Apple Inc.; or     *
 *  (iii) Google Play by Google Inc., then that store may impose any      *
 *  digital rights management, device limits and/or redistribution        *
 *  restrictions that are required by its terms of service.               *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful, but   *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *  General Public License for more details.                              *
 *                                                                        *
 *  You should have received a copy of the GNU General Public             *
 *  License along with this program; if not, write to the Free            *
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,       *
 *  MA 02110-1301, USA.                                                   *
 *                                                                        *
 **************************************************************************/

/* end stub */

#include <sstream>
#include <cppunit/extensions/HelperMacros.h>
#include "census/ncensus.h"
#include "packet/ncontainer.h"
#include "testsuite/census/testcensus.h"

using regina::NBoolSet;
using regina::NCensus;
using regina::NContainer;

class NCensusTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(NCensusTest);

    CPPUNIT_TEST(rawCounts);
    CPPUNIT_TEST(rawCountsCompact);
    CPPUNIT_TEST(rawCountsPrimeMinimalOr);
    CPPUNIT_TEST(rawCountsPrimeMinimalNor);
    CPPUNIT_TEST(rawCountsBounded);
    CPPUNIT_TEST(rawCountsHypMin);

    CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        }

        void tearDown() {
        }

        void rawCounts() {
            unsigned nAll[] = { 1, 5, 61, 1581 };
            rawCountsCompare(1, 3, nAll, "closed/ideal",
                NBoolSet::sBoth, NBoolSet::sBoth, NBoolSet::sFalse,
                0, 0, false);

            unsigned nOrientable[] = { 1, 4, 35, 454, 13776 };
            rawCountsCompare(1, 3, nOrientable, "closed/ideal orbl",
                NBoolSet::sBoth, NBoolSet::sTrue, NBoolSet::sFalse,
                0, 0, false);
        }

        void rawCountsCompact() {
            unsigned nAll[] = { 1, 4, 17, 81, 577, 5184, 57753 };
            rawCountsCompare(1, 4, nAll, "closed compact",
                NBoolSet::sTrue, NBoolSet::sBoth, NBoolSet::sFalse,
                0, 0, false);

            unsigned nOrientable[] = { 1, 4, 16, 76, 532, 4807, 52946 };
            rawCountsCompare(1, 4, nOrientable, "closed compact orbl",
                NBoolSet::sTrue, NBoolSet::sTrue, NBoolSet::sFalse,
                0, 0, false);
        }

        void rawCountsPrimeMinimalOr() {
            unsigned nOrientable[] = { 1, 4, 11, 7, 17, 50 };
            rawCountsCompare(1, 4, nOrientable, "closed orbl prime minimal",
                NBoolSet::sTrue, NBoolSet::sTrue, NBoolSet::sFalse, 0,
                NCensus::PURGE_NON_MINIMAL_PRIME, true);
        }

        void rawCountsPrimeMinimalNor() {
            unsigned nNonOrientable[] = { 0, 0, 1, 0, 2, 4 };
            rawCountsCompare(1, 4, nNonOrientable,
                "closed non-orbl prime minimal P2-irreducible",
                NBoolSet::sTrue, NBoolSet::sFalse, NBoolSet::sFalse, 0,
                NCensus::PURGE_NON_MINIMAL_PRIME |
                NCensus::PURGE_P2_REDUCIBLE, true);
        }

        void rawCountsBounded() {
            unsigned nAll[] = { 1, 3, 17, 156, 2308 };
            rawCountsCompare(1, 3, nAll, "bounded compact",
                NBoolSet::sTrue, NBoolSet::sBoth, NBoolSet::sTrue,
                -1, 0, false);

            unsigned nOrientable[] = { 1, 3, 14, 120, 1531 };
            rawCountsCompare(1, 3, nOrientable, "bounded compact orbl",
                NBoolSet::sTrue, NBoolSet::sTrue, NBoolSet::sTrue,
                -1, 0, false);
        }

        void rawCountsHypMin() {
            // Enforced: all vertices torus/KB, no low-degree edges.
            unsigned nAll[] = { 1, 1, 7, 31, 224, 1075, 6348 };
            rawCountsCompare(1, 4, nAll, "candidate minimal cusped hyperbolic",
                NBoolSet::sFalse, NBoolSet::sBoth, NBoolSet::sFalse, -1,
                NCensus::PURGE_NON_MINIMAL_HYP, false);

            unsigned nOrientable[] = { 1, 0, 3, 14, 113, 590, 3481 };
            rawCountsCompare(1, 5, nOrientable,
                "candidate minimal cusped hyperbolic orbl",
                NBoolSet::sFalse, NBoolSet::sTrue, NBoolSet::sFalse, -1,
                NCensus::PURGE_NON_MINIMAL_HYP, false);
        }

        static bool mightBeMinimal(regina::NTriangulation* tri, void*) {
            return ! tri->simplifyToLocalMinimum(false);
        }

        static void rawCountsCompare(unsigned minTets, unsigned maxTets,
                const unsigned* realAns, const char* censusType,
                NBoolSet finiteness, NBoolSet orientability,
                NBoolSet boundary, int nBdryFaces, int whichPurge,
                bool testNonMinimality) {
            NContainer* census;

            for (unsigned nTets = minTets; nTets <= maxTets; nTets++) {
                census = new NContainer();
                NCensus::formCensus(census, nTets, finiteness, orientability,
                    boundary, nBdryFaces, whichPurge,
                    testNonMinimality ? &mightBeMinimal : 0);

                std::ostringstream msg;
                msg << "Census count for " << nTets << " tetrahedra ("
                    << censusType << ") should be " << realAns[nTets]
                    << ", not " << census->getNumberOfChildren() << '.';

                CPPUNIT_ASSERT_MESSAGE(msg.str(),
                    census->getNumberOfChildren() == realAns[nTets]);
                delete census;
            }
        }
};

void addNCensus(CppUnit::TextUi::TestRunner& runner) {
    runner.addTest(NCensusTest::suite());
}

