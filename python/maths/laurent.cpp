
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Python Interface                                                      *
 *                                                                        *
 *  Copyright (c) 1999-2018, Ben Burton                                   *
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

#include <boost/python.hpp>
#include "maths/integer.h"
#include "maths/laurent.h"
#include "../helpers.h"

using namespace boost::python;
using regina::Laurent;

namespace {
    const regina::Integer& getItem(const Laurent<regina::Integer>& p, long exp) {
        return p[exp];
    }
    void setItem(Laurent<regina::Integer>& p, long exp,
            const regina::Integer& value) {
        p.set(exp, value);
    }

    regina::Integer* seqFromList(boost::python::list l) {
        long len = boost::python::len(l);
        regina::Integer* coeffs = new regina::Integer[len];
        for (long i = 0; i < len; ++i) {
            // Accept any type that we know how to convert to a rational.
            extract<regina::Integer&> x_rat(l[i]);
            if (x_rat.check()) {
                coeffs[i] = x_rat();
                continue;
            }
            extract<regina::LargeInteger&> x_large(l[i]);
            if (x_large.check()) {
                coeffs[i] = x_large();
                continue;
            }
            extract<long> x_long(l[i]);
            if (x_long.check()) {
                coeffs[i] = x_long();
                continue;
            }

            // Throw an exception.
            delete[] coeffs;
            x_rat();
        }
        return coeffs;
    }

    Laurent<regina::Integer>* create_seq(long minExp, boost::python::list l) {
        regina::Integer* coeffs = seqFromList(l);
        if (coeffs) {
            Laurent<regina::Integer>* ans = new Laurent<regina::Integer>(
                minExp, coeffs, coeffs + boost::python::len(l));
            delete[] coeffs;
            return ans;
        }
        return 0;
    }

    void init_seq(Laurent<regina::Integer>& p, long minExp, boost::python::list l) {
        regina::Integer* coeffs = seqFromList(l);
        if (coeffs) {
            p.init(minExp, coeffs, coeffs + boost::python::len(l));
            delete[] coeffs;
        }
    }

    void (Laurent<regina::Integer>::*init_void)() =
        &Laurent<regina::Integer>::init;
    void (Laurent<regina::Integer>::*init_degree)(long) =
        &Laurent<regina::Integer>::init;
    std::string (Laurent<regina::Integer>::*str_variable)(const char*) const =
        &Laurent<regina::Integer>::str;
    std::string (Laurent<regina::Integer>::*utf8_variable)(const char*) const =
        &Laurent<regina::Integer>::utf8;
}

void addLaurent() {
    scope s = class_<Laurent<regina::Integer>,
            std::auto_ptr<Laurent<regina::Integer> >,
            boost::noncopyable>("Laurent")
        .def(init<long>())
        .def(init<const Laurent<regina::Integer>&>())
        .def("__init__", make_constructor(create_seq))
        .def("init", init_void)
        .def("init", init_degree)
        .def("init", init_seq)
        .def("minExp", &Laurent<regina::Integer>::minExp)
        .def("maxExp", &Laurent<regina::Integer>::maxExp)
        .def("isZero", &Laurent<regina::Integer>::isZero)
        .def("__getitem__", getItem, return_internal_reference<>())
        .def("__setitem__", setItem)
        .def("set", &Laurent<regina::Integer>::set)
        .def("swap", &Laurent<regina::Integer>::swap)
        .def("shift", &Laurent<regina::Integer>::shift)
        .def("scaleUp", &Laurent<regina::Integer>::scaleUp)
        .def("scaleDown", &Laurent<regina::Integer>::scaleDown)
        .def("negate", &Laurent<regina::Integer>::negate)
        .def("str", str_variable)
        .def("utf8", utf8_variable)
        .def(self *= regina::Integer())
        .def(self /= regina::Integer())
        .def(self += self)
        .def(self -= self)
        .def(self *= self)
        .def(regina::python::add_output())
        .def(self_ns::repr(self)) // add_output only gives __str__
        .def(regina::python::add_eq_operators())
    ;
}

