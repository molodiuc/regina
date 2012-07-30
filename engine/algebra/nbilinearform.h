
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Computational Engine                                                  *
 *                                                                        *
 *  Copyright (c) 1999-2011, Ben Burton                                   *
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

/*! \file algebra/nbilinearform.h
 *  \brief Deals with bilinear forms -- homomorphisms out of a tensor product
 *   of abelian groups, to an abelian group. 
 */

#ifndef __NBILINEARFORM_H
#ifndef __DOXYGEN
#define __NBILINEARFORM_H
#endif

#include "regina-core.h"
#include "maths/nsparsegrid.h"
#include "maths/matrixops.h"
#include "shareableobject.h"
#include "maths/nlargeinteger.h"
#include "algebra/nmarkedabeliangroup.h"
#include "algebra/nsvpolynomialring.h"
#include "maths/nrational.h" 
#include "maths/nlargeinteger.h"
#include "maths/nprimes.h"
#include <algorithm>
#include <map>
#include <list>
#include <cmath>

namespace regina {

/**
 * \weakgroup algebra
 * @{
 */

/**
 * This class holds general bilinear forms -- homomorphisms from a tensor product of abelian
 * groups, to an abelian group. It allows for the computation of things such as the signature
 * and Kawauchi-Kojima invariants of the bilinear forms coming from Poincare duality on a 4 or
 * 3-manifold respectively. 
 *
 * @author Ryan Budney
 */
class REGINA_API NBilinearForm : public ShareableObject {
    private:
	/**
	 * Stores the pairing in SNF coordinates for all 3 groups. 
	 */
	NSparseGridRing< NLargeInteger > *reducedPairing; 
	/**
	 * The defining pairing.
	 */
    NSparseGridRing< NLargeInteger > *unreducedPairing;

	/**
	 * record of ldomain, rdomain and range
	 */
	NMarkedAbelianGroup lDomain, rDomain, Range;

    /**
     *  This is internal storage for the precomputed Kawauchi-Kojima tests 
     *  for torsion linking forms.  
     */

    bool KKinvariantsComputed;
    bool torsionLinkingFormIsSplit;
    bool torsionLinkingFormIsHyperbolic;
    bool torsionLinkingFormKKTTC;
    std::string torsionRankString;    
    std::string torsionSigmaString;  
    std::string torsionLegendreString;

    public:
        /**
         * Creates the bilinear form as a map ldomain \otimes rdomain --> range, where the map
         * is described by pairing, a nxmxk-sparse grid where n is the rank of
         * the chain complex for ldomain, m the rank of the chain complex for rdomain, and k the
         * rank of the chain complex for range. 
         */
        NBilinearForm(const NMarkedAbelianGroup &ldomain, const NMarkedAbelianGroup &rdomain, 
		      const NMarkedAbelianGroup &range,   const NSparseGridRing< NLargeInteger > &pairing);

        /**
         * Creates a permutation that is a clone of the given
         * bilinear form.
         *
         * @param cloneMe the form to clone.
         */
        NBilinearForm(const NBilinearForm& cloneMe);

    	/**
	     * Destructor
	     */
        ~NBilinearForm();

        /**
         * Evaluates the bilinear form on an element of ldomain and rdomain respectively, all in the chain
         * complex coordinates.
         *
         * @param lcc chain complex representative in ldomain
         * @param rcc chain complex representative in rdomain
         * @returns chain complex representative in range. 
         */
        std::vector<NLargeInteger> evalCC(std::vector<NLargeInteger> &lcc, 
                                          std::vector<NLargeInteger> &rcc) const;

    	/**
	     * Access to the unreducedPairing map.
    	 */
        const std::map< NMultiIndex< unsigned long >, NLargeInteger* > & unreducedMap() const;

        /**
         * Access to the reducedPairing map.  So if you wanted to iterate through the
         * values of the bilinear form evaluated on the generators of the homology of
         * ldomain and rdomain, you'd construct an iterator to a 
         *  std::map< NMultiIndex< unsigned long >, NLargeInteger* > type and iterate 
         *  through reducedMap(). 
         */
        const std::map< NMultiIndex< unsigned long >, NLargeInteger* > & reducedMap() const;

