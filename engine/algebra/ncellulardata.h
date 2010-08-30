/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Computational Engine                                                  *
 *                                                                        *
 *  Copyright (c) 1999-2009, Ben Burton                                   *
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

/*! \file algebra/ncellulardata.h
 *  \brief Deals with cellular homology of 3 and 4-manifolds. 
 */

#ifndef __NCELLULARDATA_H
#ifndef __DOXYGEN
#define __NCELLULARDATA_H
#endif

#include "triangulation/ntriangulation.h"
#include "dim4/dim4triangulation.h"

#include "algebra/nmarkedabeliangroup.h"
#include "algebra/nbilinearform.h"
#include "algebra/nhomgrouppresentation.h"
#include "algebra/ngrouppresentation.h"
#include "utilities/ptrutils.h"
#include "utilities/nbooleans.h"
#include "maths/nsparsegrid.h"
#include "maths/nperm3.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace regina {

class NTriangulation;
class Dim4Triangulation;

/**
 * \weakgroup algebra
 * @{
 */

/**
 * Data type that deals with all the detailed homological information in a
 *  manifold.  This information includes:
 *
 * - the manifold's (and its boundary's) homology and cohomology computed in various coordinate 
 *    systems, with coefficients in an arbitrary cyclic group.
 * - the maps between coordinate systems, Bocksteins, boundary inclusion, etc. 
 * - the bilinear forms coming from Poincare Duality: H_i \otimes H_j --> H_{i+j-n}
 *   and torsion linking \tau H_i \otimes \tau H_{n-i-1} --> Q/Z 
 * - fundamental groups of the manifold, natural submanifolds and maps between them.
 *
 * This class mostly takes a "least effort" approach to all computations. It only computes 
 * what is neccessary for your requests.  It also keeps a record of all previous computations 
 * you've made, we'll call it the `precomputed pile'. If a computation can be sped up by not 
 * recomputing some data, it'll try to take the shortcut.  At present the only exception to 
 * this rule is that all integer coefficient chain complexes and maps are computed on 
 * initialization, but this is relatively quick.  
 *
 * @pre - Assumes input triangulation is valid and path-connected.
 *
 * \testpart
 *
 * \todo  1) Complete collection of homology natural bilinear forms on a manifold, spin structures. See (3), (4)
 * \todo  2) test suite stuff: LES of pair, natural isos, PD, detailed tests for intersection forms.
 *        Move all the test routines out of the NCellularData class and put them in the test suite proper. 
 *        Need some kind of form tests for 4-manifolds.  But need some 4-manifolds that we understand, first.
 *        double check torsion linking form behaves properly. NPartition
 * \todo  3) New coordinate systems to implement:
 *        MIX_BDRY_coord, MIX_REL_BDRY_coord, DUAL_BDRY_coord, DUAL_REL_BDRY_coord and all the
 *        various maps.  This is required to get at things like H^i M x H^j M --> H^{i+j} M
 *        cup products. Chain complex initialization TODO. chain maps TODO.  PD / intersection forms TODO
 *        Note, current "mixed" chain complex does not subdivide ideal boundary.  Is this an issue? 
 * \todo  4) To minimize memory usage we should consider having homs, bilinear forms, etc, 
 *        not store their initialization data, instead trusting it to the NCellularData stack.  
 * \todo  5) need to set up local orientations for dual boundary coordinates, for the barycentres
 *        of all standard boundary simplices.  We'll put something in setupIndices for this.
 *        Currently done for Dim4Triangulations, but not for NTriangulations. 
 * \todo  6) We'll also eventually need maximal trees in the standard and mixed 1-skeleton, to implement
 *        Farber-Levine pairings. 
 * \todo  7) Make writeTextShort and writeTextLong more pleasant to look at.  Currently it's not 
 *        clear what all the computations mean. 
 * \todo  8) Make all of the Regina engine POSIX pthread safe.  Make a thread-pool version of the test suite. 
 *        Right now NCellularData, probably some aspect of the Alexander Ideal construction process is the 
 *        main culprit. 
 * \todo \optlong We should add Bocksteins and the long exact sequence associated to a change-of-coefficient map.
 *
 * Guide to ncellulardata.*.cpp files:
 *
 *       ncellulardata.cpp - contains only the core routines to call for information stored on internal stacks. 
 *                           NCellularData::integerChainComplex
 *                           NCellularData::integerChainMap
 *                           NCellularData::unMarkedGroup 
 *                           NCellularData::markedGroup
 *                           NCellularData::homGroup 
 *                           NCellularData::poincarePolynomial 
 *                           NCellularData::bilinearForm
 *                           NCellularData::groupPresentation
 *                           NCellularData::homGroupPresentation. 
 *                           NCellularData::alexanderChainComplex
 *
 *       ncellulardata.init.indexing.cpp - contains the NCellularData::NCellularData(NTriangulation / Dim4Triangulation)
 *                                         constructor, and the routines that set up the internal indices like icIx, etc.
 *
 *       ncellulardata.init.cc.cpp - contains the homology chain complex initialization routines.
 *
 *       ncellulardata.init.hom.cpp - contains the homology chain map initialization routines. 
 *
 *       ncellulardata.init.pi1.cpp - contains initialization routines for the fundamental groups presentations 
 *                                    and maps between them.
 *
 *       ncellulardata.locators.cpp - contains the ***Locator classes for the internal maps that store the various
 *                                    computations in NCellularData. 
 *
 *       ncellulardata.lookups.cpp  - contains reverse lookups for the cell indices such as icIx, etc. 
 *
 *       ncellulardata.output.cpp - contains the writeTextShort routines.
 *
 *       ncellulardata.tests.cpp - contains all the canned diagnostic tests. 
 *
 * @author Ryan Budney
 */
class NCellularData : public ShareableObject {
public:

 /**
  * This enum gives names to the 9 standard CW-structures associated to a semi-simplicially (ideall) triangulated
  * manifold.  Ideal triangulations are generally not compact.  These cell decomposition are decompositions corresponding
  * to the compact manifold such that the ideally-triangulated manifold is a dense subspace and such that its cells are
  * the intersection of these cells to that subspace. See NCellularData::unmarkedGroup, NCellularData::markedGroup, 
  * NCellularData::homGroup, NCellularData::bilinearForm, NCellularData::GroupLocator, NCellularData::HomLocator for usage.  
  *
  * See also the valid_coordinate_systems vector, to iterate through implemented coordinate systems.
  */
 enum homology_coordinate_system { 
 first_coord,
 /**
  * Is the most natural CW-decomposition of a semi-simplicially (ideal) triangulated manifold. The top-dimensional cells are
  * the tetrahedra (of a 3-manifold) or the pentachora (of a 4-manifold). Dual to DUAL_REL_BDRY_coord.
  */
  STD_coord = first_coord, // indexed by nicIx then icIx 
 /**
  * Is the dual polyhedral decomposition to this CW-decomposition. The top-dimensional cells correspond to the interior
  * vertices of the triangulation. Dual to STD_REL_BDRY_coord.
  */
  DUAL_coord, // indexed by dcIx
 /**
  * Is essentially the CW-decomposition of the barycentric subdivision of the triangulation. For every k-cell in the
  * original triangulation there's k+1 associated k-cells in this triangulation. 
  */
  MIX_coord, // see docs for indexing
 /**
  * This is the standard cell decomposition (which is always a triangulation) of the boundary. So this consists of two
  * natural parts -- the part from the standard boundary, and the ideal boundary. Dual to DUAL_BDRY_coord
  */
  STD_BDRY_coord, // indexed by bcIx then icIx
 /**
  * This is the same as STD_coord except the boundary cells are thrown away. Dual to DUAL_coord
  */
  STD_REL_BDRY_coord,  // indexed by rIx      
  last_implemented_coord, 
  /**
  * This is the barycentric subdivision of STD_BDRY_coord. TODO (incomplete)
  */
  MIX_BDRY_coord, 
 /**
  * This is MIX_coord with the boundary cells thrown away. TODO (incomplete)
  */
  MIX_REL_BDRY_coord, 
 /**
  * This is the dual cellular decomposition of the boundary. Dual to STD_BDRY_coord. TODO (incomplete)
  */
  DUAL_BDRY_coord, 
 /**
  * This is the cell decomposition which is dual to STD_coord TODO (incomplete)
  */
  DUAL_REL_BDRY_coord };

