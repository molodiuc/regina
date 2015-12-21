
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Computational Engine                                                  *
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

/*! \file dim4/dim4edge.h
 *  \brief Deals with edges in a 4-manifold triangulation.
 */

#ifndef __DIM4EDGE_H
#ifndef __DOXYGEN
#define __DIM4EDGE_H
#endif

#include "regina-core.h"
#include "generic/face.h"
#include "maths/nperm5.h"
// NOTE: More #includes follow after the class declarations.

namespace regina {

class Dim4BoundaryComponent;

template <int> class Isomorphism;
typedef Component<4> Dim4Component;
typedef Isomorphism<4> Dim4Isomorphism;
typedef Simplex<4> Dim4Pentachoron;
typedef Triangulation<4> Dim4Triangulation;
typedef Face<4, 0> Dim4Vertex;

/**
 * \weakgroup dim4
 * @{
 */

/**
 * A convenience typedef for FaceEmbedding<4, 1>.
 */
typedef FaceEmbedding<4, 1> Dim4EdgeEmbedding;

namespace detail {

/**
 * Helper class that specifies how edges are numbered within a pentachoron.
 *
 * See the general FaceNumbering<dim, subdim> template class notes for
 * further details.
 */
template <>
class FaceNumbering<4, 1> {
    private:
        static const NPerm5 ordering_[10];
            /**< An array that hard-codes the results of ordering(). */

