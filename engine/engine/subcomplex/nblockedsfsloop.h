
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Computational Engine                                                  *
 *                                                                        *
 *  Copyright (c) 1999-2006, Ben Burton                                   *
 *  For further details contact Ben Burton (bab@debian.org).              *
 *                                                                        *
 *  This program is free software; you can redistribute it and/or         *
 *  modify it under the terms of the GNU General Public License as        *
 *  published by the Free Software Foundation; either version 2 of the    *
 *  License, or (at your option) any later version.                       *
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

/*! \file nblockedsfsloop.h
 *  \brief Supports self-identified Seifert fibred spaces that are
 *  triangulated using saturated blocks.
 */

#ifndef __NBLOCKEDSFSLOOP_H
#ifndef __DOXYGEN
#define __NBLOCKEDSFSLOOP_H
#endif

#include "subcomplex/nstandardtri.h"
#include "utilities/nmatrix2.h"

namespace regina {

class NSatRegion;

/**
 * \weakgroup subcomplex
 * @{
 */

/**
 * TODO: Document NBlockedSFSLoop.
 *
 * TODO: When documenting this one, don't forget to talk about the two
 * possibilities for the two boundary annuli.
 *
 * Only deal with annuli for now.
 */
class NBlockedSFSLoop : public NStandardTriangulation {
    private:
        NSatRegion* region_;
        NMatrix2 matchingReln_;

    public:
        /**
         * Destroys this structure and its constituent components.
         */
        ~NBlockedSFSLoop();

        const NSatRegion& region() const;
        const NMatrix2& matchingReln() const;

        NManifold* getManifold() const;
        std::ostream& writeName(std::ostream& out) const;
        std::ostream& writeTeXName(std::ostream& out) const;
        void writeTextLong(std::ostream& out) const;

        static NBlockedSFSLoop* isBlockedSFSLoop(NTriangulation* tri);

    private:
        NBlockedSFSLoop(NSatRegion* region, const NMatrix2& matchingReln);
};

/*@}*/

// Inline functions for NBlockedSFSLoop

inline NBlockedSFSLoop::NBlockedSFSLoop(NSatRegion* region,
        const NMatrix2& matchingReln) :
        region_(region), matchingReln_(matchingReln) {
}

inline const NSatRegion& NBlockedSFSLoop::region() const {
    return *region_;
}

inline const NMatrix2& NBlockedSFSLoop::matchingReln() const {
    return matchingReln_;
}

} // namespace regina

#endif