        /**
         * reducedPairing access as an NSparseGridRing
        */
        const NSparseGridRing< NLargeInteger >* reducedSparseGrid() const;
 
        /**
         *  Given a bilinear form whose range is the integers, we can ask for the 
         * maximal dimension (rank) of a subspace on which it is non-degenerate. 
         * This is that number.  Returns zero if your target space is not the integers.
         */
        unsigned long rank() const;

        /**
         * If this is a symmetric bilinear form with range the integers, we can ask for its 
         * signature.  This routine computes said signature. 
         */
        long int signature() const;

    	/**
          * Kawauchi-Kojima vector describing the torsion ranks, as prime powers.
          */
    	const std::string& kkTorRank() const;

         /**
          * Kawauchi-Kojima sigma vector.
          */
	    const std::string& kkTorSigma() const;

    	/**
          * Kawauchi-Kojima Legendre symbol vector
          */
	    const std::string& kkTorLegendre() const;

      	 /**
          * Kawauchi-Kojima hyperbolicity test. 
          */    	
        bool kkIsHyperbolic() const;

    	 /**
          * Kawauchi-Kojima split test.
          */
        bool kkIsSplit() const;

         /**
          * Kawauchi-Kojima 2-torsion test.
          */
        bool kkTwoTor() const;

	/**
	 * The subgroup of the range generated by the image.
	 */
	NMarkedAbelianGroup image() const;

	/**
	 * Determines whether or not the form is symmetric.
	 */
	bool isSymmetric() const;

	/**
	 * Determines whether or not the form is anti-symmetric.
	 */
	bool isAntiSymmetric() const;

	/**
	 *  Given a bilinear form A x B --> C and a map A' --> A, there
     *  is a natural composite A' x B --> C
	 */
	NBilinearForm lCompose(const NHomMarkedAbelianGroup &f) const; 
	/**
	 *  Given a bilinear form A x B --> C and a map B' --> B, there
     *  is a natural composite A x B' --> C
	 */
	NBilinearForm rCompose(const NHomMarkedAbelianGroup &f) const; 

	/**
	 *  Given a bilinear form A x B --> C and a map C --> C', there
     *  is a natural composite A x B --> C'
	 */
	NBilinearForm postCompose(const NHomMarkedAbelianGroup &f) const; 

	/**
	 * Given a map A x B --> C, there are two adjoints, 
	 *  this returns A --> Hom(B,C), the "left" adjoint. At present
     *  we use a cheap construction of this so the domain of this map
     *  will not satisfy isEqualTo(A)
	 */
	NHomMarkedAbelianGroup leftAdjoint() const;

	/**
	 * Given a map A x B --> C, there are two adjoints, 
     *  this returns B --> Hom(A,C), the "right" adjoint.   
	 */
	NHomMarkedAbelianGroup rightAdjoint() const;

	/**
	 * Form is a bilinear function A x B --> C, this returns A.
	 */
	const NMarkedAbelianGroup& ldomain() const;
	/**
	 * Form is a bilinear function A x B --> C, this returns B.
	 */
	const NMarkedAbelianGroup& rdomain() const;
	/**
	 * Form is a bilinear function A x B --> C, this returns C.
	 */
	const NMarkedAbelianGroup& range() const;

    /**
     * TODO: maybe talk about nullity, symmetry, l/rdomain, range, etc.
     * various invariants if they make sense, etc.
     */
    void writeTextShort(std::ostream& out) const;