    public:
        /**
         * Given an edge number within a pentachoron, returns the
         * corresponding canonical ordering of the pentachoron vertices.
         *
         * If this canonical ordering is \a c, then <tt>c[0,1]</tt> will be
         * the vertices of the given edge in increasing numerical order.
         * That is, <tt>c[0]</tt> &lt; <tt>c[1]</tt>.  The remaining images
         * <tt>c[2,3,4]</tt> will be chosen to make the permutation even.
         *
         * Note that this is \e not the same permutation as returned by
         * Dim4Pentachoron::getEdgeMapping():
         *
         * - ordering() is a static function, which returns the same
         *   permutation for the same edge number, regardless of which
         *   pentachoron we are looking at.  The images of 0,1 will always
         *   appear in increasing order, and the permutation will always
         *   be even.
         *
         * - getEdgeMapping() examines the underlying edge \a E of the
         *   triangulation and, across all appearances of \a E in different
         *   pentachora: (i) chooses the images of 0,1 to map to the same
         *   respective vertices of \a E; and (ii) chooses the images
         *   of 2,3,4 to maintain a "consistent orientation" constraint.
         *
         * @param edge identifies which edge of a pentachoron to query.
         * This must be between 0 and 9 inclusive.
         * @return the corresponding canonical ordering of the
         * pentachoron vertices.
         */
        static NPerm5 ordering(unsigned edge);
        /**
         * Identifies which edge number in a pentachoron is represented
         * by the first two elements of the given permutation.
         *
         * In other words, this routine identifies which edge number in
         * a pentachoron joins vertices <tt>vertices[0]</tt> and
         * <tt>vertices[1]</tt>.
         *
         * @param vertices a permutation whose first two elements
         * represent some vertex numbers in a pentachoron.
         * @return the corresponding edge number in a pentachoron.
         * This will be between 0 and 9 inclusive.
         */
        static unsigned faceNumber(NPerm5 vertices);
        /**
         * Tests whether the given edge in a pentachoron contains the
         * given vertex of the pentachoron.
         *
         * @param edge an edge number in a pentachoron; this must
         * be between 0 and 9 inclusive.
         * @param vertex a vertex number in a pentachoron; this must be
         * between 0 and 4 inclusive.
         * @return \c true if and only if the given edge contains the
         * given vertex.
         */
        static bool containsVertex(unsigned edge, unsigned vertex);
};

} // namespace detail

/**
 * Represents an edge in the skeleton of a 4-manifold triangulation.
 *
 * This is a specialisation of the generic Face class template; see the
 * documentation for Face for a general overview of how this class works.
 *
 * These specialisations for Regina's \ref stddim "standard dimensions"
 * offer significant extra functionality.
 */
template <>
class REGINA_API Face<4, 1> : public detail::FaceBase<4, 1>,
        public Output<Face<4, 1>> {
    public:
        /**
         * A table that maps vertices of a pentachoron to edge numbers.
         *
         * Edges in a pentachoron are numbered 0,...,9.  This table
         * converts vertices to edge numbers; in particular, the edge
         * joining vertices \a i and \a j of a pentachoron is
         * edge number <tt>edgeNumber[i][j]</tt>.  Here \a i and \a j
         * must be distinct, must be between 0 and 4 inclusive, and may
         * be given in any order.  The resulting edge number will be
         * between 0 and 9 inclusive.
         *
         * Note that edge \i is always opposite triangle \i in a pentachoron.
         *
         * This table is the 4-manifold analogue to the 3-manifold table
         * NEdge::edgeNumber.  Note that NEdge::edgeNumber is still used
         * by 4-manifold triangulations to describe the numbering of
         * \a tetrahedron edges 0,...,5; see Dim4Tetrahedron::getEdge()
         * for details.
         */
        static const int edgeNumber[5][5];

        /**
         * A table that maps edges of a pentachoron to vertex numbers.
         *
         * Edges in a pentachoron are numbered 0,...,9.  This table
         * converts edge numbers to vertices; in particular, edge \a i
         * in a pentachoron joins vertices <tt>edgeVertex[i][0]</tt> and
         * <tt>edgeVertex[i][1]</tt>.  Here \a i must be between 0 and 9
         * inclusive; the resulting vertex numbers will be between 0 and 4
         * inclusive.
         *
         * Note that edge \i is always opposite triangle \i in a pentachoron.
         * It is guaranteed that <tt>edgeVertex[i][0]</tt> will always
         * be smaller than <tt>edgeVertex[i][1]</tt>.
         *
         * This table is the 4-manifold analogue to the 3-manifold table
         * NEdge::edgeVertex.  Note that NEdge::edgeVertex is still used
         * by 4-manifold triangulations to describe the numbering of
         * \a tetrahedron edges 0,...,5; see Dim4Tetrahedron::getEdge()
         * for details.
         */
        static const int edgeVertex[10][2];

    private:
        Dim4BoundaryComponent* boundaryComponent_;
            /**< The boundary component that this edge is a part of,
                 or 0 if this edge is internal. */
        Triangulation<2>* link_;
            /**< A triangulation of the edge link.  This will only be
             * constructed on demand; until then it will be null. */

    public:
        /**
         * Returns the boundary component of the triangulation to which
         * this edge belongs.
         *
         * See the note in the Dim4BoundaryComponent overview regarding
         * what happens if the edge link itself has more than one
         * boundary component.  Note that such an edge link makes the
         * triangulation invalid.
         *
         * @return the boundary component containing this edge, or 0 if this
         * edge does not lie entirely within the boundary of the triangulation.
         */
        Dim4BoundaryComponent* getBoundaryComponent() const;

        /**
         * Returns the vertex of the 4-manifold triangulation corresponding
         * to the given vertex of this edge.
         *
         * @param vertex the vertex of this edge to examine.  This should
         * be 0 or 1.
         * @return the corresponding vertex of the 4-manifold triangulation.
         */
        Dim4Vertex* getVertex(int vertex) const;

        /**
         * Determines if this edge lies entirely on the boundary of the
         * triangulation.
         *
         * @return \c true if and only if this edge lies on the boundary.
         */
        bool isBoundary() const;

        /**
         * Returns a full 2-manifold triangulation describing
         * the link of this edge.
         *
         * This routine is fast (it uses a pre-computed triangulation
         * where possible).  The downside is that the triangulation is
         * read-only, and does not contain any information on how the
         * triangles in the link correspond to pentachora in the original
         * triangulation (though this is easily deduced; see below).
         * If you want a writable triangulation, or one with this extra
         * information, then call buildLinkDetail() instead.
         *
         * The triangulation of the edge link is built as follows.
         * Let \a i lie between 0 and getDegree()-1 inclusive, let
         * \a pent represent <tt>getEmbedding(i).getPentachoron()</tt>,
         * and let \a e represent <tt>getEmbedding(i).getEdge()</tt>.
         * Then <tt>buildLink()->getTriangle(i)</tt> is the triangle
         * in the edge link that links edge \a e of pentachoron \a pent.
         * In other words, <tt>buildLink()->getTriangle(i)</tt> in the edge link
         * is parallel to triangle <tt>pent->getTriangle(e)</tt> in the
         * surrounding 4-manifold triangulation.
         *
         * The vertices of each triangle in the edge link are
         * numbered as follows.  Following the discussion above,
         * suppose that <tt>buildLink()->getTriangle(i)</tt>
         * sits within \c pent and is parallel to
         * <tt>pent->getTriangle(e)</tt>.
         * Then vertices 0,1,2 of the triangle in the link will be
         * parallel to vertices 0,1,2 of the corresponding Dim4Triangle.
         * The permutation <tt>pent->getTriangleMapping(e)</tt> will map
         * vertices 0,1,2 of the triangle in the link to the
         * corresponding vertices of \c pent (those opposite \c e),
         * and will map 3 and 4 to the vertices of \c e itself.
         *
         * This Dim4Edge object will retain ownership of the triangulation
         * that is returned.  If you wish to edit the triangulation, you
         * should make a new clone and edit the clone instead.
         *
         * @return the read-only triangulated link of this edge.
         */
        const Triangulation<2>* buildLink() const;

        /**
         * Returns a full 2-manifold triangulation describing
         * the link of this edge.
         *
         * This routine is heavyweight (it computes a new triangulation
         * each time).  The benefit is that the triangulation is writeable,
         * and optionally contain detailed information on how the triangles
         * in the link correspond to pentachora in the original triangulation.
         * If you do not need this extra information, consider using the
         * faster buildLink() instead.
         *
         * See the buildLink() documentation for an explanation of
         * exactly how the triangulation will be constructed.
         *
         * If \a labels is passed as \c true, each triangle of the new
         * edge link will be given a text description of the form
         * <tt>p&nbsp;(e)</tt>, where \c p is the index of the pentachoron
         * the triangle is from, and \c e is the edge of that pentachoron
         * that this triangle links.
         *
         * If \a inclusion is non-null (i.e., it points to some
         * Dim4Isomorphism pointer \a p), then it will be modified to
         * point to a new Dim4Isomorphism that describes in detail how the
         * individual triangles of the link sit within pentachora of
         * the original triangulation.  Specifically, after this routine
         * is called, <tt>p->pentImage(i)</tt> will indicate which pentachoron
         * \a pent of the 4-manifold triangulation contains the <i>i</i>th
         * triangle of the link.  Moreover, <tt>p->facetPerm(i)</tt> will
         * indicate exactly where the <i>i</i>th triangle sits within
         * \a pent: (i) it will send 3,4 to the vertices of \a pent that lie
         * on the edge that the triangle links, with 3 and 4 mapping to
         * vertices 0 and 1 respectively of the corresponding Dim4Edge;
         * and (ii) it will send 0,1,2 to the vertices of \a pent that
         * are parallel to vertices 0,1,2 of this triangle.
         *
         * The triangulation that is returned, as well as the isomorphism
         * if one was requested, will be newly allocated.  The caller of
         * this routine is responsible for destroying these objects.
         *
         * Strictly speaking, this is an abuse of the Dim4Isomorphism class
         * (the domain is a triangulation of the wrong dimension, and
         * the map is not 1-to-1 into the range pentachora).  We use
         * it anyway, but you should not attempt to call any high-level
         * routines (such as Dim4Isomorphism::apply).
         *
         * \ifacespython The second (isomorphism) argument is not present.
         * Instead this routine returns a pair (triangulation, isomorphism).
         * As a side-effect, the isomorphism will always be constructed
         * (i.e., it is not optional).
         *
         * @return a newly constructed triangulation of the link of this edge.
         */
        Triangulation<2>* buildLinkDetail(bool labels = true,
            Dim4Isomorphism** inclusion = 0) const;

        /**
         * Writes a short text representation of this object to the
         * given output stream.
         *
         * \ifacespython Not present.
         *
         * @param out the output stream to which to write.
         */
        void writeTextShort(std::ostream& out) const;
        /**
         * Writes a detailed text representation of this object to the
         * given output stream.
         *
         * \ifacespython Not present.
         *
         * @param out the output stream to which to write.
         */
        void writeTextLong(std::ostream& out) const;

    private:
        /**
         * Creates a new edge and marks it as belonging to the
         * given triangulation component.
         *
         * @param component the triangulation component to which this
         * edge belongs.
         */
        Face(Dim4Component* component);

    friend class Triangulation<4>;
    friend class detail::TriangulationBase<4>;
};

/**
 * A convenience typedef for Face<4, 1>.
 */
typedef Face<4, 1> Dim4Edge;

/*@}*/

} // namespace regina
// Some more headers that are required for inline functions:
#include "dim4/dim4pentachoron.h"
namespace regina {

// Inline functions for FaceNumbering

namespace detail {

inline NPerm5 FaceNumbering<4, 1>::ordering(unsigned edge) {
    return ordering_[edge];
}

inline unsigned FaceNumbering<4, 1>::faceNumber(NPerm5 vertices) {
    return Dim4Edge::edgeNumber[vertices[0]][vertices[1]];
}

inline bool FaceNumbering<4, 1>::containsVertex(unsigned edge,
        unsigned vertex) {
    return (vertex == Dim4Edge::edgeVertex[edge][0] ||
            vertex == Dim4Edge::edgeVertex[edge][1]);
}

} // namespace detail

// Inline functions for Dim4Edge

inline Face<4, 1>::Face(Dim4Component* component) :
        detail::FaceBase<4, 1>(component),
        boundaryComponent_(0), link_(0) {
}

inline Dim4BoundaryComponent* Face<4, 1>::getBoundaryComponent() const {
    return boundaryComponent_;
}

inline Dim4Vertex* Face<4, 1>::getVertex(int vertex) const {
    return front().pentachoron()->vertex(front().vertices()[vertex]);
}

inline bool Face<4, 1>::isBoundary() const {
    return (boundaryComponent_ != 0);
}

inline const Triangulation<2>* Face<4, 1>::buildLink() const {
    if (! link_) {
        // This is a construct-on-demand member; cast away constness to
        // set it here.
        const_cast<Dim4Edge*>(this)->link_ = buildLinkDetail(false, 0);
    }
    return link_;
}

inline void Face<4, 1>::writeTextShort(std::ostream& out) const {
    out << (boundaryComponent_ ? "Boundary " : "Internal ")
        << "edge of degree " << degree();
}

} // namespace regina

#endif

