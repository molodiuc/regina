
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

#include "packet/npacket.h"
#include "../semiweakheldtype.h"

// Held type must be declared before boost/python.hpp
#include <boost/python.hpp>

using namespace boost::python;
using namespace regina::python;
using regina::NPacket;

namespace {
    NPacket* (NPacket::*firstTreePacket_non_const)(const std::string&) =
        &NPacket::firstTreePacket;
    NPacket* (NPacket::*nextTreePacket_non_const)(const std::string&) =
        &NPacket::nextTreePacket;
    NPacket* (NPacket::*findPacketLabel_non_const)(const std::string&) =
        &NPacket::findPacketLabel;
    bool (NPacket::*save_filename)(const char*, bool) const = &NPacket::save;
    NPacket* (*open_filename)(const char*) = &regina::open;

    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(OL_moveUp,
        NPacket::moveUp, 0, 1);
    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(OL_moveDown,
        NPacket::moveDown, 0, 1);
    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(OL_nextTreePacket,
        NPacket::nextTreePacket, 0, 1);
    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(OL_clone,
        NPacket::clone, 0, 2);
    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(OL_save,
        NPacket::save, 1, 2);

    void reparent_check(NPacket& child, NPacket* newParent,
            bool first = false) {
        if (child.getTreeParent())
            child.reparent(newParent, first);
        else {
            PyErr_SetString(PyExc_AssertionError,
                "reparent() cannot be used on packets with no parent");
            ::boost::python::throw_error_already_set();
        }
    }

    BOOST_PYTHON_FUNCTION_OVERLOADS(OL_reparent, reparent_check, 2, 3);

    NPacket* makeOrphan_return(NPacket* subtree) {
        subtree->makeOrphan();
        return subtree;
    }

    boost::python::list getTags_list(const NPacket* p) {
        const std::set<std::string>& tags = p->getTags();
        std::set<std::string>::const_iterator it;

        boost::python::list ans;
        for (it = tags.begin(); it != tags.end(); it++)
            ans.append(*it);
        return ans;
    }
}

void addNPacket() {

    class_<
        NPacket, boost::noncopyable, SemiWeakHeldType<NPacket> >(
            "NPacket", no_init)
        .def("getPacketType", &NPacket::getPacketType)
        .def("getPacketTypeName", &NPacket::getPacketTypeName)
        .def("getPacketLabel", &NPacket::getPacketLabel,
            return_value_policy<return_by_value>())
        .def("getHumanLabel", &NPacket::getHumanLabel)
        .def("adornedLabel", &NPacket::adornedLabel)
        .def("setPacketLabel", &NPacket::setPacketLabel)
        .def("getFullName", &NPacket::getFullName)
        .def("makeUniqueLabel", &NPacket::makeUniqueLabel)
        .def("makeUniqueLabels", &NPacket::makeUniqueLabels)
        .def("hasTag", &NPacket::hasTag)
        .def("hasTags", &NPacket::hasTags)
        .def("addTag", &NPacket::addTag)
        .def("removeTag", &NPacket::removeTag)
        .def("removeAllTags", &NPacket::removeAllTags)
        .def("getTags", getTags_list)
        .def("getTreeParent", &NPacket::getTreeParent,
             return_value_policy<to_held_type<> >())
        .def("getFirstTreeChild", &NPacket::getFirstTreeChild,
             return_value_policy<to_held_type<> >())
        .def("getLastTreeChild", &NPacket::getLastTreeChild,
             return_value_policy<to_held_type<> >())
        .def("getNextTreeSibling", &NPacket::getNextTreeSibling,
             return_value_policy<to_held_type<> >())
        .def("getPrevTreeSibling", &NPacket::getPrevTreeSibling,
             return_value_policy<to_held_type<> >())
        .def("getTreeMatriarch", &NPacket::getTreeMatriarch,
             return_value_policy<to_held_type<> >())
        .def("levelsDownTo", &NPacket::levelsDownTo)
        .def("levelsUpTo", &NPacket::levelsUpTo)
        .def("isGrandparentOf", &NPacket::isGrandparentOf)
        .def("getNumberOfChildren", &NPacket::getNumberOfChildren)
        .def("getNumberOfDescendants", &NPacket::getNumberOfDescendants)
        .def("getTotalTreeSize", &NPacket::getTotalTreeSize)
        .def("insertChildFirst", &NPacket::insertChildFirst)
        .def("insertChildLast", &NPacket::insertChildLast)
        .def("insertChildAfter", &NPacket::insertChildAfter)
        .def("makeOrphan", makeOrphan_return,
             return_value_policy<to_held_type<> >())
        .def("reparent", reparent_check, OL_reparent())
        .def("transferChildren", &NPacket::transferChildren)
        .def("swapWithNextSibling", &NPacket::swapWithNextSibling)
        .def("moveUp", &NPacket::moveUp, OL_moveUp())
        .def("moveDown", &NPacket::moveDown, OL_moveDown())
        .def("moveToFirst", &NPacket::moveToFirst)
        .def("moveToLast", &NPacket::moveToLast)
        .def("sortChildren", &NPacket::sortChildren)
        .def("nextTreePacket", nextTreePacket_non_const, OL_nextTreePacket()
             [return_value_policy<to_held_type<> >()])
        .def("firstTreePacket", firstTreePacket_non_const,
             return_value_policy<to_held_type<> >())
        .def("firstTreePacket", firstTreePacket_non_const,
             return_value_policy<to_held_type<> >())
        .def("findPacketLabel", findPacketLabel_non_const,
             return_value_policy<to_held_type<> >())
        .def("dependsOnParent", &NPacket::dependsOnParent)
        .def("isPacketEditable", &NPacket::isPacketEditable)
        .def("clone", &NPacket::clone,
             OL_clone()[return_value_policy<to_held_type<> >()])
        .def("save", save_filename, OL_save())
        .def("internalID", &NPacket::internalID)
        .def("str", &NPacket::str)
        .def("toString", &NPacket::toString)
        .def("detail", &NPacket::detail)
        .def("toStringLong", &NPacket::toStringLong)
        .def("__str__", &NPacket::str);
    ;

    def("open", open_filename, return_value_policy<to_held_type<> >());

}

