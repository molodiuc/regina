
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

/*! \file generic/detail/xmltrireader.h
 *  \brief Implementation details for parsing XML data for triangulation
 *  packets.
 */

#ifndef __XMLTRIREADER_H_DETAIL
#ifndef __DOXYGEN
#define __XMLTRIREADER_H_DETAIL
#endif

#include "regina-core.h"
#include "packet/nxmlpacketreader.h"
#include "generic/triangulation.h"
#include "utilities/stringutils.h"
#include <vector>

namespace regina {

template <int> class XMLTriangulationReader;

namespace detail {

/**
 * \weakgroup detail
 * @{
 */

/**
 * Internal class that indicates the XML tags and attributes used to describe
 * top-dimensional simplices in a <i>dim</i>-dimensional triangulation.
 *
 * \ifacespython Not present.
 *
 * \tparam dim The dimension of the triangulation in question.
 */
template <int dim>
struct REGINA_API XMLTriangulationTags {
    constexpr static const char* simplices();
        /**< The XML tag that stores the set of all top-dimensional
             simplices for a <i>dim</i>-dimensional triangulation. */
    constexpr static const char* simplex();
        /**< The XML tag that stores a single top-dimensional simplex
             in a <i>dim</i>-dimensional triangulation. */
    constexpr static const char* size();
        /**< The XML attribute that stores the number of top-dimensional
             simplices in a <i>dim</i>-dimensional triangulation. */
};

/**
 * Helper class that reads the XML element for a single top-dimensional
 * simplex in a <i>dim</i>-dimensional triangulation.
 * In other words, this reads the contents of a single &lt;simplex&gt;
 * element for dimension \a dim &ge; 5, or a single &lt;triangle&gt;,
 * &lt;tet&gt; or &lt;pent&gt; element for dimension \a dim = 2, 3 or 4.
 *
 * It is assumed that the underlying triangulation and its simplices
 * have already been created.  The task of this reader is to flesh out
 * the "contents" of a single simplex; that is, the description of the
 * simplex and its gluings to adjacent simplices.
 *
 * \ifacespython Not present.
 *
 * \tparam dim The dimension of the triangulation being read.
 */
template <int dim>
class XMLSimplexReader : public NXMLElementReader {
    private:
        Triangulation<dim>* tri_;
            /**< The triangulation containing the simplices being read. */
        Simplex<dim>* simplex_;
            /**< The specific simplex being read. */

    public:
        /**
         * Creates a new simplex element reader.
         *
         * \pre The given triangulation \a tri already contains at least
         * (\a whichSimplex + 1) top-dimensional simplices.
         *
         * @param tri the triangulation containing the simplex being read.
         * @param whichSimplex the index of the simplex being read
         * within the triangulation \a tri.
         */
        XMLSimplexReader(Triangulation<dim>* tri, size_t whichSimplex);

        virtual void startElement(const std::string&,
                const regina::xml::XMLPropertyDict& props, NXMLElementReader*);

        virtual void initialChars(const std::string& chars);
};

/**
 * Helper class that reads the XML element for the set of all top-dimensional
 * simplices in a <i>dim</i>-dimensional triangulation.
 * In other words, this reads the contents of a single &lt;simplices&gt;
 * element for dimension \a dim &ge; 5, or a single &lt;triangles&gt;,
 * &lt;tetrahedra&gt; or &lt;pentachora&gt; element for dimension
 * \a dim = 2, 3 or 4.
 *
 * It is assumed that the underlying triangulation has already been created,
 * but its simplices have not.
 *
 * \ifacespython Not present.
 *
 * \tparam dim The dimension of the triangulation being read.
 */
template <int dim>
class XMLSimplicesReader : public NXMLElementReader {
    private:
        Triangulation<dim>* tri_;
            /**< The triangulation to contain the simplices being read. */
        size_t readSimplices_;
            /**< The number of simplices in the triangulation, as defined by
                 the \c size attribute of this tag (or, in standard dimensions,
                 the \c ntriangles, \c ntet or \c npent attribute instead). */

    public:
        /**
         * Creates a new simplices element reader.
         *
         * The given triangulation should be empty; its simplices will
         * be created by this reader.
         *
         * @param tri the triangulation being read.
         */
        XMLSimplicesReader(Triangulation<dim>* tri);

        virtual void startElement(const std::string&,
                const regina::xml::XMLPropertyDict& props, NXMLElementReader*);

        virtual NXMLElementReader* startSubElement(
                const std::string& subTagName,
                const regina::xml::XMLPropertyDict&);
};

/**
 * Helper class that provides core functionality for the XML packet reader
 * that reads a single <i>dim</i>-dimensional triangulation.
 *
 * The XML packet reader itself is provided by the class
 * XMLTriangulationReader<dim>, which uses this as a base class.
 * There should be no need for other classes to refer to
 * XMLTriangulationReaderBase directly.
 *
 * \ifacespython Not present.
 *
 * \tparam dim The dimension of the triangulation being read.
 */
template <int dim>
class REGINA_API XMLTriangulationReaderBase : public NXMLPacketReader {
    protected:
        Triangulation<dim>* tri_;
            /**< The triangulation currently being read. */

    public:
        /**
         * Creates a new triangulation reader.
         *
         * @param resolver the master resolver that will be used to fix
         * dangling packet references after the entire XML file has been read.
         */
        XMLTriangulationReaderBase(NXMLTreeResolver& resolver);

