
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Python Interface                                                      *
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

#include <boost/python.hpp>
#include "split/nsignature.h"
#include "triangulation/ntriangulation.h"

using namespace boost::python;
using regina::NSignature;

namespace {
    void writeCycles(const NSignature& sig, const std::string& cycleOpen,
            const std::string& cycleClose, const std::string& cycleJoin) {
        sig.writeCycles(std::cout, cycleOpen, cycleClose, cycleJoin);
    }
}

void addNSignature() {
    class_<NSignature, bases<regina::ShareableObject>,
            std::auto_ptr<NSignature>, boost::noncopyable>("NSignature",
            init<const NSignature&>())
        .def("getOrder", &NSignature::getOrder)
        .def("parse", &NSignature::parse,
            return_value_policy<manage_new_object>())
        .def("triangulate", &NSignature::triangulate,
            return_value_policy<manage_new_object>())
        .def("writeCycles", &writeCycles)
        .staticmethod("parse")
    ;
}