 /**
  * Use this to specify if you want homology (coVariant) or cohomology (contraVariant) in a (co)homology
  * computation. See NCellularData::unmarkedGroup, NCellularData::markedGroup, NCellularData::homGroup, 
  * NCellularData::bilinearForm for usage.
  */
 enum variance_type { 
 /**
  * for Homology
  */
  coVariant, 
 /**
  * for coHomology
  */
  contraVariant };

 /**
  *  NCellularData stores chain complexes internally in a stack. The Chain Complex Locator allows
  * unique identification of a chain complex when passing requests to NCellularData::chainComplex
  * Chain complexes are stored where dim indicates the dimension of the cells in the domain of
  * the map. 
  */
 struct ChainComplexLocator {
        signed long dim; 
        homology_coordinate_system hcs;	

	/**
	 *  Initialization constructor.
	 */
	ChainComplexLocator(unsigned long newDim, homology_coordinate_system useHcs);

	/**
	 *  Copy constructor.
	 */
        ChainComplexLocator(const ChainComplexLocator &cloneMe);

	bool operator<(const ChainComplexLocator &rhs) const;
        bool operator==(const ChainComplexLocator &rhs) const;
        bool operator!=(const ChainComplexLocator &rhs) const;
        /**
         * Returns true if chain complex is of a boundary type. 
         */
        bool boundaryType() const; 
       
        void writeTextShort(std::ostream& out) const;
        void writeTextLong(std::ostream& out) const;
 };

 /**
  *  NCellularData stores chain maps internally in a stack. The Chain Map Locator allows
  * unique identification of a chain maps when passing requests to NCellularData::chainMap
  */
 struct ChainMapLocator { 
        ChainComplexLocator domain, range; 

	/**
	 *  Initialization constructor.
	 */
	ChainMapLocator(const ChainComplexLocator &Domain, const ChainComplexLocator &Range);

	/**
	 *  Copy constructor.
	 */
        ChainMapLocator(const ChainMapLocator &cloneMe);

	bool operator<(const ChainMapLocator &rhs) const;
        bool operator==(const ChainMapLocator &rhs) const;
        bool operator!=(const ChainMapLocator &rhs) const;
       
        void writeTextShort(std::ostream& out) const;
        void writeTextLong(std::ostream& out) const;
 };

 /**
  * NCellularData has several routines that require GroupLocator objects as arguments: unmarkedGroup, markedGroup, 
  *  homGroup, bilinearForm. See NCellularData::unmarkedGroup, NCellularData::markedGroup, NCellularData::homGroup 
  *  and NCellularData::bilinearForm for usage.
  */
 struct GroupLocator {
        // if its an unMarkedGroup or MarkedGroup we need to know dimension, variance, coordinates, coefficients
        unsigned long dim;
	variance_type var;
	homology_coordinate_system hcs;	
	unsigned long cof;

	/**
	 *  Initialization constructor.
	 */
	GroupLocator(unsigned long newDim, variance_type newVar, homology_coordinate_system useHcs, 
			unsigned long useCof);
	/**
	 *  Copy constructor.
	 */
        GroupLocator(const GroupLocator &cloneMe);

	bool operator<(const GroupLocator &rhs) const;
        bool operator==(const GroupLocator &rhs) const;
        bool operator!=(const GroupLocator &rhs) const;
       
        void writeTextShort(std::ostream& out) const;
        void writeTextLong(std::ostream& out) const;
 };

 /**
  * NCellularData::homGroup requires a HomLocator object as an argument.   
  */
 struct HomLocator {
	GroupLocator domain;
	GroupLocator range;

	/**
	 *  Initialization constructor.
	 */
	HomLocator(const GroupLocator &newDomain, const GroupLocator &newRange);
	/**
	 *  Copy constructor.
	 */
	HomLocator(const HomLocator &cloneMe);

	bool operator<(const HomLocator &rhs) const;
        bool operator==(const HomLocator &rhs) const;
        bool operator!=(const HomLocator &rhs) const;

        void writeTextShort(std::ostream& out) const;
        void writeTextLong(std::ostream& out) const;
 };

 /**
  * Use this enum in the FormLocator constructor to further specify which NBilinearForm
  *  you're interested in.     
  *
  * \todo cupproductForm
  */
 enum form_type { 
 /**
  * intersectionForm is the form which is dual to the cup product, computed by finding homologous 
  * classes in DUAL_coord and STD_REL_BDRY_coord (which are transverse) and intersecting them there.
  */
  intersectionForm, 
 /**
  * torsionlinkingForm is the induced pairing on the torsion classes in shifted degree.
  */
  torsionlinkingForm, 
 /**
  * evaluationForm is the homology-cohomology pairing. 
  */
  evaluationForm, 
 /**
  * cupproductForm is the cup product pairing TODO (incomplete)
  */
  cupproductForm };

 /**
  * NCellularData::bilinearForm requires a FormLocator object as an argument.   
  */
 struct FormLocator {
	GroupLocator ldomain;
	GroupLocator rdomain;
        form_type ft;

	/**
	 *  Initialization constructor.
	 */
	FormLocator(form_type FT, const GroupLocator &newLdomain, const GroupLocator &newRdomain);
	/**
	 *  Copy constructor.
	 */
	FormLocator(const FormLocator &cloneMe);

	bool operator<(const FormLocator &rhs) const;
        bool operator==(const FormLocator &rhs) const;
        bool operator!=(const FormLocator &rhs) const;

        void writeTextShort(std::ostream& out) const;
        void writeTextLong(std::ostream& out) const;
 };

 /**
  * Enum for specifying characteristic parts of the manifold. 
  */
 enum submanifold_type { 
  /**
   * The entire manifold is specified. 
   */
  whole_manifold, 
  /**
   * The non-ideal part of the boundary is specified.
   */
  standard_boundary, 
  /**
   * The ideal part of the boundary is specified.
   */
  ideal_boundary };

 /**
  * NCellularData::groupPresentation requires a groupPresentationLocator as
  * an argument.
  */
 struct GroupPresLocator {
   submanifold_type sub_man; // specify which submanifold
   unsigned long component_index; // which component of the submanifold

   /**
    *  Initialization constructor.
    */
   GroupPresLocator( submanifold_type ST, unsigned long CI );
   /**
    *  Copy constructor.
    */
   GroupPresLocator( const GroupPresLocator &cloneMe );
   
   bool operator<(const GroupPresLocator &rhs) const;
   bool operator==(const GroupPresLocator &rhs) const;
   bool operator!=(const GroupPresLocator &rhs) const;

   void writeTextShort(std::ostream& out) const;
   void writeTextLong(std::ostream& out) const;
 };

 /**
  * NCellularData::homGroupPresentation requires a homGroupPresentationLocator as
  * an argument.
  */
 struct HomGroupPresLocator {
   submanifold_type inclusion_sub_man; // specify which submanifold
   unsigned long subman_component_index; // which component of the submanifold

   /**
    *  Initialization constructor.
    */
   HomGroupPresLocator( submanifold_type ST, unsigned long CI );
   /**
    *  Copy constructor.
    */
   HomGroupPresLocator( const HomGroupPresLocator &cloneMe );
   