        virtual NPacket* packet() override;
        virtual NXMLElementReader* startContentSubElement(
            const std::string& subTagName,
            const regina::xml::XMLPropertyDict& subTagProps) override;
        virtual void endContentSubElement(const std::string& subTagName,
            NXMLElementReader* subReader) override;
};

/*@}*/

// Inline functions for XMLTriangulationTags

template <int dim>
inline constexpr const char* XMLTriangulationTags<dim>::simplices() {
    return "simplices";
}
template <>
inline constexpr const char* XMLTriangulationTags<4>::simplices() {
    return "pentachora";
}
template <>
inline constexpr const char* XMLTriangulationTags<3>::simplices() {
    return "tetrahedra";
}
template <>
inline constexpr const char* XMLTriangulationTags<2>::simplices() {
    return "triangles";
}

template <int dim>
inline constexpr const char* XMLTriangulationTags<dim>::simplex() {
    return "simplex";
}
template <>
inline constexpr const char* XMLTriangulationTags<4>::simplex() {
    return "pent";
}
template <>
inline constexpr const char* XMLTriangulationTags<3>::simplex() {
    return "tet";
}
template <>
inline constexpr const char* XMLTriangulationTags<2>::simplex() {
    return "triangle";
}

template <int dim>
inline constexpr const char* XMLTriangulationTags<dim>::size() {
    return "size";
}
template <>
inline constexpr const char* XMLTriangulationTags<4>::size() {
    return "npent";
}
template <>
inline constexpr const char* XMLTriangulationTags<3>::size() {
    return "ntet";
}
template <>
inline constexpr const char* XMLTriangulationTags<2>::size() {
    return "ntriangles";
}

// Implementation details for XMLSimplexReader

template <int dim>
inline XMLSimplexReader<dim>::XMLSimplexReader(
        Triangulation<dim>* tri, size_t whichSimplex) :
        tri_(tri), simplex_(tri->simplices()[whichSimplex]) {
}

template <int dim>
inline void XMLSimplexReader<dim>::startElement(const std::string&,
        const regina::xml::XMLPropertyDict& props, NXMLElementReader*) {
    simplex_->setDescription(props.lookup("desc"));
}

template <int dim>
void XMLSimplexReader<dim>::initialChars(const std::string& chars) {
    std::vector<std::string> tokens;
    if (basicTokenise(back_inserter(tokens), chars) != 2 * (dim + 1))
        return;

    long simpIndex;
    typename NPerm<dim + 1>::Code permCode;
    NPerm<dim + 1> perm;
    Simplex<dim>* adjSimp;
    int adjFacet;
    for (int k = 0; k <= dim; ++k) {
        if (! valueOf(tokens[2 * k], simpIndex))
            continue;
        if (! valueOf(tokens[2 * k + 1], permCode))
            continue;

        if (simpIndex < 0 || simpIndex >= static_cast<long>(tri_->size()))
            continue;
        if (! NPerm<dim + 1>::isPermCode(permCode))
            continue;

        perm.setPermCode(permCode);
        adjSimp = tri_->simplices()[simpIndex];
        adjFacet = perm[k];
        if (adjSimp == simplex_ && adjFacet == k)
            continue;
        if (simplex_->adjacentSimplex(k))
            continue;
        if (adjSimp->adjacentSimplex(adjFacet))
            continue;

        simplex_->joinTo(k, adjSimp, perm);
    }
}

// Implementation details for XMLSimplicesReader

template <int dim>
inline XMLSimplicesReader<dim>::XMLSimplicesReader(Triangulation<dim>* tri) :
        tri_(tri), readSimplices_(0) {
}

template <int dim>
void XMLSimplicesReader<dim>::startElement(const std::string& /* tagName */,
        const regina::xml::XMLPropertyDict& props, NXMLElementReader*) {
    long nSimplices;
    if (valueOf(props.lookup(XMLTriangulationTags<dim>::size()), nSimplices))
        for ( ; nSimplices > 0; --nSimplices)
            tri_->newSimplex();
}

template <int dim>
NXMLElementReader* XMLSimplicesReader<dim>::startSubElement(
        const std::string& subTagName,
        const regina::xml::XMLPropertyDict&) {
    if (subTagName == XMLTriangulationTags<dim>::simplex()) {
        if (readSimplices_ < tri_->size())
            return new XMLSimplexReader<dim>(tri_, readSimplices_++);
        else
            return new NXMLElementReader();
    } else
        return new NXMLElementReader();
}

// Inline functions for XMLTriangulationReaderBase

template <int dim>
inline XMLTriangulationReaderBase<dim>::XMLTriangulationReaderBase(
        NXMLTreeResolver& resolver) :
        NXMLPacketReader(resolver), tri_(new Triangulation<dim>()) {
}

template <int dim>
inline NPacket* XMLTriangulationReaderBase<dim>::packet() {
    return tri_;
}

template <int dim>
NXMLElementReader* XMLTriangulationReaderBase<dim>::startContentSubElement(
        const std::string& subTagName,
        const regina::xml::XMLPropertyDict& subTagProps) {
    if (subTagName == XMLTriangulationTags<dim>::simplices())
        return new XMLSimplicesReader<dim>(tri_);
    return static_cast<XMLTriangulationReader<dim>*>(this)->
        startPropertySubElement(subTagName, subTagProps);
}

template <int dim>
inline void XMLTriangulationReaderBase<dim>::endContentSubElement(
        const std::string&, NXMLElementReader*) {
}

} } // namespace regina::detail

#endif
