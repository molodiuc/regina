
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

#include "algebra/nmarkedabeliangroup.h"
#include "maths/matrixops.h"
#include <iostream>

namespace regina {

NMarkedAbelianGroup::NMarkedAbelianGroup(const unsigned long &rk, const NLargeInteger &p) : 
OM(rk, rk), ON(rk,rk), OMR(rk,rk), OMC(rk,rk), OMRi(rk, rk), OMCi(rk, rk), rankOM(0), 
ornR(0), ornC(0), ornRi(0), ornCi(0), otR(0), otC(0), otRi(0), otCi(0), 
InvFacList(0), snfrank(0), snffreeindex(0), ifNum(0), ifLoc(0), 
coeff(NLargeInteger::zero)
{
ornR.reset(new NMatrixInt(rk, rk)); ornRi.reset(new NMatrixInt(rk, rk)); 
ornC.reset(new NMatrixInt(rk, rk)); ornCi.reset(new NMatrixInt(rk, rk));
for (unsigned long i=0; i<rk; i++) ON.entry(i,i) = p;
// everything is already in SNF, so these are identity matrices
OMR.makeIdentity();OMC.makeIdentity(); OMRi.makeIdentity();OMCi.makeIdentity(); 
ornR->makeIdentity(); ornRi->makeIdentity(); ornC->makeIdentity(); ornCi->makeIdentity(); 
if (p != NLargeInteger::zero) ifNum=rk;
if (ifNum != 0) InvFacList.resize(ifNum);
for (unsigned long i=0; i<InvFacList.size(); i++) InvFacList[i] = p;
snfrank = rk - ifNum;
}

NMarkedAbelianGroup::NMarkedAbelianGroup(const NMatrixInt& M,
        const NMatrixInt& N) :
        OM(M), ON(N), OMR(M.columns(),M.columns()),
        OMC(M.rows(),M.rows()), OMRi(M.columns(),M.columns()),
        OMCi(M.rows(),M.rows()),
        rankOM(0),  ornR(0), ornC(0), ornRi(0), ornCi(0),
        InvFacList(0), snfrank(0), snffreeindex(0), ifNum(0), ifLoc(0), 
	coeff(NLargeInteger::zero), TORLoc(0), tensorIfLoc(0), TORVec(0) {
    NMatrixInt tM(M);
 
    #ifdef __useControlledSNF
    controlledSmithNormalForm(tM, OMR, OMRi, OMC, OMCi);
    #endif
    #ifndef __useControlledSNF
    smithNormalForm(tM, OMR, OMRi, OMC, OMCi);
    #endif

    for (unsigned long i=0; (i<tM.rows()) && (i<tM.columns()); i++)
	if (tM.entry(i,i) != NLargeInteger::zero) rankOM++;
    TORLoc = rankOM; // various routines need this to be true later -- only important
		     // to keep the mod-p calculations happy. 

    // construct the internal presentation matrix.
    std::auto_ptr<NMatrixRing<NLargeInteger> > prod=OMRi*ON;
    NMatrixInt ORN(N.rows()-rankOM, N.columns());
    ornR.reset( new NMatrixInt( ORN.columns(), ORN.columns() ) );
    ornRi.reset(new NMatrixInt( ORN.columns(), ORN.columns() ) );
    ornC.reset( new NMatrixInt( ORN.rows(), ORN.rows() ) );
    ornCi.reset(new NMatrixInt( ORN.rows(), ORN.rows() ) );

    for (unsigned long i=0;i<ORN.rows();i++) for (unsigned long j=0;j<ORN.columns();j++)
            ORN.entry(i,j) = prod->entry(i+rankOM,j);

    // put the presentation matrix in Smith normal form, and
    // build the list of invariant factors and their row indexes
    // now compute the rank and column indexes ...

    #ifdef __useControlledSNF
    controlledSmithNormalForm(ORN, *ornR, *ornRi, *ornC, *ornCi);
    #endif
    #ifndef __useControlledSNF
    smithNormalForm(ORN, *ornR, *ornRi, *ornC, *ornCi);
    #endif
    
    for (unsigned long i=0; ( (i<ORN.rows()) && (i<ORN.columns()) ); i++)
	{
        if (ORN.entry(i,i)==1) ifLoc++; else 
        if (ORN.entry(i,i)>1) InvFacList.push_back(ORN.entry(i,i)); 
	}

    ifNum = InvFacList.size();
    snfrank = ORN.rows() - ifLoc - ifNum;
    snffreeindex = ifLoc + InvFacList.size();

}

// We'll store H_k(M;Z_p) internally in two different ways.  The reason boils down to the pleasant version
// of the universal coefficient theorem that you see using Smith normal forms. 
// Summary:  if Z^a --N--> Z^b --M--> Z^c is a chain complex for H_k(M;Z) and p>0 an integer, put M into SNF(M), 
//  this gives SNF(M) == OMC*M*OMR. Let SNF(M) == diag[s_0, ..., s_{k-1}, 0, ... 0] and suppose
//  only entries s_i, ..., s_{k-1} share common factors with p.   Then you immediately get the 
//  presentation matrix for H_k(M;Z_p) which is a product of the two matrices:
//  [ trunc_top_k_rows[OMRi*N], diag(p,p,...,p) ] \times [ diag(gcd(s_i,p), ..., gcd(s_{k-1},p) ]
//  here trunc_top_k_rows means remove the first k rows from [OMRi*N].  The left matrix is basically
//  by design the presentation matrix for H_k(M;Z)\times Z_p, and the right matrix the presentation
//  matrix for TOR(H_{k-1}(M;Z), Z_p).   The 2nd matrix is already in SNF.  We apply SNF to the first, 
//  and store the change-of-basis matrices in otR, otC, otRi, otCi.  We then concatenate these two
//  diagonal matrices and apply SNF to them to get a situation NMarkedAbelianGroup will be happy with. 
//  This has the added advantage of us being able to later easily implement the NHomMarkedAbelianGroup
//  maps for UCT later when we're interested in that kind of stuff. 
NMarkedAbelianGroup::NMarkedAbelianGroup(const NMatrixInt& M, const NMatrixInt& N, const NLargeInteger &pcoeff):
        OM(M), ON(N), OMR(M.columns(),M.columns()),
        OMC(M.rows(),M.rows()), OMRi(M.columns(),M.columns()),
        OMCi(M.rows(),M.rows()),
        rankOM(0),  ornR(0), ornC(0), ornRi(0), ornCi(0), otR(0), otC(0), otRi(0), otCi(0),
        InvFacList(0), snfrank(0), snffreeindex(0), ifNum(0), ifLoc(0), coeff(pcoeff), 
	TORLoc(0), tensorIfLoc(0), TORVec(0)
{
    // find SNF(M).
    NMatrixInt tM(M);
    
    #ifdef __useControlledSNF
    controlledSmithNormalForm(tM, OMR, OMRi, OMC, OMCi);
    #endif
    #ifndef __useControlledSNF
    smithNormalForm(tM, OMR, OMRi, OMC, OMCi);
    #endif

    for (unsigned i=0; ( (i<tM.rows()) && (i<tM.columns()) ); i++)
	if (tM.entry(i,i) != 0) rankOM++;

    // in the case coeff > 0 we need to consider the TOR part of homology. 

    if (coeff > 0) for (unsigned i=0; i<rankOM; i++) 
	if (tM.entry(i,i).gcd(coeff) > 1) TORVec.push_back(tM.entry(i,i));
    TORLoc = rankOM - TORVec.size();

    // okay, lets get a presentation matrix for H_*(M;Z) \otimes Z_p
    // starting by computing the trunc[OMRi*N] matrix and padding with a diagonal p matrix

    std::auto_ptr<NMatrixRing<NLargeInteger> > OMRiN = OMRi*ON;

    // hmm, if we're using p == 0 coefficients, lets keep it simple
    if (coeff > 0)
     {
     NMatrixInt tensorPres( OMRiN->rows() - rankOM, OMRiN->columns() + OMRiN->rows() - rankOM );
     for (unsigned long i=0; i<tensorPres.rows(); i++) for (unsigned long j=0; j<OMRiN->columns(); j++)
	tensorPres.entry(i,j) = OMRiN->entry(i+rankOM, j);
     for (unsigned long i=0; i< OMRiN->rows() - rankOM; i++)
	tensorPres.entry(i, OMRiN->columns() + i) = coeff;

     // initialize coordinate-change matrices for the SNF computation. 
     otR.reset(new  NMatrixInt(tensorPres.columns(), tensorPres.columns() ));
     otRi.reset(new NMatrixInt(tensorPres.columns(), tensorPres.columns() ));
     otC.reset(new  NMatrixInt(tensorPres.rows(), tensorPres.rows() ));
     otCi.reset(new NMatrixInt(tensorPres.rows(), tensorPres.rows() ));

     #ifdef __useControlledSNF
     controlledSmithNormalForm(tensorPres, *otR, *otRi, *otC, *otCi);
     #endif
     #ifndef __useControlledSNF
     smithNormalForm(tensorPres, *otR, *otRi, *otC, *otCi);
     #endif

     // okay, so this group is a direct sum of groups of the form Z_q where q = gcd(p, TORVec[i]), 
     //  and groups Z_q where q is on the diagonal of tensorPres, and either q=0 or q>1. 
     //  unfortunately in rare occurances these are not the invariant factors of the group, so we
     //  assemble these numbers into a diagonal presentation matrix and apply SNF!  First, determine
     //  the size of the matrix we'll need. 

     std::vector<NLargeInteger> tensorInvFacs(0);
     for (unsigned long i=0; ( (i<tensorPres.rows()) && (i<tensorPres.columns()) ); i++)
	{
	if (tensorPres.entry(i,i) == 1) tensorIfLoc++; else
	if (tensorPres.entry(i,i) > 1) tensorInvFacs.push_back(tensorPres.entry(i,i)); else
	if (tensorPres.entry(i,i) == 0) snfrank++; // should always be zero. 
	}	

     NMatrixInt diagPres( TORVec.size() + tensorInvFacs.size() + snfrank, 
			  TORVec.size() + tensorInvFacs.size() + snfrank);
     for (unsigned long i=0; i<diagPres.rows(); i++)
	{
	if (i<TORVec.size()) diagPres.entry(i,i) = TORVec[i].gcd(coeff); else
	 diagPres.entry(i,i) = tensorPres.entry( i - TORVec.size() + tensorIfLoc, 
						 i - TORVec.size() + tensorIfLoc);
	}
  
     ornR.reset(new  NMatrixInt(diagPres.columns(), diagPres.columns() ));
     ornRi.reset(new NMatrixInt(diagPres.columns(), diagPres.columns() ));
     ornC.reset(new  NMatrixInt(diagPres.rows(), diagPres.rows() ));
     ornCi.reset(new NMatrixInt(diagPres.rows(), diagPres.rows() ));

     #ifdef __useControlledSNF
     controlledSmithNormalForm(diagPres, *ornR, *ornRi, *ornC, *ornCi);
     #endif
     #ifndef __useControlledSNF
     smithNormalForm(diagPres, *ornR, *ornRi, *ornC, *ornCi);
     #endif

     for (unsigned long i=0; i<diagPres.rows(); i++)
	{ // should only have terms > 1 or == 0. 
	if (diagPres.entry(i,i) > 1) InvFacList.push_back(diagPres.entry(i,i)); 
	}
     snffreeindex = InvFacList.size(); 
     ifNum = InvFacList.size(); 
     ifLoc = diagPres.rows() - ifNum; 
    }
   else
     { // coeff == p == 0 case
     NMatrixInt tensorPres( OMRiN->rows() - rankOM, OMRiN->columns() );
     for (unsigned long i=0; i<tensorPres.rows(); i++) for (unsigned long j=0; j<OMRiN->columns(); j++)
	tensorPres.entry(i,j) = OMRiN->entry(i+rankOM, j);

     // initialize coordinate-change matrices for the SNF computation. 
     ornR.reset(new  NMatrixInt(tensorPres.columns(), tensorPres.columns() ));
     ornRi.reset(new NMatrixInt(tensorPres.columns(), tensorPres.columns() ));
     ornC.reset(new  NMatrixInt(tensorPres.rows(), tensorPres.rows() ));
     ornCi.reset(new NMatrixInt(tensorPres.rows(), tensorPres.rows() ));

     #ifdef __useControlledSNF
     controlledSmithNormalForm(tensorPres, *ornR, *ornRi, *ornC, *ornCi);
     #endif
     #ifndef __useControlledSNF
     smithNormalForm(tensorPres, *ornR, *ornRi, *ornC, *ornCi);
     #endif

     for (unsigned long i=0; ( (i<tensorPres.rows()) && (i<tensorPres.columns()) ); i++)
	{
	if (tensorPres.entry(i,i) == 1) ifLoc++; else
	if (tensorPres.entry(i,i) > 1) InvFacList.push_back(tensorPres.entry(i,i)); 
	}
     snffreeindex = ifLoc + InvFacList.size();
     ifNum = InvFacList.size(); 
     snfrank = tensorPres.rows() - ifLoc - ifNum;
     }
}


unsigned long NMarkedAbelianGroup::getTorsionRank(const NLargeInteger& degree)
        const {
    unsigned long ans = 0;
    for (unsigned long i=0;i<InvFacList.size();i++) {
        if (InvFacList[i] % degree == 0)
            ans++;
    }
    return ans;
}


void NMarkedAbelianGroup::writeTextShort(std::ostream& out) const {
    bool writtenSomething = false;

    if (snfrank > 0) {
        if (snfrank > 1)
            out << snfrank << ' ';
        out << 'Z';
        writtenSomething = true;
    }

    std::vector<NLargeInteger>::const_iterator it = InvFacList.begin();
    NLargeInteger currDegree;
    unsigned currMult = 0;
    while(true) {
        if (it != InvFacList.end()) {
            if ((*it) == currDegree) {
                currMult++;
                it++;
                continue;
            }
        }
        if (currMult > 0) {
            if (writtenSomething)
                out << " + ";
            if (currMult > 1)
                out << currMult << ' ';
            out << "Z_" << currDegree.stringValue();
            writtenSomething = true;
        }
        if (it == InvFacList.end())
            break;
        currDegree = *it;
        currMult = 1;
        it++;
    }

    if (! writtenSomething)
        out << '0';
}


/*
 * The marked abelian group was defined by matrices M and N
 * with M*N==0.  Think of M as m by l and N as l by n.  Then
 * this routine returns the index-th free generator of the
 * ker(M)/img(N) in Z^l.
 */
std::vector<NLargeInteger> NMarkedAbelianGroup::getFreeRep(unsigned long index)
        const {
    static const std::vector<NLargeInteger> nullvec;
    if (index >= snfrank) return nullvec;

    std::vector<NLargeInteger> retval(OM.columns(),NLargeInteger::zero);
    // index corresponds to the (index+snffreeindex)-th column of ornCi
    // we then pad this vector (at the front) with rankOM 0's
    // and apply OMR to it.

     std::vector<NLargeInteger> temp(ornCi->rows()+rankOM,NLargeInteger::zero);
     for (unsigned long i=0;i<ornCi->rows();i++)
         temp[i+rankOM]=ornCi->entry(i,index+snffreeindex);
     // the above line takes the index+snffreeindex-th column of ornCi and
     // pads it.
     for (unsigned long i=0;i<retval.size();i++) for (unsigned long j=0;j<OMR.columns();j++)
            retval[i] += OMR.entry(i,j)*temp[j];
     // the above takes temp and multiplies it by the matrix OMR.
    return retval;
}

void printVec(const std::vector<NLargeInteger> &V)
{
std::cout<<"(";
for (unsigned long i=0; i<V.size(); i++)
 {
 if (i>0) std::cout<<" ";
 std::cout<<V[i];
 }
std::cout<<")";
}


/*
 * The marked abelian group was defined by matrices M and N
 * with M*N==0.  Think of M as m by l and N as l by n.  Then
 * this routine returns the index-th torsion generator of the
 * ker(M)/img(N) in Z^l.
 */
std::vector<NLargeInteger> NMarkedAbelianGroup::getTorsionRep(
        unsigned long index) const {
    static const std::vector<NLargeInteger> nullvec;
    if (index >= ifNum) return nullvec;
    std::vector<NLargeInteger> retval(OM.columns(),NLargeInteger::zero);

    if (coeff == 0)
     {
     std::vector<NLargeInteger> temp(ornCi->rows()+rankOM,  NLargeInteger::zero);
     for (unsigned long i=0;i<ornCi->rows();i++)
         temp[i+TORLoc] = ornCi->entry(i,ifLoc+index);
      // the above line takes the index+snffreeindex-th column of ornCi and
      // pads it suitably
     for (unsigned long i=0;i<retval.size();i++) for (unsigned long j=0;j<OMR.columns();j++) 
          retval[i] += OMR.entry(i,j)*temp[j];
     } 
    else
     { // coeff > 0 with coefficients there's the extra step of dealing with the UCT splitting
       // start with column column index + ifLoc of matrix ornCi, split into two vectors
       // 1) first TORVec.size() entries and 2) remaining entries. 
       std::vector<NLargeInteger> firstV(TORVec.size(), NLargeInteger::zero);
       std::vector<NLargeInteger> secondV(ornC->rows()-TORVec.size(), NLargeInteger::zero);
       for (unsigned long i=0; i<firstV.size(); i++)
	  firstV[i] = ornCi->entry( i, index + ifLoc );
       for (unsigned long i=0; i<secondV.size(); i++)
	  secondV[i] = ornCi->entry( i + firstV.size(), index + ifLoc );
       // 1st vec needs coords scaled appropriately by p/gcd(p,q) and multiplied by appropriate OMR columns
       for (unsigned long i=0; i<firstV.size(); i++)
	   firstV[i] *= coeff.divExact( TORVec[i].gcd(coeff) );
       std::vector<NLargeInteger> otCiSecondV(otCi->rows(), NLargeInteger::zero);
       for (unsigned long i=0; i<otCi->rows(); i++) for (unsigned long j=tensorIfLoc; j<otCi->columns(); j++)
	   otCiSecondV[i] += otCi->entry(i,j) * secondV[j-tensorIfLoc];
       // 2nd vec needs be multiplied by otCi, padded, then have OMR applied. 
       for (unsigned long i=0; i<retval.size(); i++) for (unsigned long j=0; j<firstV.size(); j++)
	  retval[i] += OMR.entry(i, TORLoc + j)*firstV[j];
       for (unsigned long i=0; i<retval.size(); i++) for (unsigned long j=0; j<otCiSecondV.size(); j++)
	  retval[i] += OMR.entry(i, rankOM+j) * otCiSecondV[j];
       // add answers together. 
     }
    return retval;
}


std::vector<NLargeInteger> NMarkedAbelianGroup::ccRep(const std::vector<NLargeInteger> SNFRep) const
{
    static const std::vector<NLargeInteger> nullV;
    if (SNFRep.size() != snfrank + ifNum) return nullV;

    std::vector<NLargeInteger> retval(OM.columns(),NLargeInteger::zero);
    std::vector<NLargeInteger> temp(ornCi->rows()+TORLoc,NLargeInteger::zero);

    if (coeff == 0)
     {
     for (unsigned long j=0; j<ifNum+snfrank; j++) for (unsigned long i=0; i<ornCi->rows(); i++)
         temp[i+TORLoc] += ornCi->entry(i,ifLoc+j) * SNFRep[j];
     for (unsigned long i=0;i<retval.size();i++) for (unsigned long j=0;j<OMR.columns();j++)
            retval[i] += OMR.entry(i,j)*temp[j];
     }
    else
     { // coeff > 0
       std::vector<NLargeInteger> firstV(TORVec.size(), NLargeInteger::zero);
       std::vector<NLargeInteger> secondV(ornC->rows()-TORVec.size(), NLargeInteger::zero);
       for (unsigned long i=0; i<firstV.size(); i++) for (unsigned long j=0; j<SNFRep.size(); j++)
	  firstV[i] += ornCi->entry( i, j + ifLoc ) * SNFRep[j];
       for (unsigned long i=0; i<secondV.size(); i++) for (unsigned long j=0; j<SNFRep.size(); j++)
	  secondV[i] += ornCi->entry( i + firstV.size(), j + ifLoc ) * SNFRep[j];
       // 1st vec needs coords scaled appropriately by p/gcd(p,q) and multiplied by appropriate OMR columns
       for (unsigned long i=0; i<firstV.size(); i++)
	   firstV[i] *= coeff.divExact( TORVec[i].gcd(coeff) );
       std::vector<NLargeInteger> otCiSecondV(otCi->rows(), NLargeInteger::zero);
       for (unsigned long i=0; i<otCi->rows(); i++) for (unsigned long j=tensorIfLoc; j<otCi->columns(); j++)
	   otCiSecondV[i] += otCi->entry(i,j) * secondV[j-tensorIfLoc];
       // 2nd vec needs be multiplied by otCi, padded, then have OMR applied. 
       for (unsigned long i=0; i<retval.size(); i++) for (unsigned long j=0; j<firstV.size(); j++)
	  retval[i] += OMR.entry(i, TORLoc + j)*firstV[j];
       for (unsigned long i=0; i<retval.size(); i++) for (unsigned long j=0; j<otCiSecondV.size(); j++)
	  retval[i] += OMR.entry(i, rankOM+j) * otCiSecondV[j];

     }
    return retval;
}

/*
 * The marked abelian group was defined by matrices M and N
 * with M*N==0.  Think of M as m by l and N as l by n.
 * When the group was initialized, it was computed to be isomorphic
 * to some Z_{d1} + ... + Z_{dk} + Z^d where d1 | d2 | ... | dk
 * this routine assumes element is in Z^l, and it returns a vector
 * of length d+k where the last d elements represent which class the
 * vector projects to in Z^d, and the first k elements represent the
 * projections to Z_{d1} + ... + Z_{dk}. Of these last elements, they
 * will be returned mod di respectively. Returns an empty vector if
 * element is not in the kernel of M. element is assumed to have
 * OM.columns()==ON.rows() entries.
 */
std::vector<NLargeInteger> NMarkedAbelianGroup::snfRep(
        const std::vector<NLargeInteger>& element)  const {
    std::vector<NLargeInteger> retval(snfrank+ifNum,
        NLargeInteger::zero);
    // apply OMRi, crop, then apply ornC, tidy up and return.
    static const std::vector<NLargeInteger> nullvec; // this is returned if
                                                     // element not in ker(M)
    // first, does element have the right dimensions? if not, abort.
    if (element.size() != OM.columns()) return nullvec;
    // this holds OMRi * element which we will use to check first if
    // element is in the kernel, and then to construct its SNF rep. 
    std::vector<NLargeInteger> temp(ON.rows(), NLargeInteger::zero); 
    for (unsigned long i=0;i<ON.rows();i++) for (unsigned long j=0;j<ON.rows();j++)
            temp[i] += OMRi.entry(i,j)*element[j];

    // make judgement on if in kernel.
    // needs to be tweaked for mod p coefficients.
    if (coeff == 0)
     { for (unsigned long i=0;i<rankOM;i++) if (temp[i] != 0) return nullvec; }
    else
     { // the first TORLoc-1 terms of tM were units mod p so we need only check divisibility by p for temp[i]
       // the remaining terms of tM were given by TORVec[i-TORLoc] and share a common factor with p==coeff. 
       // so for element to be in ker(M), we need temp[i]*TORVec[i-TORLoc] % p == 0 
       for (unsigned long i=0; i<rankOM; i++) 
	 { 
	  if (i<TORLoc) { if (temp[i] % coeff != 0) return nullvec; }
	  else { if ((temp[i]*TORVec[i-TORLoc]) % coeff != 0) return nullvec; 
                     temp[i] = temp[i].divExact(coeff.divExact(coeff.gcd(TORVec[i-TORLoc])));  }
	 }
     } //ok

    if (coeff == 0)
     {
     for (unsigned long i=0;i<snfrank;i++) for (unsigned long j=rankOM;j<ON.rows();j++)
                retval[i+ifNum] += ornC->entry(i+snffreeindex,j-rankOM)*temp[j];
     for (unsigned long i=0;i<ifNum;i++)   for (unsigned long j=rankOM;j<ON.rows();j++)
                retval[i] += ornC->entry(i+ifLoc,j-rankOM)*temp[j];
     }
    else
     {
     std::vector<NLargeInteger> diagPresV( ornC->rows(), NLargeInteger::zero);
     for (unsigned long i=0; i<diagPresV.size(); i++)
	{
	// TOR part
	if (i < TORVec.size()) diagPresV[i] = temp[ i + TORLoc ]; else
	// tensor part
	for (unsigned long j=0; j<otC->columns(); j++) 
		diagPresV[i] += otC->entry( i - TORVec.size() + tensorIfLoc, j) * temp[ j + rankOM ];
	}
     // assemble to a diagPres vector, apply ornC
     for (unsigned long i=0; i<retval.size(); i++) for (unsigned long j=0; j<diagPresV.size(); j++) 
	retval[i] += ornC->entry(i,j) * diagPresV[j];
     }
    // do modular reduction to make things look nice...
    for (unsigned long i=0; i<ifNum; i++)
	{
        retval[i] %= InvFacList[i];
        if (retval[i] < 0) retval[i] += InvFacList[i];
	}

    return retval;
}



// convert to the SNF(ORN) coordinates and check if boundary.  If so, find what it's the boundary of
// and convert to CC coords. 
std::vector<NLargeInteger> NMarkedAbelianGroup::writeAsBoundary(const std::vector<NLargeInteger> &input) const
{
    static const std::vector<NLargeInteger> nullvec;
    std::vector<NLargeInteger> retval(ON.columns(), NLargeInteger::zero);
    if ( input.size() != OM.columns() ) return nullvec;
    std::vector<NLargeInteger> temp(ON.rows(), NLargeInteger::zero); 
    for (unsigned long i=0;i<ON.rows();i++) for (unsigned long j=0;j<ON.rows();j++)
            temp[i] += OMRi.entry(i,j)*input[j]; //ok
    if (coeff == 0)
     { for (unsigned long i=0;i<rankOM;i++) if (temp[i] != 0) return nullvec; }
    else
     { 
       for (unsigned long i=0; i<TORLoc; i++) 
	{
 	 if (temp[i] % coeff != 0) return nullvec; 
	 temp[i]=NLargeInteger::zero; // get rid of TOR terms
	}
       for (unsigned long i=0; i<TORVec.size(); i++) if (temp[TORLoc+i]*TORVec[i] % coeff != 0) return nullvec;
     } // now we can be certain we're dealing with a cycle.
    // careful to disregard the TOR coordinates
    std::vector<NLargeInteger> snfcoords(ornC->rows(), NLargeInteger::zero);
    for (unsigned long i=0; i<ornC->rows(); i++) for (unsigned long j=TORVec.size(); j<ornC->columns(); j++)
        	snfcoords[i] += ornC->entry(i, j)*temp[j+TORLoc];

std::cout<<"temp[] == "; // ok
for (unsigned long i=0; i<temp.size(); i++) std::cout<<temp[i]<<" ";
std::cout<<"\n";
std::cout<<
std::cout<<"snfcoords[] == "; // ok
for (unsigned long i=0; i<snfcoords.size(); i++) std::cout<<snfcoords[i]<<" ";
std::cout<<"\n";
std::cout<<"InvFacList == [";
for (unsigned long i=0; i<InvFacList.size(); i++) std::cout<<InvFacList[i]<<" ";
std::cout<<"] ifLoc == "<<ifLoc<<"\n";

// only need to worry about coords ifLoc, ... <ifLoc+ifNum
     // test to see if a boundary. 
     for (unsigned long i=0; i<ifNum; i++)
	{ // needs to be modified for integer coeffs? 
         if (snfcoords[i+ifLoc] % InvFacList[i] != NLargeInteger::zero) return nullvec;
	 snfcoords[i+ifLoc]=snfcoords[i+ifLoc].divExact(InvFacList[i]);
        }

std::cout<<"mod snfcoords[] == "; // ok
for (unsigned long i=0; i<snfcoords.size(); i++) std::cout<<snfcoords[i]<<" ";
std::cout<<"\n";

     // now we apply ornR to snfcoords[] and we only care about the first ON.columns() entries.
     for (unsigned long i=0; i<retval.size(); i++)
	for (unsigned long j=0; j<snfcoords.size(); j++)
		retval[i] += ornR->entry(i,j+TORVec.size()) * snfcoords[j];

     // so if coeff==0 we need snfcoords[i] to be divisible by InvFacList[i]
     //    if coeff>0  we need snfcoords[i] to be a multiple of InvFacList[i] mod coeff.

return retval;
}



bool NMarkedAbelianGroup::isCycle(const std::vector<NLargeInteger> &input) const
{ 
bool retval = true;
if (input.size() == OM.columns())
 {
   for (unsigned long i=0; i<OM.rows(); i++)
	{
	NLargeInteger T(NLargeInteger::zero);
	for (unsigned long j=0; j<OM.columns(); j++) T += input[j]*OM.entry(i,j);
        if (coeff == NLargeInteger::zero)	
	  {if (T != NLargeInteger::zero) retval = false;} else
	  if ( (T % coeff) != NLargeInteger::zero ) retval = false;
	}
 } else retval = false;
return retval;
}

bool NMarkedAbelianGroup::isBoundary(const std::vector<NLargeInteger> &input) const
{ 
// isBoundary iff snfRep is the zero vector.
bool retval = true;
// check dimension
if (input.size() == OM.columns())
 { // check if in image of the matrix.
   std::vector<NLargeInteger> snF(snfRep(input));
   if (snF.size() != getNumberOfInvariantFactors() + getRank()) retval = false;
   else for (unsigned long i=0; i<snF.size(); i++) if (snF[i]!=NLargeInteger::zero) retval = false;
 } else retval = false;
return retval;
}

std::vector<NLargeInteger> NMarkedAbelianGroup::bdryMap(const std::vector<NLargeInteger> &CCrep) const
{
std::vector<NLargeInteger> retval(OM.rows(), NLargeInteger::zero);

if (CCrep.size() == OM.columns()) for (unsigned long i=0; i<OM.rows(); i++)
 {
 for (unsigned long j=0; j<OM.columns(); j++) retval[i] += CCrep[j]*OM.entry(i,j);
 if (coeff > NLargeInteger::zero) 
  { 
   retval[i] %= coeff;
   if (retval[i] < NLargeInteger::zero) retval[i] += coeff;
  }
 }
return retval;
}




// this is when you want to define an NHomMarkedAbelianGroup from a its reduced matrix, not via
// a chain map of chain complexes. 
// So matrix needs to be computed, and reducedMatrix properly assigned.
// at first, we make reducedMatrix unassigned and matrix zero with the proper dimensions.

NHomMarkedAbelianGroup::NHomMarkedAbelianGroup(const NMatrixInt &tobeRedMat, 
		       const NMarkedAbelianGroup &dom, 
		       const NMarkedAbelianGroup &ran) : 
domain(dom), range(ran), matrix(ran.getM().columns(), dom.getM().columns()), reducedMatrix(0), 
kernel(0), coKernel(0), image(0), reducedKernelLattice(0)
{
// need to compute matrix and assign reducedMatrix
reducedMatrix = new NMatrixInt(tobeRedMat);
// rarely would matrix be correct at this stage, so we proceed to fix that...
// roughly we're looking for matrix = ccRep ( tobeRedMat * SNFIsoRep ( projkerMdomain 

NMatrixInt domMRB( domain.getMRB() );
NMatrixInt domMRBi( domain.getMRBi() );

NMatrixInt domKerProj( domMRB.rows(), domMRB.columns() );
// define this matrix to be domMRB [ 0 0 | 0 I ] domMRBi where the first 0 is getrankM() square
for (unsigned long i=0; i<domMRB.rows(); i++) for (unsigned long j=0; j<domMRB.columns(); j++)
 for (unsigned long k=domain.getRankM(); k<domMRB.rows(); k++)
	domKerProj.entry(i,j) += domMRB.entry(i,k) * domMRBi.entry(k,j);

for (unsigned long j=0; j<domKerProj.columns(); j++)
 {
 // for each column of domKerProj write as a std::vector, push through SNFIsoRep
 std::vector<NLargeInteger> colj(domKerProj.rows());
 for (unsigned long i=0; i<domKerProj.rows(); i++) colj[i] = domKerProj.entry(i,j);
 std::vector<NLargeInteger> ircj(domain.snfRep(colj));

 // push through tobeRedMat
 std::vector<NLargeInteger> rmj(ircj.size(), NLargeInteger::zero);
 for (unsigned long i=0; i<tobeRedMat.rows(); i++) for (unsigned long k=0; k<tobeRedMat.columns(); k++) 
  rmj[i] += tobeRedMat.entry(i,k) * ircj[k];

 // apply ccRep
 std::vector<NLargeInteger> Mcolj(range.ccRep(rmj));

 // assemble into matrix.
 for (unsigned long i=0; i<matrix.rows(); i++) matrix.entry(i,j) = Mcolj[i];
 }

// done
}


NHomMarkedAbelianGroup::NHomMarkedAbelianGroup(const NHomMarkedAbelianGroup& g):
        ShareableObject(), domain(g.domain), range(g.range), matrix(g.matrix) {
    if (g.reducedMatrix) {
        reducedMatrix = new NMatrixInt(*g.reducedMatrix);
    } else reducedMatrix = 0;

    if (g.kernel) {
        kernel = new NMarkedAbelianGroup(*g.kernel);
    } else kernel = 0;

    if (g.coKernel) {
        coKernel = new NMarkedAbelianGroup(*g.coKernel);
    } else coKernel = 0;

    if (g.image) {
        image = new NMarkedAbelianGroup(*g.image);
    } else image = 0;

    if (g.reducedKernelLattice) {
        reducedKernelLattice = new NMatrixInt(*g.reducedKernelLattice);
    } else reducedKernelLattice = 0;
}

void NHomMarkedAbelianGroup::computeReducedMatrix() {
    if (!reducedMatrix) {
        unsigned long i,j,k;

        NMatrixInt kerMatrix( matrix.rows()-range.getRankM(),
                matrix.columns()-domain.getRankM() );
        // kerMatrix = truncate (range.getMRBi() * matrix * domain.getMRBi)
        // to construct this we do it in two steps:
        // step 1) temp1 = truncate columns (matrix * domain.getMRBi )
        // step 2) kerMatrix = truncate rows (range.getMRBi * temp1 )

        const NMatrixInt& dcckb(domain.getMRB());
        const NMatrixInt& rcckb(range.getMRBi());

        NMatrixInt temp1( matrix.rows(), matrix.columns()-domain.getRankM() );
        for (i=0;i<temp1.rows();i++)
            for (j=0;j<temp1.columns();j++)
                for (k=0;k<matrix.columns();k++)
                    temp1.entry(i,j) += matrix.entry(i,k) *
                        dcckb.entry(k,j + domain.getRankM() );

        for (i=0;i<kerMatrix.rows();i++)
            for (j=0;j<kerMatrix.columns();j++)
                for (k=0;k<rcckb.rows();k++)
                    kerMatrix.entry(i,j) +=
                        rcckb.entry(i+range.getRankM(), k) * temp1.entry(k,j);

        reducedMatrix = new NMatrixInt( kerMatrix.rows()-range.getTorsionLoc(),
                kerMatrix.columns()-domain.getTorsionLoc() );

        const NMatrixInt& dccqb(domain.getNCBi());
        const NMatrixInt& rccqb(range.getNCB());

        NMatrixInt temp2( kerMatrix.rows(),
            kerMatrix.columns() - domain.getTorsionLoc() );
        for (i=0;i<temp2.rows();i++)
            for (j=0;j<temp2.columns();j++)
                for (k=0;k<kerMatrix.columns();k++) {
                    temp2.entry(i,j) += kerMatrix.entry(i,k) *
                        dccqb.entry(k,j + domain.getTorsionLoc() );
                }

        for (i=0;i<reducedMatrix->rows();i++)
            for (j=0;j<reducedMatrix->columns();j++)
		{
                for (k=0;k<rccqb.rows();k++)
                    reducedMatrix->entry(i,j) +=
                        rccqb.entry(i+range.getTorsionLoc(), k) *
                        temp2.entry(k,j);
                // if representing torsion, mod out to reduce the size of the integers to something reasonable.
                if (i < range.getNumberOfInvariantFactors()) 
			{
			reducedMatrix->entry(i,j) %= range.getInvariantFactor(i);
			if (reducedMatrix->entry(i,j) < 0) reducedMatrix->entry(i,j) +=
				range.getInvariantFactor(i);
			}
		}
    }
}

void NHomMarkedAbelianGroup::computeReducedKernelLattice() {
    if (!reducedKernelLattice) {
        computeReducedMatrix();

        unsigned long i;

        const NMatrixInt& redMatrix(*reducedMatrix);

        // the kernel is the dcLpreimage lattice mod the domain lattice.
        // so after computing the dcLpreimage lattice, we need to represent
        // the domain lattice in its coordinates.

        std::vector<NLargeInteger> dcL(range.getRank() +
            range.getNumberOfInvariantFactors() );
        for (i=0; i<dcL.size(); i++)
            if (i<range.getNumberOfInvariantFactors())
                dcL[i]=range.getInvariantFactor(i);
            else
                dcL[i]="0";

        reducedKernelLattice = preImageOfLattice( redMatrix, dcL ).release();
    }
}

void NHomMarkedAbelianGroup::computeKernel() {
    if (!kernel) {
        computeReducedKernelLattice();
        NMatrixInt dcLpreimage( *reducedKernelLattice );

        unsigned long i,j,k;

        NMatrixInt R( dcLpreimage.columns(), dcLpreimage.columns() );
        NMatrixInt Ri( dcLpreimage.columns(), dcLpreimage.columns() );
        NMatrixInt C( dcLpreimage.rows(), dcLpreimage.rows() );
        NMatrixInt Ci( dcLpreimage.rows(), dcLpreimage.rows() );

        #ifdef __useControlledSNF
        controlledSmithNormalForm( dcLpreimage, R, Ri, C, Ci );
        #endif
        #ifndef __useControlledSNF
        smithNormalForm( dcLpreimage, R, Ri, C, Ci );
        #endif

        // the matrix representing the domain lattice in dcLpreimage
        // coordinates is given by domainLattice * R * (dcLpreimage inverse) * C

        NMatrixInt workMat( dcLpreimage.columns(),
            domain.getNumberOfInvariantFactors() );

        for (i=0;i<workMat.rows();i++)
            for (j=0;j<workMat.columns();j++)
                for (k=0;k<R.columns();k++) {
                    workMat.entry(i,j) += (domain.getInvariantFactor(j) *
                        R.entry(i,k) * C.entry(k,j) ) / dcLpreimage.entry(k,k);
                }

        NMatrixInt dummy( 1, dcLpreimage.columns() );

        kernel = new NMarkedAbelianGroup(dummy, workMat);
    }
}



void NHomMarkedAbelianGroup::computeCokernel() {
    if (!coKernel) {
        computeReducedMatrix();

        NMatrixInt ccrelators( reducedMatrix->rows(),
            reducedMatrix->columns() + range.getNumberOfInvariantFactors() );
        unsigned i,j;
        for (i=0;i<reducedMatrix->rows();i++)
            for (j=0;j<reducedMatrix->columns();j++)
                ccrelators.entry(i,j)=reducedMatrix->entry(i,j);
        for (i=0;i<range.getNumberOfInvariantFactors();i++)
            ccrelators.entry(i,i+reducedMatrix->columns())=
                range.getInvariantFactor(i);

        NMatrixInt ccgenerators( 1, reducedMatrix->rows() );

        coKernel = new NMarkedAbelianGroup(ccgenerators, ccrelators);
    }
}


void NHomMarkedAbelianGroup::computeImage() {
    if (!image) {
        computeReducedKernelLattice();
        const NMatrixInt& dcLpreimage( *reducedKernelLattice );

        unsigned long i,j;

        NMatrixInt imgCCm(1, dcLpreimage.rows() );
        NMatrixInt imgCCn(dcLpreimage.rows(),
            dcLpreimage.columns() + domain.getNumberOfInvariantFactors() );

        for (i=0;i<domain.getNumberOfInvariantFactors();i++)
            imgCCn.entry(i,i) = domain.getInvariantFactor(i);

        for (i=0;i<imgCCn.rows();i++)
            for (j=0;j< dcLpreimage.columns(); j++)
                imgCCn.entry(i,j+domain.getNumberOfInvariantFactors()) =
                    dcLpreimage.entry(i,j);

        image = new NMarkedAbelianGroup(imgCCm, imgCCn);
    }
}

NHomMarkedAbelianGroup NHomMarkedAbelianGroup::operator * (const NHomMarkedAbelianGroup &X) const
{
std::auto_ptr<NMatrixRing<NLargeInteger> > prod=matrix*X.matrix;

NMatrixInt compMat(matrix.rows(), X.matrix.columns() );

for (unsigned long i=0;i<prod->rows();i++) for (unsigned long j=0;j<prod->columns();j++)
            compMat.entry(i,j) = prod->entry(i, j);

return NHomMarkedAbelianGroup(X.domain, range, compMat);
}

std::vector<NLargeInteger> NHomMarkedAbelianGroup::evalCC(const std::vector<NLargeInteger> &input) const
{
static const std::vector<NLargeInteger> nullV;
std::vector<NLargeInteger> retval;

if (domain.isCycle(input))
 {
  retval.resize(matrix.rows(), NLargeInteger::zero);
  for (unsigned long i=0; i<retval.size(); i++) for (unsigned long j=0; j<matrix.columns(); j++)
	retval[i] += input[j]*matrix.entry(i,j);
 } else retval = nullV;
return retval;
}

std::vector<NLargeInteger> NHomMarkedAbelianGroup::evalSNF(const std::vector<NLargeInteger> &input) const
{
const_cast<NHomMarkedAbelianGroup*>(this)->computeReducedMatrix();
static const std::vector<NLargeInteger> nullV; 
std::vector<NLargeInteger> retval;

if (input.size() == (domain.getRank() + domain.getNumberOfInvariantFactors()) )
 {
   retval.resize(range.getRank()+range.getNumberOfInvariantFactors(), NLargeInteger::zero);
   for (unsigned long i=0; i<retval.size(); i++) for (unsigned long j=0; j<getReducedMatrix().columns(); j++)
	retval[i] += input[j] * getReducedMatrix().entry(i,j);
 } else retval = nullV; 

return retval;
}


void NHomMarkedAbelianGroup::writeReducedMatrix(std::ostream& out) const {
    // Cast away const to compute the reduced matrix -- the only reason we're
    // changing data members now is because we delayed calculations
    // until they were really required.
    const_cast<NHomMarkedAbelianGroup*>(this)->computeReducedMatrix();

    unsigned long i,j;
    out<<"Reduced Matrix is "<<reducedMatrix->rows()<<" by "
        <<reducedMatrix->columns()<<" corresponding to domain ";
    domain.writeTextShort(out);
    out<<" and range ";
    range.writeTextShort(out);
    out<<"\n";
    for (i=0;i<reducedMatrix->rows();i++) {
        out<<"[";
        for (j=0;j<reducedMatrix->columns();j++) {
            out<<reducedMatrix->entry(i,j);
            if (j+1 < reducedMatrix->columns()) out<<" ";
        }
        out<<"]\n";
    }
}

void NHomMarkedAbelianGroup::writeTextShort(std::ostream& out) const {
    if (isIso())
        out<<"isomorphism";
    else if (isZero()) // zero map
        out<<"zero map";
    else if (isMonic()) { // monic not epic
        out<<"monic, with cokernel ";
        getCokernel().writeTextShort(out);
    } else if (isEpic()) { // epic not monic
        out<<"epic, with kernel ";
        getKernel().writeTextShort(out);
    } else { // nontrivial not epic, not monic
        out<<"kernel ";
        getKernel().writeTextShort(out);
        out<<" | cokernel ";
        getCokernel().writeTextShort(out);
        out<<" | image ";
        getImage().writeTextShort(out);
    }
}


bool NHomMarkedAbelianGroup::isIdentity() const
{
bool retval(true);
if (domain == range)
 {
     const_cast<NHomMarkedAbelianGroup*>(this)->computeReducedMatrix();
     if (!reducedMatrix->isIdentity()) retval = false;
 } else retval = false;
return retval;
}

bool NHomMarkedAbelianGroup::isCycleMap() const
{
// run through a basis for ker OM, plug in and check isCycle()
bool retval(true);
// the last getRankCC() - getRankM() columns of getMRB() are a basis for the kernel of the defining matrix M
for (unsigned long i=domain.getRankM(); i<domain.getRankCC(); i++)
 {
 std::vector<NLargeInteger> domChain(domain.getRankCC(), NLargeInteger::zero);
 for (unsigned long j=0; j<domain.getRankCC(); j++) domChain[j] = domain.getMRB().entry(j,i);
 if (!range.isCycle(evalCC(domChain))) retval = false;
 }

return retval;
}


/**
 * Given two NHomMarkedAbelianGroups, you have two diagrams:
 *	Z^a --N1--> Z^b --M1--> Z^c   Z^g --N3--> Z^h --M3--> Z^i
 *                   ^                             ^
 *                   |  this                       | other
 *      Z^d --N2--> Z^e --M2--> Z^f   Z^j --N4--> Z^k --M4--> Z^l
 * @return true if and only if M1 == N3, M2 == N4 and diagram commutes
 *         commutes.
 */
bool NHomMarkedAbelianGroup::isChainMap(const NHomMarkedAbelianGroup &other) const
{
bool retval(true);
// M1 is getRange().getM()   N3 is other.getRange().getN()   so  b==g, c==h, M1 == N3
// M2 is getDomain().getM()  N4 is other.getDomain().getN()  and e==j, f==k, M2 == N4

if ( (getRange().getM().rows() != other.getRange().getN().rows()) ||
     (getRange().getM().columns() != other.getRange().getN().columns()) ||
     (getDomain().getM().rows() != other.getDomain().getN().rows()) ||
     (getDomain().getM().columns() != other.getDomain().getN().columns())
   ) retval = false;
else if ( (getRange().getM() != other.getRange().getN()) ||
     (getDomain().getM() != other.getDomain().getN()) ) retval = false;
else
{
      std::auto_ptr< NMatrixRing<NLargeInteger> > prodLU = range.getM() * getDefiningMatrix();
      std::auto_ptr< NMatrixRing<NLargeInteger> > prodBR = other.getDefiningMatrix() * domain.getM();
     if ( (*prodLU) != (*prodBR) ) retval = false;
     }
return retval;
}



// Start with the reduced matrix which is a 2x2 block matrix:
//
//  [A|B]
//  [---]  where D is an inveritble square matrix, 0 is a zero matrix,  
//  [0|D]  A a square matrix and B maybe not square. 
//         the columns of D represent the free factors of the domain, 
// 	   the rows of D the free factors of the range, 
//         the columns/rows of A represent the torsion factors of the domain/range
// 	   respectively.  So the inverse matrix must have the form
//  [A'|B']
//  [-----]
//  [0 |D']  where D' is the inverse of D, A' represents the inverse automorphism of
//           Z_p1 + Z_p2 + ... Z_pk, etc.  And so B' must equal -A'BD', which is readily
//           computable.  So it all boils down to computing A'.  So we need a routine which
//           takes a matrix A representing an automorphism of Z_p1 + ... Z_pk and then computes
//           the matrix representing the inverse automorphism.  
//           So to do this we'll need a new matrixops.cpp command -- call it torsionAutInverse.  
NHomMarkedAbelianGroup NHomMarkedAbelianGroup::inverseHom() const
{
const_cast<NHomMarkedAbelianGroup*>(this)->computeReducedMatrix();
NMatrixInt invMat( reducedMatrix->columns(), reducedMatrix->rows() );

if (isIso()) 
 {
  // get A, B, D from reducedMatrix
  // A must be square with domain/range.getNumberOfInvariantFactors() columns
  // D must be square with domain/range.getRank() columns
  // B may not be square with domain.getRank() columns and range.getNumberOfInvariantFactors() rows.
  NMatrixInt A(range.getNumberOfInvariantFactors(), domain.getNumberOfInvariantFactors());
  NMatrixInt B(range.getNumberOfInvariantFactors(), domain.getRank());
  NMatrixInt D(range.getRank(), domain.getRank());
  for (unsigned long i=0; i<A.rows(); i++) for (unsigned long j=0; j<A.columns(); j++)
   A.entry(i,j) = reducedMatrix->entry(i,j);
  for (unsigned long i=0; i<B.rows(); i++) for (unsigned long j=0; j<B.columns(); j++)
   B.entry(i,j) = reducedMatrix->entry(i, j + A.columns());
  for (unsigned long i=0; i<D.rows(); i++) for (unsigned long j=0; j<D.columns(); j++)
   D.entry(i,j) = reducedMatrix->entry( i + A.rows(), j + A.columns() );
  // compute A', B', D'
  // let's use void columnEchelonForm(NMatrixInt &M, NMatrixInt &R, NMatrixInt &Ri,
  //      const std::vector<unsigned> &rowList); from matrixOps to compute the inverse of D.
  NMatrixInt Di(D.rows(), D.columns()); Di.makeIdentity();
  NMatrixInt Dold(D.rows(), D.columns()); Dold.makeIdentity();
  std::vector<unsigned> rowList(D.rows());
  for (unsigned i=0; i<rowList.size(); i++) rowList[i]=i;
  columnEchelonForm(D, Di, Dold, rowList); // now Di is the inverse of the old D, and D is the identity, 
      // and Dold is the old D.
  //NMatrixInt torsionAutInverse(const NMatrixInt& input, const std::vector<NLargeInteger> &invF);
  // can be used to compute A'.  So we need to make a vector containing the invariant factors. 
  std::vector<NLargeInteger> invF(domain.getNumberOfInvariantFactors());
  for (unsigned long i=0; i<invF.size(); i++) invF[i] = domain.getInvariantFactor(i);
  NMatrixInt Ai(torsionAutInverse( A, invF));
  // then Bi is given by Bi = -AiBDi
  NMatrixInt Bi(range.getNumberOfInvariantFactors(), domain.getRank());
  NMatrixInt Btemp(range.getNumberOfInvariantFactors(), domain.getRank());
   // Btemp will give -BDi
   // Bi will be AiBtemp
   // Strangely, we don't seem to have a convienient matrix multiplication available so we just do it
   // `by hand'.  Perhaps this is what Ben's multiplyAs routine is about?  But it isn't used anywhere
   // and the docs are pretty vague.  Ask Ben to clarify.
  for (unsigned long i=0; i<Btemp.rows(); i++) for (unsigned long j=0; j<Btemp.columns(); j++)
   for (unsigned long k=0; k<Btemp.columns(); k++)
    Btemp.entry(i,j) -= B.entry(i,k)*Di.entry(k,j);
  for (unsigned long i=0; i<Bi.rows(); i++) for (unsigned long j=0; j<Bi.columns(); j++)
   for (unsigned long k=0; k<Ai.columns(); k++)
    Bi.entry(i,j) += Ai.entry(i,k)*Btemp.entry(k,j);

  // compute the coefficients of invMat.  We're in the funny situation where we know what invMat's
  // reduced matrix, not its chain-complex level defining matrix.  So we use the alternative
  // NHomMarkedAbelianGroup constructor designed specifically for this situation.  
  // assemble into invMat  [A'|B']
  //                       [-----]
  //                       [0 |D'] 

  for (unsigned long i=0; i<Ai.rows(); i++) for (unsigned long j=0; j<Ai.columns(); j++)
	invMat.entry(i,j) = Ai.entry(i,j);
  for (unsigned long i=0; i<Di.rows(); i++) for (unsigned long j=0; j<Di.columns(); j++)
	invMat.entry(i+Ai.rows(),j+Ai.columns()) = Di.entry(i,j);
  for (unsigned long i=0; i<Bi.rows(); i++) for (unsigned long j=0; j<Bi.columns(); j++)
	invMat.entry(i,j+Ai.columns()) = Bi.entry(i,j);
  // for debugging purposes check that reducedMat of inverseHom is the A' B' 0 D' matrix
 }

return NHomMarkedAbelianGroup( invMat, range, domain );
}



} // namespace regina