   bool operator<(const HomGroupPresLocator &rhs) const;
   bool operator==(const HomGroupPresLocator &rhs) const;
   bool operator!=(const HomGroupPresLocator &rhs) const;

   void writeTextShort(std::ostream& out) const;
   void writeTextLong(std::ostream& out) const;
 };

 /**
  * For the purpose of various homological computations we need data on how  
  * k-cells are incident to (k-1)-cells.  This struct is used to encode that
  * data at the level of the universal cover of the manifold. 
  */
 struct coverFacetData {
  /**
   * This call is incident to cell number cellNo, taken from the chain complex
   * indexing. 
   */
  unsigned long cellNo;
  /**
   * sig=+1 or -1 depending on whether or not the boundary orientation matches 
   * or not. 
   */ 
  signed long sig; 
  /**
   * The path from the basepoint of this cell to cell numbered cellNo is a closed 
   * loop, therefore generates an element of pi1 of the fundamental group of the
   * manifold.  This is one such representative. 
   */
  NGroupExpression trans;
   /**
    * Constructor.
    */
  coverFacetData( unsigned long cellN, signed long Sig, const NGroupExpression& word );
   /**
    * Copy constructor.
    */
  coverFacetData( const coverFacetData& cloneMe );
   /**
    * Assignment operator.
    */
   coverFacetData operator=(const coverFacetData &rhs);
   /**
    * Output operator.
    */
   std::string stringValue() const;
 };

private:
    /**
     * Stored pointer to a valid triangulation. All routines use either tri4 or tri3
     * as reference, and only one will be allocated. This is the triangulation NCellularData is
     * initialised by.
     */
    Dim4Triangulation* tri4;
    NTriangulation* tri3;

    // for integer chain complexes
    std::map< ChainComplexLocator, NMatrixInt* > integerChainComplexes; 
    // for maps of integer chain complexes
    std::map< ChainMapLocator, NMatrixInt* > integerChainMaps;
    // for abelian groups
    std::map< GroupLocator, NAbelianGroup* > abelianGroups;
    // for marked abelian groups
    std::map< GroupLocator, NMarkedAbelianGroup* > markedAbelianGroups;
    // for homomorphisms of marked abelian group
    std::map< HomLocator, NHomMarkedAbelianGroup* > homMarkedAbelianGroups;
    // for bilinear forms
    std::map< FormLocator, NBilinearForm* > bilinearForms;
    // for group presentations
    std::map< GroupPresLocator, NGroupPresentation* > groupPresentations;
    // for homomorphisms of group presentations
    std::map< HomGroupPresLocator, NHomGroupPresentation* > homGroupPresentations;
    // for alexander module chain complexes
    std::map< ChainComplexLocator, NMatrixRing< NSVPolynomialRing< NLargeInteger > >* > alexanderChainComplexes;

    /** 
     * numStandardCells = number of cells in the standard CW decomposition in dimensions: 0, 1, 2, 3, (4).
     *                  == numNonIdealCells + numIdealCells 
     * numDualCells     = number of cells in the dual CW decomposition in dimension: 0, 1, 2, 3, (4). 
     * numMixCells      = number of cells in the mixed CW decomposition in dimensions: 0, 1, 2, 3, (4) 
     *                    (see constructor for details)
     * numStandardBdryCells = number of cells in the standard CW decomposition of the boundary 
     *                        in dimensions: 0, 1, 2, (3). 
     *                  == numNonIdealBdryCells + numIdealCells
     * numRelativeCells = number of cells from the standard CW-decomposition rel boundary.
     *
     * numNonIdealBdryCells == numStandardBdryCells - numIdealCells: 0, 1, 2, (3). 
     * numNonIdealCells = number of non-ideal cells in standard CW-decomposition in dimension: 0, 1, 2, 3, (4) 
     * numIdealCells    = number of ideal cells in standard CW-decomposition in dimension: 0, 1, 2, (3) 
     */
   unsigned long numStandardCells[5], numDualCells[5],    numMixCells[5],          numStandardBdryCells[4], 
                 numNonIdealCells[5], numIdealCells[4],   numNonIdealBdryCells[4], 
	         numRelativeCells[5], numDualRelCells[5], numMixRelCells[5],       numMixBdryCells[4], 
		 numDualBdryCells[4];

    /** 
     * Chain complex indexing and orientation and boundary-map conventions:
     *
     * nicIx is indexing for the standard cells of the non-ideal variety. This keeps track
     *       of the vertex/edge/face/tet/pent index of the associated cell. 
     *       We orient these cells via their characteristic maps, so boundary maps
     *       signs given by getCodimOneObjectMapping().sign(), all ideal boundary bits given +1
     *       orientation.
     *  icIx is Indexing for the standard cells of the ideal variety.  Ie this are the ideal
     *       boundaries of non-ideal cells that *have* ideal boundaries.
     *       icIx[i][j] is stored as (i+2)*[index of (i+1)-simplex containing the j-th
     *       ideal i-cell in its boundary] + [the corresponding index of this in (i+1)-simplex].
     *       We orient these cells as the boundary of this i+1-simplex, so the boundary map is
     *       -1*sign corresponding boundary map of the i+1 simplex.
     *  dcIx is Indexing for the dual cells.  dcIx[i] indexes the non-ideal, nonboundary
     *       standard cells of dimension (3 or 4)-i. We orient these via the getEmbeddings
     *       conventions in Regina.
     *  rIx  relative chain complex for standard homology rel boundary. 
     *  bcIx is Indexing for the boundary cells, standard decomposition, ignoring the ideal ends
     *       of standard cells. 
     * 
     * We systematically use the outwards orientation convention to define the boundary maps.
     **/
    std::vector< std::vector<unsigned long> > nicIx, icIx, dcIx, bcIx, rIx;

    // generic type for holding chain complex data 
    // this will be a 2x2 grid 1st coordinate the cell index and 
    //  2nd coordinate the indices of the incident faces.
    public:
    typedef NSparseGrid< coverFacetData > ccMapType; 
    // there is a ccMapType for chain complexes (9 coordinate systems, 
    //  all dimensions)
    typedef std::map< ChainComplexLocator, ccMapType* > ccCollectionType;
    private:
    // the "master" chain complex for the manifold. 
    ccCollectionType genCC;  
    typedef std::map< ChainMapLocator, ccMapType* > cmCollectionType;
    cmCollectionType genCM;

    /** 
     * Chain maps: 
     * 
     *   standard: 
     * sbiCM[i]  :  sbCC[i]   -> sCC[i]      std coords, boundary inclusion             
     * strCM[i]  :  sCC[i]    -> srCC[i]     std coords, relative projection       
     * schCM[i]  :  srCC[i+1] -> sbCC[i]     std coords, connecting hom 
     *
     *     dual:
     * dbiCM[i]  :  dbCC[i]   -> dCC[i]      dual coords, boundary inclusion     TODO & TESTS
     * dtrCM[i]  :  dCC[i]    -> drCC[i]     dual coords, relative projection    TODO & TESTS
     * dchCM[i]  :  drCC[i+1] -> dbCC[i]     dual coords, connecting hom         TODO & TESTS
     * 
     *    mixed:
     * mbiCM[i]  :  mbCC[i]   -> mCC[i]      mixed coords, boundary inclusion    TODO & TESTS
     * mtrCM[i]  :  mCC[i]    -> mrCC[i]     mixed coords, relative projection   TODO & TESTS
     * mchCM[i]  :  mrCC[i+1] -> mbCC[i]     mixed coords, connecting hom        TODO & TESTS
     *
     *   inter-coordinate maps, all dimension-preserving:
     * smCM   :   sCC -> mCC       standard to mixed
     * dmCM   :   dCC -> mCC       dual to mixed
     * smbCM  :  sbCC -> mbCC      standard to mixed,   boundary map  TODO & TESTS
     * dmbCM  :  dbCC -> mbCC      dual to mixed,       boundary map  TODO & TESTS
     * srmCM  :  srCC -> mrCC      standard to mixed,   relative map  TODO & TESTS      
     * drmCM  :  drCC -> mrCC      dual to mixed,       relative map  TODO & TESTS 
     */
    std::vector< NMatrixInt* > sbiCM, strCM, schCM,   dbiCM, dtrCM, dchCM,   mbiCM, mtrCM, mchCM,  
 			       smCM, dmCM,            smbCM, dmbCM,          srmCM, drmCM;