    /**
     * TODO: maybe talk about nullity, symmetry, l/rdomain, range, etc.
     * various invariants if they make sense, etc.
     */
    void writeTextLong(std::ostream& out) const;
};


/**
 *  This procedure takes as input the NBilinearForm intP which is required to be a torsion linking 
 * form on an abelian group.  Specifically, it's assumed the map A x A --> Q/Z is symmetric, and 
 * the range is a trivially presented Z/nZ. 
 *
 * The procedure outputs the complete Kawauchi-Kojima invariants of the form, which are:
 *
 * 1) The prime power decomposition of A, as ppVec.
 *
 * 2) The classification of the 2-torsion linking pairing.  This invariant takes the form of a vector 
 *    with n elements, where 2^n is the order of the largest subgroup of A whose order is a power of 
 *    two.  The entries of the vector represent which the angle of a certain vector V in the complex 
 *    plane -- this vector can either be at the origin (8), or it is a non-zero vector along the line 
 *    of angle 2(pi)i/8*k, k=0,1,2,3,4,5,6,7.  Kawauchi and Kojima use the symbol infinity to represent
 *    zero-sums, but we use 8. The vector V is the sum of e^{ 2^{i+1} pi i form(x,x) } where x ranges 
 *    over ?? read KK paper again...  So this invariant is orientation-sensitive.  In particular if you 
 *    reverse the orientation, the elements of the vector negate mod 8 (except for 8, which remains 
 *    fixed).  So a vector (1 3 8 7 8) is turned to (7 5 8 1 8) on reversal of orientation. 
 *    TODO: supply more details
 *
 * 3) The classification of the odd p-torsion linking pairing.  Given a prime power p^k, you consider 
 *    the induced linking form on the quotient of elements of order p^k modulo p^{k-1}.  The determinant 
 *    you can consider as an element of the field Z_p, and then you take the legendre symbol (whether or 
 *    not the determinant is a square mod p).  This is the invariant, as a list for all p^k dividing the 
 *    order of A, with p odd.  This invariant is also orientation-sensitive, since when you change the 
 *    orientation you negate the relevant matrices, so the legendre symbol changes if and 
 *    only if the rank n of the p^k-torsion mod p^(k-1) is odd and p | n^2 + 1. 
 *
 * The routine also outputs linkingFormPD, which is the restriction of the torsion linking form to the 
 *   subgroups of A divisible by all the prime factors of the order of A. 
 */
REGINA_API void computeTorsionLinkingFormInvariants(const NBilinearForm &intP, 
	std::vector< std::pair< NLargeInteger, std::vector< unsigned long > > > &ppVec, 
    std::vector< std::pair< NLargeInteger, std::vector< unsigned long > > > &ppList,
    std::vector<unsigned long> &ttVec, 
    std::vector< std::pair< unsigned long, std::vector< int > > > &ptVec, 
    std::vector< NMatrixRing<NRational>* > &linkingFormPD );

/**
 * Function takes as input the output of computeTorsionLinkingFormInvariants, and returns various text
 * human-readable strings that interpret the result.
 */
REGINA_API void readTeaLeavesTLF(const std::vector< std::pair< NLargeInteger, 
                                                    std::vector< unsigned long > > > &ppVec,
        const std::vector< std::pair< NLargeInteger, std::vector< unsigned long > > > &ppList,
        const std::vector<unsigned long> &ttVec, 
        const std::vector< std::pair< unsigned long, std::vector< int > > > &ptVec, 
        const std::vector< NMatrixRing<NRational>* > &linkingFormPD, 
        bool orientable, 
        bool* torsionLinkingFormIsSplit, bool* torsionLinkingFormIsHyperbolic, 
        bool* torsionLinkingFormKKTTC, 
        std::string* torsionRankString,     std::string* torsionSigmaString,     
        std::string* torsionLegendreString );


/*@}*/

inline REGINA_API const NSparseGridRing< NLargeInteger >* NBilinearForm::reducedSparseGrid() const
{ return reducedPairing; }
inline REGINA_API const NMarkedAbelianGroup& NBilinearForm::ldomain() const
{ return lDomain; }
inline REGINA_API const NMarkedAbelianGroup& NBilinearForm::rdomain() const
{ return rDomain; }
inline REGINA_API const NMarkedAbelianGroup& NBilinearForm::range() const
{ return Range; }


} // namespace regina

#endif

