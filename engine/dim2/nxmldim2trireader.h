
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Computational Engine                                                  *
 *                                                                        *
 *  Copyright (c) 1999-2014, Ben Burton                                   *
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

/*! \file dim2/nxmldim2trireader.h
 *  \brief Deals with parsing XML data for 4-manifold triangulation packets.
 */

#ifndef __NXMLDIM2TRIREADER_H
#ifndef __DOXYGEN
#define __NXMLDIM2TRIREADER_H
#endif

#include "regina-core.h"
#include "packet/nxmlpacketreader.h"
#include "dim2/dim2triangulation.h"

namespace regina {

/**
 * \weakgroup dim2
 * @{
 */

/**
 * An XML packet reader that reads a single 2-manifold triangulation.
 *
 * \ifacespython Not present.
 */
class REGINA_API NXMLDim2TriangulationReader : public NXMLPacketReader {
    private:
        Dim2Triangulation* tri_;
            /**< The triangulation currently being read. */

    public:
        /**
         * Creates a new triangulation reader.
         *
         * @param resolver the master resolver that will be used to fix
         * dangling packet references after the entire XML file has been read.
         */
        NXMLDim2TriangulationReader(NXMLTreeResolver& resolver);

        virtual NPacket* getPacket();
        virtual NXMLElementReader* startContentSubElement(
            const std::string& subTagName,
            const regina::xml::XMLPropertyDict& subTagProps);
        virtual void endContentSubElement(const std::string& subTagName,
            NXMLElementReader* subReader);
};

/*@}*/

// Inline functions for NXMLDim2TriangulationReader

inline NXMLDim2TriangulationReader::NXMLDim2TriangulationReader(
        NXMLTreeResolver& resolver) :
        NXMLPacketReader(resolver), tri_(new Dim2Triangulation()) {
}

inline NPacket* NXMLDim2TriangulationReader::getPacket() {
    return tri_;
}

} // namespace regina

#endif