   /**
    *  Data that stores the maximal tree in the dual 1-skeleton.  n=dimension of top-dimensional simplices.
    *
    *  maxTreeStd    stores the edges dual to co-dimension 1 simplices, indexed by nicIx[n-1]
    *  maxTreeStB    boundary edges dual to co-dimension 2 boundary faces, indexed by bcIx[n-2]
    *  maxTreeIdB    boundary edges dual to co-dimension 2 ideal boundary faces, indexed by icIx[n-2]
    *  maxTreeSttIdB edges connecting top-dimensional simplices barycentres to boundary cd1 barycentres,
    *										indexed by icIx[n-1]
    */
   std::set< unsigned long > maxTreeStd, maxTreeStB, maxTreeIdB, maxTreeSttIdB; // 1-cells in maximal tree

   struct dim4BoundaryFaceInclusion
    { Dim4Tetrahedron *firsttet, *secondtet;
      unsigned long firstfacnum, secondfacnum; };

   struct dim4BoundaryEdgeInclusion
    { std::vector< Dim4Tetrahedron* > tet;
      std::vector< unsigned long > edgenum; 
      std::vector< NPerm4 > edginc; };

   struct dim4BoundaryVertexInclusion
    { std::vector< Dim4Tetrahedron* > tet;
      std::vector< unsigned long > vrtnum; 
      std::vector< NPerm4 > vrtinc; };

   struct dim3BoundaryEdgeInclusion
    { NFace *firstfac, *secondfac;
      unsigned long firstedgnum, secondedgnum; };

   struct dim3BoundaryVertexInclusion
    { std::vector< NFace* > face;
      std::vector< unsigned long > vrtnum; 
      std::vector< NPerm3 > vrtinc; };

    /**
     * Normal orientations for cells Regina does not naturally give normal orientations to. We will have
     * to build this by hand.  For a dim4Triangulation we need to build the boundary NTriangulation class
     * Ben suggests iterating through the dim4Boundary components, appending them into an NTriangulation
     * using dim4BoundaryComponent::getTriangulation and NTriangulation::insertTriangulation.  Building
     * up the NTriangulation -> Dim4Triangulation indices using dim4BoundaryComponent::getEdge getFace, etc,
     * maps.  Then we can build the classes below.  For 3-manifold and boundary 2-manifold triangulations
     * we'll likely have to build these by hands.  The edges shouldn't be bad, the vertices might take work.
     *
     * normalsDim4BdryFaces is a vector that assigns to the i-th boundary face [tri4->getFace(bcIx[2][i])]
     *  the two boundary tetrahedra that contain it and the face number of the face in the tetrahedron. 
     *
     * normalsDim4BdryEdges is a vector that assigns to the i-th boundary edge [tri4->getFace(bcIx[1][i])]
     *  the circle of tetrahedra incident to that edge, with edginc[2] and edginc[3] forming the normal orientation
     *  in agreement with the indexing of tet. 
     *
     * normalsDim4BdryVertices is a vector that assigns to the i-th boundary vertex [tri4->getVertex(bcIx[0][i])]
     *  the sphere of tetrahedra incident to that vertex, with vrtinc[1], vrtinc[2], vrtinc[3] forming a normal
     *  orientation.
     *
     * normalsDim3BdryEdges is a vector that assigns to the i-th boundary face [tri3->getEdge(bcIx[1][i])]
     *  the two boundary faces that contain it and the edge number of the edge in the NFace.  
     *
     * normalsDim3BdryVertices is a vector that assigns to the i-th boundary vertex [tri3->getVertex(bcIx[0][i])]
     *  the circle of faces incident to that vertex, with vrtinc[1] and vrtinc[2] forming the normal orientation
     *  in agreement with the indexing of face.  TODO
     */
   std::vector< dim4BoundaryFaceInclusion >   normalsDim4BdryFaces;
   std::vector< dim4BoundaryEdgeInclusion >   normalsDim4BdryEdges;  
   std::vector< dim4BoundaryVertexInclusion > normalsDim4BdryVertices;
   std::vector< dim3BoundaryEdgeInclusion >   normalsDim3BdryEdges;
   std::vector< dim3BoundaryVertexInclusion > normalsDim3BdryVertices; 

   /**
    * Keeps track of numbers of standard and ideal boundary components. Eventually used to initialize
    * length of stdBdryPi1Gen and idBdryPi1Gen lengths. These are initialized in buildMaximalTree()
    */
   unsigned long numStdBdryComps, numIdealBdryComps;

    /**
     * Given a co-dimension 1 object in the standard boundary of the triangulation, you'd sometimes
     *  like to know which boundary component it belongs to. This stores those indices in a vector. 
     *  Input is the bcIx[n-2] index. Initialized in buildMaximalTree()
     */
   std::vector< unsigned long > stdBdryCompIndexCD1;
    /**
     * Given a co-dimension 1 object in the standard boundary of the triangulation, you'd sometimes
     *  like to know which boundary component it belongs to. This stores those indices in a vector. 
     *  Input is the icIx[n-2] index. Initialized in buildMaximalTree()
     */
   std::vector< unsigned long > idBdryCompIndexCD1; 
   /**
    * Given the i-th std boundary component, the j-th generator of pi1 of this
    *  component is represented by bcIx[n-2][ stdBdryPi1Gen[i][j] ] Initialized in buildMaximalTree()
    */
   std::vector< std::vector< unsigned long > > stdBdryPi1Gen;
   /**
    * Given ideal boundary component i, the j-th generator of pi1 of this
    *  component is represented by icIx[n-2][ stdBdryPi1Gen[i][j] ] Initialized in buildMaximalTree()
    */
   std::vector< std::vector< unsigned long > > idBdryPi1Gen;

    /**
     * Routine returns true if and only if tet is represents an edge in the maximal tree for the dual 1-skeleton of the triangulation.
     *  Any tetrahedron from the triangulation can potentially represent an edge. Corresponds to maxTreeStd, maxTreeStB, 
     *  maxTreeIdB, and MaxTreeSttIdB respectively.
     */
    bool inMaximalTree(const Dim4Tetrahedron* tet) const;
    bool inMaximalTree(const Dim4Face* fac) const;
    bool inMaximalTree(const Dim4Tetrahedron* tet, unsigned long num) const;
    bool inMaximalTree(const Dim4Pentachoron* pen, unsigned long num) const;
    /**
     * Same routines for 3-dimensional triangulations.
     */
    bool inMaximalTree(const NFace* fac) const;
    bool inMaximalTree(const NEdge* edg) const;
    bool inMaximalTree(const NFace* fac, unsigned long num) const;
    bool inMaximalTree(const NTetrahedron* tet, unsigned long num) const;
    
   /**
    *  During initialization many revere-lookups are needed.  We proved them in one place.
    * See cellulardata.lookups.cpp for implementations.
    */ 
   unsigned long nicIxLookup(const NVertex* vrt) const;
   unsigned long nicIxLookup(const NEdge* edg) const;
   unsigned long nicIxLookup(const NFace* fac) const;
   unsigned long nicIxLookup(const NTetrahedron* tet) const;
   unsigned long nicIxLookup(const Dim4Vertex* vrt) const;
   unsigned long nicIxLookup(const Dim4Edge* edg) const;
   unsigned long nicIxLookup(const Dim4Face* fac) const;
   unsigned long nicIxLookup(const Dim4Tetrahedron* tet) const;
   unsigned long nicIxLookup(const Dim4Pentachoron* pen) const;

   unsigned long icIxLookup(const NEdge* edg, unsigned long i) const;
   unsigned long icIxLookup(const NFace* fac, unsigned long i) const;
   unsigned long icIxLookup(const NTetrahedron* tet, unsigned long i) const;
   unsigned long icIxLookup(const Dim4Edge* edg, unsigned long i) const;
   unsigned long icIxLookup(const Dim4Face* fac, unsigned long i) const;
   unsigned long icIxLookup(const Dim4Tetrahedron* tet, unsigned long i) const;
   unsigned long icIxLookup(const Dim4Pentachoron* edg, unsigned long i) const;

   unsigned long dcIxLookup(const NTetrahedron* tet) const; 
   unsigned long dcIxLookup(const NFace* fac) const; 
   unsigned long dcIxLookup(const NEdge* edg) const; 
   unsigned long dcIxLookup(const NVertex* vrt) const; 
   unsigned long dcIxLookup(const Dim4Pentachoron* pen) const; 
   unsigned long dcIxLookup(const Dim4Tetrahedron* tet) const; 
   unsigned long dcIxLookup(const Dim4Face* fac) const; 
   unsigned long dcIxLookup(const Dim4Edge* edg) const; 
   unsigned long dcIxLookup(const Dim4Vertex* vrt) const; 

   unsigned long bcIxLookup(const NVertex* vrt) const; 
   unsigned long bcIxLookup(const NEdge* edg) const; 
   unsigned long bcIxLookup(const NFace* fac) const; 
   unsigned long bcIxLookup(const Dim4Vertex* vrt) const;
   unsigned long bcIxLookup(const Dim4Edge* edg) const;
   unsigned long bcIxLookup(const Dim4Face* fac) const;
   unsigned long bcIxLookup(const Dim4Tetrahedron* tet) const;

   unsigned long rIxLookup(const NVertex* vrt) const;
   unsigned long rIxLookup(const NEdge* edg) const;
   unsigned long rIxLookup(const NFace* fac) const;
   unsigned long rIxLookup(const NTetrahedron* tet) const;
   unsigned long rIxLookup(const Dim4Vertex* vrt) const;
   unsigned long rIxLookup(const Dim4Edge* edg) const;
   unsigned long rIxLookup(const Dim4Face* fac) const;
   unsigned long rIxLookup(const Dim4Tetrahedron* tet) const;
   unsigned long rIxLookup(const Dim4Pentachoron* pen) const;

   unsigned long pi1Lookup(const Dim4Tetrahedron* tet) const;
   unsigned long pi1Lookup(const Dim4Face* fac) const;
   unsigned long pi1Lookup(const Dim4Tetrahedron* tet, unsigned long num) const;
   unsigned long pi1Lookup(const Dim4Pentachoron* pen, unsigned long num) const;
   unsigned long pi1Lookup(const NFace* fac) const;
   unsigned long pi1Lookup(const NEdge* edg) const;
   unsigned long pi1Lookup(const NFace* fac, unsigned long num) const;
   unsigned long pi1Lookup(const NTetrahedron* tet, unsigned long num) const;

   /**
    *  Internal routines to set up chain complexes. 
    */
   void fillStandardHomologyCC();   void fillDualHomologyCC();   void fillMixedHomologyCC();
   void fillBoundaryHomologyCC(); // void fillBoundaryDualHomologyCC()  // void fillBoundaryMixedHomologyCC() // TODO
   void fillRelativeHomologyCC(); // void fillRelativeDualHomologyCC()  // void fillRelativeMixedHomologyCC() // TODO
   /**
    * Internal routines to set up chain maps. 
    */
   void fillStandardToMixedHomCM(); // void fillDualToMixedHomCM(); 

   /**
    * internal routine to build the maximal tree in the dual 1-skeleton, suitable for computing
    *  pi1 information about the manifold. Called after buildExtraNormalData() and before buildFundGrpPres()
    */
   void buildMaximalTree();

   /**
    *  Routine constructs tables normalsDim4BdryFaces normalsDim4BdryEdges normalsDim3BdryEdges normalsDim3BdryVertices
    * for homology and fundamental group computations.  Called before buildMaximalTree()
    */
   void buildExtraNormalData();

   /**
    *  Internal routine to construct the fundamental group presentation for the ambient manifold.
    */
   void buildFundGrpPres() const;

public:

    /**
     * Takes as input an Dim4Triangulation -- this class will make its own internal
     * copy of the triangulation so it is okay to deallocate whatever you pass, after
     * the initial call is made. 
     *
     * @param the triangulation to use -- must be valid Dim4Triangulation. 
     */
    NCellularData(const Dim4Triangulation &input);
    /**
     * Takes as input an NTriangulation -- this class will make its own internal
     * copy of the triangulation so it is okay to deallocate whatever you pass, after
     * the initial call is made. 
     *
     * @param pointer to the triangulation to use -- must be a valid NTriangulation. 
     */
    NCellularData(const NTriangulation &input);
    /**
     * Copy constructor.
     *
     * @param h the homological data to clone.
     */
    NCellularData(const NCellularData& cloneMe);
    /**
     * Destructor.
     */
    ~NCellularData();

    /**
     * Short text representation as required by SharableObject.
     * This only prints out precomputed data, so if you haven't
     * done anything with this NCellularData object, this
     * string will be quite short. 
     *
     * @param out the stream to write to.
     */
    void writeTextShort(std::ostream& out) const;

    /**
     * Longer text representation.
     *
     * @param out the stream to write to.
     */
    void writeTextLong(std::ostream& out) const;

    /**
     * @param coord_system specifies the cell complex and dimension. 
     * @return the number of cells of the given dimension in the cell complex
     *  specified by coord_system.
     */
    unsigned long cellCount( const ChainComplexLocator &coord_system) const;

    /**
     *  The number of path-components of a given submanifold type. 
     */
    unsigned long components( submanifold_type ctype ) const; 
   
    /**
     * The Euler characteristic of the manifold, computed from
     * the dual CW-decomposition.
     *
     * This routine calculates the Euler characteristic of the
     * corresponding compact triangulated 3-manifold, with each ideal
     * vertex treated as a surface boundary component.
     *
     * This routine returns the same value as
     * NTriangulation::getEulerCharManifold(), though it computes it
     * in a different way.
     *
     * On the other hand, this routine differs from
     * NTriangulation::getEulerCharTri(), which handles ideal triangulations
     * in a non-standard way (treating each ideal vertex as just a single
     * vertex).
     *
     * @return the Euler characteristic of the corresponding compact
     * triangulated 3-manifold.
     */
    long eulerChar() const;

    /**
     * Computes the Poincare polynomial -- this is the polynomial such that the 
     * coefficient of t^i is the rank of the i-th homology group of the manifold. 
     */
    NSVPolynomialRing< NLargeInteger > poincarePolynomial() const;

    /**
     *  If this is a 4-manifold, this routine returns the signature of the H_2 intersection form.
     * If the manifold is not orientable (or not a 4-manifold) this routine returns 0.  
     */
    long int signature() const;

    /**
     * Determine if the torsion linking form is hyperbolic.  Returns true if it is, false if not, 
     * unknown if the request doesn't make sense.
     */
    NTriBool torsionLinkingFormIsHyperbolic() const;

    /**
     * Verifies that the maps used to define the various homology groups for the manifold are
     * actually chain complexes. 
     *
     * This procedure is largely for debugging purposes as in any release this should always produce
     *  true.
     *
     * @return true if all homomorphisms are defined by matrices that really are chain maps of
     *         chain complexes.
     */
    bool chainComplexesVerified() const;

    /**
     * Runs through all the various standard homomorphisms between boundary, standard, dual and mixed
     *  homologies and checks the matrices defining them really are chain maps.
     *
     * @return true if all homomorphisms are defined by matrices that really are chain maps of
     *         chain complexes.
     */
    bool chainMapsVerified() const;

    /**
     * If all the chain complexes check out, might as well verify a few basic isomorphisms as well. 
     * These are the isomorphisms between homology in standard, dual and mixed coordinates. 
     * Optional parameters allow for the check to be done on cohomology, and with coefficients.
     *
     * This is largely for debugging purposes as in any release this should always produce an answer
     * of true. 
     *
     * @return true provided every natural map between the various homology groups computable that
     *         should be isomorphisms, actually are isomorphisms. 
     */
    bool coordinateIsomorphismsVerified(variance_type var=coVariant, unsigned long coef=0) const;

    /**
     * Similarly, check that the homology long exact sequence of the pair (M, bdry M)
     * holds.  At present this isn't fully implemented as it only checks to see that the
     * image of one map is isomorphic to the kernel of the next, and that the composite of
     * one map with the next is zero.  If the image/kernel is infinite this is only a partial
     * check of exactness.  Eventually this will be patched. Optional parameters allow for
     * the corresponding check for cohomology, and with coefficients.
     */
    bool homologyLESVerified(variance_type var=coVariant, unsigned long coef=0) const;

    /**
     * Checks to see if Poincare duality holds.  If the manifold is orientable, it uses integer
     * coefficients, if non-orientable it uses Z_2-coefficients.
     */
    bool poincareDualityVerified() const;

    /**
     * Ensure the intersection forms do what they're supposed to do. 
     */
    bool intersectionFormsVerified() const;

    /**
     * Computes a chain complex or retrieves it from the precomputed pile.
     */
    const NMatrixInt* integerChainComplex( const ChainComplexLocator &c_desc ) const;

    /**
     * Computes a map of chain complexes or retrieves it from the precomputed pile.
     */
    const NMatrixInt* integerChainMap( const ChainMapLocator &m_desc ) const;

    /**
     * Computes an NAbelianGroup or retrieves it from the precomputed pile. 
     */
    const NAbelianGroup* unmarkedGroup( const GroupLocator &g_desc) const;
    /**
     * Computes an NMarkedAbelianGroup or retrieves it from the precomputed pile. 
     */
    const NMarkedAbelianGroup* markedGroup( const GroupLocator &g_desc) const;
    /**
     *  Computes an NHomMarkedAbelianGroup or retrieves it from the precomputed pile. 
     *
     *  At present there's 4 basic varieties of homomorphisms that Regina knows how to 
     *  compute.  
     *
     *  1) A pure change-of-coefficients map.  These are maps of the form H_i(*,R_1) --> H_i(*,R_2) or
     *     H^i(*,R_2) --> H^i(*,R_1) where R_1 --> R_2 is a quotient map of rings, and
     *     * is either M, (M,\partial M) or (\partial M), ie: any homology_coordinate_system
     *     is valid. todo: add full Bockstein sequence
     *     @pre h_desc.domain.(dim, var, hcs) == h_desc.range.(dim, var, hcs). 
     *     @pre h_desc.domain.cof is an integer multiple of h_desc.range.cof 
     *
     *  2) Maps induced by subdivision of the cell complex maps. 
     *     @pre h_desc.domain.(var, dim) == h_desc.range.(var, dim)
     *     @pre if var==coVariant,    h_desc.domain.hcs == STD_coord or DUAL_coord and
     *                                h_desc.range.hcs == MIX_coord
     *          if var==contraVariant h_desc.domain.hcs == MIX_coord and
     *                                h_desc.range.hcs == STD_coord or DUAL_coord
     *     @pre h_desc.domain.cof is an integer multiple of h_desc.range.cof
     *
     *  3) Homology long exact sequence of the pair (M,\partial M) maps. 
     *     @pre if var==coVariant,    h_desc.domain.hcs == STD_coord, STD_REL_BDRY_coord or STD_BDRY_coord
     *             and respectively:  h_desc.range.hcs == STD_REL_BDRY_coord, STD_BDRY_coord or STD_coord
     *          if var==contraVariant h_desc.domain.hcs == STD_coord, STD_BDRY_coord or STD_REL_BDRY_coord
     *             and respectively:  h_desc.range.hcs == STD_BDRY_coord, STD_REL_BDRY_coord or STD_coord
     *     @pre h_desc.domain.var == h_desc.range.var
     *     @pre h_desc.domain.dim and h_desc.range.dim as in the homology long exact sequence
     *     @pre h_desc.domain.cof is an integer multiple of h_desc.range.cof
     *
     *  4) Strict Poincare Duality maps, these are maps of the form H_i(M;R_1) --> H^{n-i}(M,\partial M;R_2)
     *     or H^i(M;R_1) --> H_{n-i}(M,\partial M;R_2). 
     *     @pre h_desc.domain.var and h_desc.range.var opposite. 
     *     @pre h_desc.domain.dim and h_desc.range.dim add up to the dimension of the manifold. 
     *     @pre h_desc.domain.hcs == DUAL_coord and h_desc.range.hcs == STD_REL_BDRY_coord
     *     @pre h_desc.domain.cof is an integer multiple of h_desc.range.cof, 
     *      both must be Z_2 if the manifold is not orientable.
     *
     *  5) Convienience maps.  These are natural maps users might be interested in that are composites of maps (1)--(4)
     *     and their inverses.  TODO
     */
    const NHomMarkedAbelianGroup* homGroup( const HomLocator &h_desc) const;

    /**
     *  Computes an NBilinearForm or retrieves it from the precomputed pile. 
     *
     *  At present there's plans for 5 bilinear form types that will be requestable.  This procedure
     *  is not fully implemented yet and potentially buggy.  
     *
     *  1) Homology-Cohomology pairing <.,.>  ? ie: H_i(M;R) x H^i(M;R) --> R  where R is the coefficients
     *     
     *
     *  2) Intersection product               ie: (dual)H_i(M;R) x (std rel bdry)H_j(M;R) --> (mix)H_{(i+j)-n}(M;R)
     *                                            (dual)H_i(M;R) x (dual)H_j(M;R) --> (mix)H_{(i+j)-n}(M;R)
     *                                            (std)H_i(M;R) x (std rel bdry)H_j(M;R) --> (mix)H_{(i+j)-n}(M;R)
     *
     *  3) Torsion linking form               ie: tH_i(M;Z) x tH_j(M;Z) --> Q/Z 
     *     (not yet implemented)                  when i+j=n-1, i,j>0. So for 3-manifolds only defined for i,j = 1,1
     *						  and for 4-manifolds i,j=1,2 or 2,1. 
     *      Present implementation has      The range of the form in Q/Z will be taken to be Z_k where k is the largest
     *      ldomain and rdomain given        invariant factor of tH_j(M;Z), so we implement the range as Z_k with
     *      trivial presentations            trivial presentation 0 --> Z --k--> Z ---> Z_k ---> 0
     *
     *  4) cup products                       ie: H^i(M;R) x H^j(M;R) --> H^{i+j}(M;R)
     *     (not yet implemented)              various coordinate systems  TODO
     *     
     */
    const NBilinearForm* bilinearForm( const FormLocator &f_desc ) const;

    /**
     *  Describes presentations of various groups associated to the manifold.  There is the fundamental
     *  group of the entire manifold, and fundamental groups of the various boundary components.
     *
     *  The fundamental groups are computed by first finding a maximal tree in the dual 1-skeleton to
     *  the manifold.  Unlike NTriangulation and Dim4Triangulation's maximal forest routines, this routine
     *  produces a maximal tree in the dual 1-skeleton that restricts to maximal trees in the boundary
     *  and ideal boundaries of the manifold. So it is quite a bit larger in general than the NTriangulation
     *  and Dim4Triangulation routine outputs. But this allows for computations of maps between groups.
     *  The generators of pi1 are the dual 1-cells not in the maximal tree. They are indexed in this order:
     *
     *  First the dual 1-cells that cross co-dimension 1 simplices from the triangulation. 
     *  Second the dual 1-cells in the standard boundary that cross co-dimension 2 simplices from the
     *   boundary triangulation. 
     *  Third, the dual 1-cells in the ideal boundary that cross co-dimension 2 ideal simplices from the
     *   ideal boundary triangulation. 
     *  Fourth, the dual 1-cells connecting top-dimensional simplex barycentres to ideal boundary simplex
     *   barycentres.
     */
    const NGroupPresentation* groupPresentation( const GroupPresLocator &g_desc ) const;

    /**
     *  Describes the homomorphisms from the fundamental group of the boundary components to the
     *  fundamental group of the manifold's components. This routine uses the dual cellular
     *  coordinates, constructing a maximal forest in the dual 1-skeleton which restricts to 
     *  maximal forests in the boundary and ideal boundary components. 
     */
    const NHomGroupPresentation* homGroupPresentation( const HomGroupPresLocator &h_desc ) const;

    /**
     *  Chain complex for the Alexander module of the manifold.  Returns NULL if this does not
     * make sense -- i.e. if the 1st Betti number of the manifold is not 1. Presently only the
     * C_2 -> C_1 -> C_0 part of the complex is defined, in dual coordinates, for 4-manifolds.
     */
    const NMatrixRing< NSVPolynomialRing< NLargeInteger > >* alexanderChainComplex( const ChainComplexLocator &a_desc ) const;

    /**
     *  Computes the presentation matrix for the 1-dimensional Alexander module. 
     *  Returns null if rank H1 != 1. 
     */
    std::auto_ptr< NMatrixRing< NSVPolynomialRing< NLargeInteger > > > alexanderPresentationMatrix() const;

    /**
     *  Computes the H1 Alexander ideal. Tries to reduce it as much as possible.
     */
    std::auto_ptr< std::list< NSVPolynomialRing< NLargeInteger > > > alexanderIdeal() const;
};


/*@}*/

// Inline functions for NCellularData

// copy constructor
inline NCellularData::NCellularData(const NCellularData& g) : ShareableObject(),
        tri4(clonePtr(g.tri4)), tri3(clonePtr(g.tri3)), 
 // chain complex indexing
	nicIx(g.nicIx), icIx(g.icIx), dcIx(g.dcIx), bcIx(g.bcIx), rIx(g.rIx), 
 // chain maps 
	sbiCM(g.sbiCM.size()), strCM(g.strCM.size()), schCM(g.schCM.size()), 
        dbiCM(g.dbiCM.size()), dtrCM(g.dtrCM.size()), dchCM(g.dchCM.size()), 
        mbiCM(g.mbiCM.size()), mtrCM(g.mtrCM.size()), mchCM(g.mchCM.size()), 
        smCM(g.smCM.size()),   dmCM(g.dmCM.size()),   smbCM(g.smbCM.size()), 
        dmbCM(g.dmbCM.size()), srmCM(g.srmCM.size()), drmCM(g.drmCM.size())
{
// copy all the pre-computed data. 
// the "master" chain complex collection for the manifold. 
ccCollectionType::const_iterator ccmi;
for (ccmi = g.genCC.begin(); ccmi != g.genCC.end(); ccmi++)
 genCC.insert( std::pair< ChainComplexLocator, ccMapType* >(ccmi->first, clonePtr(ccmi->second) ) );
// the "master" chain map collection for the manifold. 
cmCollectionType::const_iterator cmi;
for (cmi = g.genCM.begin(); cmi != g.genCM.end(); cmi++)
 genCM.insert( std::pair< ChainMapLocator, ccMapType* >(cmi->first, clonePtr(cmi->second) ) );
 // integer chain complexes
std::map< ChainComplexLocator, NMatrixInt* >::const_iterator ci;
for (ci = g.integerChainComplexes.begin(); ci != g.integerChainComplexes.end(); ci++) integerChainComplexes.insert( 
 std::pair< ChainComplexLocator, NMatrixInt* >( ci->first, clonePtr(ci->second) ) );
 // integer chain maps
std::map< ChainMapLocator, NMatrixInt* >::const_iterator mi;
for (mi = g.integerChainMaps.begin(); mi != g.integerChainMaps.end(); mi++) integerChainMaps.insert( 
 std::pair< ChainMapLocator, NMatrixInt* >( mi->first, clonePtr(mi->second) ) );
 // abelianGroups
std::map< GroupLocator, NAbelianGroup* >::const_iterator abi;
for (abi = g.abelianGroups.begin(); abi != g.abelianGroups.end(); abi++) abelianGroups.insert( 
 std::pair< GroupLocator, NAbelianGroup* >( abi->first, clonePtr(abi->second) ) );
 // markedAbelianGroups
std::map< GroupLocator, NMarkedAbelianGroup* >::const_iterator mabi;
for (mabi = g.markedAbelianGroups.begin(); mabi != g.markedAbelianGroups.end(); mabi++)	markedAbelianGroups.insert( 
 std::pair< GroupLocator, NMarkedAbelianGroup* >(mabi->first, clonePtr(mabi->second) ) );
 // homMarkedAbelianGroups
std::map< HomLocator, NHomMarkedAbelianGroup* >::const_iterator hmabi;
for (hmabi = g.homMarkedAbelianGroups.begin(); hmabi != g.homMarkedAbelianGroups.end(); hmabi++) 
 homMarkedAbelianGroups.insert( std::pair< HomLocator, NHomMarkedAbelianGroup* >(hmabi->first, clonePtr(hmabi->second) ) );
 // bilinearForms
std::map< FormLocator, NBilinearForm* >::const_iterator fi;
for (fi = g.bilinearForms.begin(); fi != g.bilinearForms.end(); fi++) 
 bilinearForms.insert( std::pair< FormLocator, NBilinearForm* >(fi->first, clonePtr(fi->second) ) );
 // groupPresentations
std::map< GroupPresLocator, NGroupPresentation* >::const_iterator pi;
for (pi = g.groupPresentations.begin(); pi != g.groupPresentations.end(); pi++) 
 groupPresentations.insert( std::pair< GroupPresLocator, NGroupPresentation* >(pi->first, 
 clonePtr(pi->second) ) );
 // homGroupPresentations
std::map< HomGroupPresLocator, NHomGroupPresentation* >::const_iterator hpi;
for (hpi = g.homGroupPresentations.begin(); hpi != g.homGroupPresentations.end(); hpi++) 
 homGroupPresentations.insert( std::pair< HomGroupPresLocator, NHomGroupPresentation* >(hpi->first,
  clonePtr(hpi->second) ) );
 // alexanderChainComplexes
std::map< ChainComplexLocator, NMatrixRing< NSVPolynomialRing< NLargeInteger > >* >::const_iterator amci;
for (amci = g.alexanderChainComplexes.begin(); amci != g.alexanderChainComplexes.end(); amci++)
 alexanderChainComplexes.insert( std::pair< ChainComplexLocator, NMatrixRing< NSVPolynomialRing< NLargeInteger > >* >
  (amci->first, clonePtr(amci->second) ) );

// numStandardCells[5], numDualCells[5], numMixCells[5], numStandardBdryCells[4], 
//               numNonIdealCells[5], numIdealCells[4];
for (unsigned long i=0; i<5; i++) numStandardCells[i] = g.numStandardCells[i];
for (unsigned long i=0; i<5; i++) numDualCells[i] = g.numDualCells[i];
for (unsigned long i=0; i<5; i++) numMixCells[i] = g.numMixCells[i];
for (unsigned long i=0; i<4; i++) numStandardBdryCells[i] = g.numStandardBdryCells[i];
for (unsigned long i=0; i<5; i++) numNonIdealCells[i] = g.numNonIdealCells[i];
for (unsigned long i=0; i<4; i++) numIdealCells[i] = g.numIdealCells[i];
for (unsigned long i=0; i<4; i++) numNonIdealBdryCells[i] = g.numNonIdealBdryCells[i];
for (unsigned long i=0; i<5; i++) numRelativeCells[i] = g.numRelativeCells[i];
for (unsigned long i=0; i<5; i++) numDualRelCells[i] = g.numDualRelCells[i];
for (unsigned long i=0; i<5; i++) numMixRelCells[i] = g.numMixRelCells[i];
for (unsigned long i=0; i<4; i++) numMixBdryCells[i] = g.numMixBdryCells[i];
for (unsigned long i=0; i<4; i++) numDualBdryCells[i] = g.numDualBdryCells[i];

// chain maps
for (unsigned long i=0; i<sbiCM.size(); i++)    sbiCM[i] =  clonePtr(g.sbiCM[i]);
for (unsigned long i=0; i<strCM.size(); i++)    strCM[i] =  clonePtr(g.strCM[i]);
for (unsigned long i=0; i<schCM.size(); i++)    schCM[i] =  clonePtr(g.schCM[i]);
for (unsigned long i=0; i<dbiCM.size(); i++)    dbiCM[i] =  clonePtr(g.dbiCM[i]);
for (unsigned long i=0; i<dtrCM.size(); i++)    dtrCM[i] =  clonePtr(g.dtrCM[i]);
for (unsigned long i=0; i<dchCM.size(); i++)    dchCM[i] =  clonePtr(g.dchCM[i]);
for (unsigned long i=0; i<mbiCM.size(); i++)    mbiCM[i] =  clonePtr(g.mbiCM[i]);
for (unsigned long i=0; i<mtrCM.size(); i++)    mtrCM[i] =  clonePtr(g.mtrCM[i]);
for (unsigned long i=0; i<mchCM.size(); i++)    mchCM[i] =  clonePtr(g.mchCM[i]);
for (unsigned long i=0; i<smCM.size(); i++)     smCM[i]  =  clonePtr(g.smCM[i]);
for (unsigned long i=0; i<dmCM.size(); i++)     dmCM[i]  =  clonePtr(g.dmCM[i]);
for (unsigned long i=0; i<smbCM.size(); i++)    smbCM[i] =  clonePtr(g.smbCM[i]);
for (unsigned long i=0; i<dmbCM.size(); i++)    dmbCM[i] =  clonePtr(g.dmbCM[i]);
for (unsigned long i=0; i<srmCM.size(); i++)    srmCM[i] =  clonePtr(g.srmCM[i]);
for (unsigned long i=0; i<drmCM.size(); i++)    drmCM[i] =  clonePtr(g.drmCM[i]);

// maximal tree, extraNormalData
buildExtraNormalData(); buildMaximalTree();
}


// destructor
inline NCellularData::~NCellularData() {
 if (tri4) delete tri4; if (tri3) delete tri3; 
 // iterate through all the stored data and delete

 // master chain complex
 ccCollectionType::const_iterator ccmi;
 for (ccmi = genCC.begin(); ccmi != genCC.end(); ccmi++)
  delete ccmi->second;
 // master chain map collection
 cmCollectionType::const_iterator cmi;
 for (cmi = genCM.begin(); cmi != genCM.end(); cmi++)
  delete cmi->second;
 // integer chain complexes.
 std::map< ChainComplexLocator, NMatrixInt* >::const_iterator ci;
 for (ci = integerChainComplexes.begin(); ci != integerChainComplexes.end(); ci++) 
  delete ci->second;
 // integer chain maps
 std::map< ChainMapLocator, NMatrixInt* >::const_iterator mi;
 for (mi = integerChainMaps.begin(); mi != integerChainMaps.end(); mi++) 
  delete mi->second;
 // abelian groups
 std::map< GroupLocator, NAbelianGroup* >::iterator abi;
 for (abi = abelianGroups.begin(); abi != abelianGroups.end(); abi++)
	delete abi->second;
 // marked abelian groups
 std::map< GroupLocator, NMarkedAbelianGroup* >::iterator mabi;
 for (mabi = markedAbelianGroups.begin(); mabi != markedAbelianGroups.end(); mabi++)
	delete mabi->second;
 // hom marked abelian groups
 std::map< HomLocator, NHomMarkedAbelianGroup* >::iterator hmabi;
 for (hmabi = homMarkedAbelianGroups.begin(); hmabi != homMarkedAbelianGroups.end(); hmabi++)
	delete hmabi->second; 
 // bilinear forms
 std::map< FormLocator, NBilinearForm* >::iterator fi;
 for (fi = bilinearForms.begin(); fi != bilinearForms.end(); fi++)
	delete fi->second;
 // group presentations
 std::map< GroupPresLocator, NGroupPresentation* >::iterator gi;
 for (gi = groupPresentations.begin(); gi != groupPresentations.end(); gi++) 
        delete gi->second;
 // homomorphisms of group presentations
 std::map< HomGroupPresLocator, NHomGroupPresentation* >::iterator hi;
 for (hi = homGroupPresentations.begin(); hi != homGroupPresentations.end(); hi++)
        delete hi->second; 
 // alexanderChainComplexes
 std::map< ChainComplexLocator, NMatrixRing< NSVPolynomialRing< NLargeInteger > >* >::iterator amci;
 for (amci = alexanderChainComplexes.begin(); amci != alexanderChainComplexes.end(); amci++)
  delete amci->second;

 // iterate through sbiCM, strCM, schCM,  dbiCM, dtrCM, dchCM,  
 //                 mbiCM, mtrCM, mchCM,  smCM, dmCM, smbCM, 
 //                 dmbCM, srmCM, drmCM and deallocate
 for (unsigned long i=0; i<sbiCM.size(); i++) if (sbiCM[i]) delete sbiCM[i];
 for (unsigned long i=0; i<strCM.size(); i++) if (strCM[i]) delete strCM[i];
 for (unsigned long i=0; i<schCM.size(); i++) if (schCM[i]) delete schCM[i];
 for (unsigned long i=0; i<dbiCM.size(); i++) if (dbiCM[i]) delete dbiCM[i];
 for (unsigned long i=0; i<dtrCM.size(); i++) if (dtrCM[i]) delete dtrCM[i];
 for (unsigned long i=0; i<dchCM.size(); i++) if (dchCM[i]) delete dchCM[i];
 for (unsigned long i=0; i<mbiCM.size(); i++) if (mbiCM[i]) delete mbiCM[i];
 for (unsigned long i=0; i<mtrCM.size(); i++) if (mtrCM[i]) delete mtrCM[i];
 for (unsigned long i=0; i<mchCM.size(); i++) if (mchCM[i]) delete mchCM[i];
 for (unsigned long i=0; i<smCM.size(); i++)  if (smCM[i])  delete smCM[i];
 for (unsigned long i=0; i<dmCM.size(); i++)  if (dmCM[i])  delete dmCM[i];
 for (unsigned long i=0; i<smbCM.size(); i++) if (smbCM[i]) delete smbCM[i];
 for (unsigned long i=0; i<dmbCM.size(); i++) if (dmbCM[i]) delete dmbCM[i];
 for (unsigned long i=0; i<srmCM.size(); i++) if (srmCM[i]) delete srmCM[i];
 for (unsigned long i=0; i<drmCM.size(); i++) if (drmCM[i]) delete drmCM[i];
}


} // namespace regina

#endif

