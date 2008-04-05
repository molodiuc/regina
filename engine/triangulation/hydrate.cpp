
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Computational Engine                                                  *
 *                                                                        *
 *  Copyright (c) 1999-2008, Ben Burton                                   *
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

#include <cctype>

#include "triangulation/ntriangulation.h"

/**
 * Determine the integer value represented by the given letter.
 */
#define VAL(x) ((x) - 'a')

/**
 * Determine the letter that represents the given integer value.
 */
#define LETTER(x) (char((x) + 'a'))

namespace regina {

bool NTriangulation::insertRehydration(const std::string& dehydration) {
    unsigned len = dehydration.length();

    // Ensure the string is non-empty.
    if (len == 0)
        return false;

    // Rewrite the string in lower case and verify that it contains only
    // letters.
    std::string proper(dehydration);
    for (std::string::iterator it = proper.begin(); it != proper.end(); it++) {
        if (*it >= 'A' && *it <= 'Z')
            *it = *it + ('a' - 'A');
        else if (*it < 'a' || *it > 'z')
            return false;
    }

    // Determine the number of tetrahedra.
    unsigned nTet = VAL(proper[0]);

    // Determine the expected length of each piece of the dehydrated string.
    unsigned lenNewTet = 2 * ((nTet + 3) / 4);
    unsigned lenGluings = nTet + 1;

    // Ensure the string has the expected length.
    if (len != 1 + lenNewTet + lenGluings + lenGluings)
        return false;

    // Determine which face gluings should involve new tetrahedra.
    bool* newTetGluings = new bool[2 * nTet];

    unsigned val;
    unsigned i, j;
    for (i = 0; i < lenNewTet; i++) {
        val = VAL(proper[i + 1]);
        if (val > 15) {
            delete[] newTetGluings;
            return false;
        }

        if (i % 2 == 0) {
            // This letter stores values 4i+4 -> 4i+7.
            for (j = 0; (j < 4) && (4*i + 4 + j < 2 * nTet); j++)
                newTetGluings[4*i + 4 + j] = ((val & (1 << j)) != 0);
        } else {
            // This letter stores values 4i-4 -> 4i-1.
            for (j = 0; (j < 4) && (4*i - 4 + j < 2 * nTet); j++)
                newTetGluings[4*i - 4 + j] = ((val & (1 << j)) != 0);
        }
    }

    // Create the tetrahedra and start gluing.
    NTetrahedron** tet = new NTetrahedron*[nTet];
    for (i = 0; i < nTet; i++)
        tet[i] = new NTetrahedron();

    unsigned currTet = 0;       // Tetrahedron of the next face to glue.
    int currFace = 0;           // Face number of the next face to glue.
    unsigned gluingsMade = 0;   // How many face pairs have we already glued?
    unsigned specsUsed = 0;     // How many gluing specs have we already used?
    unsigned tetsUsed = 0;      // How many tetrahedra have we already used?
    bool broken = false;        // Have we come across an inconsistency?
    unsigned adjTet;            // The tetrahedron to glue this to.
    unsigned permIndex;         // The index of the gluing permutation to use.
    NPerm adjPerm;              // The gluing permutation to use.
    NPerm identity;             // The identity permutation.
    NPerm reverse(3,2,1,0);     // The reverse permutation.

    while (currTet < nTet) {
        // Is this face already glued?
        if (tet[currTet]->getAdjacentTetrahedron(currFace)) {
            if (currFace < 3)
                currFace++;
            else {
                currFace = 0;
                currTet++;
            }
            continue;
        }

        // If this is a new tetrahedron, be aware of this fact.
        if (tetsUsed <= currTet)
            tetsUsed = currTet + 1;

        // Do we simply glue to a new tetrahedron?
        if (newTetGluings[gluingsMade]) {
            // Glue to tetrahedron tetsUsed.
            if (tetsUsed < nTet) {
                tet[currTet]->joinTo(currFace, tet[tetsUsed], identity);
                tetsUsed++;
            } else {
                broken = true;
                break;
            }
        } else {
            // Glue according to the next gluing spec.
            if (specsUsed >= lenGluings) {
                broken = true;
                break;
            }

            adjTet = VAL(proper[1 + lenNewTet + specsUsed]);
            permIndex = VAL(proper[1 + lenNewTet + lenGluings + specsUsed]);

            if (adjTet >= nTet || permIndex >= 24) {
                broken = true;
                break;
            }

            adjPerm = orderedPermsS4[permIndex] * reverse;

            if (tet[adjTet]->getAdjacentTetrahedron(adjPerm[currFace]) ||
                    (adjTet == currTet && adjPerm[currFace] == currFace)) {
                broken = true;
                break;
            }

            tet[currTet]->joinTo(currFace, tet[adjTet], adjPerm);

            specsUsed++;
        }

        // Increment everything for the next gluing.
        gluingsMade++;

        if (currFace < 3)
            currFace++;
        else {
            currFace = 0;
            currTet++;
        }
    }

    // Insert the tetrahedra into the triangulation and we're done!
    if (broken)
        for (i = 0; i < nTet; i++)
            delete tet[i];
    else {
        ChangeEventBlock block(this);
        for (i = 0; i < nTet; i++)
            addTetrahedron(tet[i]);
    }

    delete[] newTetGluings;
    delete[] tet;

    return (! broken);
}

std::string NTriangulation::dehydrate() const {
    // Can we even dehydrate at all?
    if (tetrahedra.size() > 25 || hasBoundaryFaces() || ! isConnected())
        return "";

    // Get the empty case out of the way, since it requires an
    // additional two redundant letters (two blocks of N+1 letters to
    // specify "non-obvious gluings").
    if (tetrahedra.empty())
        return "aaa";

    // Find an isomorphism that will put the triangulation in a form
    // sufficiently "canonical" to be described by a dehydration string.
    // When walking through faces from start to finish, this affects
    // only gluings to previously unseen tetrahedra:
    // (i) such gluings must be to the smallest numbered unused tetrahedron;
    // (ii) the gluing permutation must be the identity permutation.
    //
    // The array image[] maps tetrahedron numbers from this
    // triangulation to the canonical triangulation; preImage[] is the
    // inverse map.  The array vertexMap[] describes the corresponding
    // rearrangement of tetrahedron vertices and faces; specifically,
    // vertex i of tetrahedron t of this triangulation maps to vertex
    // vertexMap[t][i] of tetrahedron image[t].
    //
    // Each element of newTet[] is an 8-bit integer.  These bits
    // describe whether the gluings for some corresponding 8 faces
    // point to previously-seen or previously-unseen tetrahedra.
    // See the Callahan, Hildebrand and Weeks paper for details.
    unsigned nTets = tetrahedra.size();
    int* image = new int[nTets];
    int* preImage = new int[nTets];
    NPerm* vertexMap = new NPerm[nTets];

    unsigned char* newTet = new unsigned char[(nTets / 4) + 2];
    unsigned newTetPos = 0;
    unsigned newTetBit = 0;

    char* destChars = new char[nTets + 2];
    char* permChars = new char[nTets + 2];
    unsigned currGluingPos = 0;

    unsigned nextUnused = 1;
    unsigned tetIndex, tet, dest, faceIndex, face;
    NPerm map;
    unsigned mapIndex;

    for (tet = 0; tet < nTets; tet++)
        image[tet] = preImage[tet] = -1;

    image[0] = preImage[0] = 0;
    vertexMap[0] = NPerm();
    newTet[0] = 0;

    for (tetIndex = 0; tetIndex < nTets; tetIndex++) {
        // We must run through the tetrahedra in image order, not
        // preimage order.
        tet = preImage[tetIndex];

        for (faceIndex = 0; faceIndex < 4; faceIndex++) {
            // Likewise for faces.
            face = vertexMap[tet].preImageOf(faceIndex);

            // INVARIANTS (held while tet < nTets):
            // - nextUnused > tetIndex
            // - image[tet], preImage[image[tet]] and vertexMap[tet] are
            //   all filled in.
            // These invariants are preserved because the triangulation is
            // connected.  They break when tet == nTets.
            dest = tetrahedronIndex(
                tetrahedra[tet]->getAdjacentTetrahedron(face));

            // Is it a gluing we've already seen from the other side?
            if (image[dest] >= 0)
                if (image[dest] < image[tet] || (image[dest] == image[tet] &&
                        vertexMap[tet][tetrahedra[tet]->getAdjacentFace(face)]
                        < vertexMap[tet][face]))
                    continue;

            // Is it a completely new tetrahedron?
            if (image[dest] < 0) {
                // Previously unseen.
                image[dest] = nextUnused;
                preImage[nextUnused] = dest;
                vertexMap[dest] = vertexMap[tet] *
                    tetrahedra[tet]->getAdjacentTetrahedronGluing(face).
                    inverse();
                nextUnused++;

                newTet[newTetPos] |= (1 << newTetBit);
            } else {
                // It's a tetrahedron we've seen before.  Record the gluing.
                // Don't forget that our permutation abcd becomes dcba
                // in dehydration language.
                destChars[currGluingPos] = LETTER(image[dest]);
                map = vertexMap[dest] *
                    tetrahedra[tet]->getAdjacentTetrahedronGluing(face) *
                    vertexMap[tet].inverse() * NPerm(3, 2, 1, 0);
                // Just loop to find the index of the corresponding
                // gluing permutation.  There's only 24 permutations and
                // at most 25 tetrahedra; we'll live with it.
                for (mapIndex = 0; mapIndex < 24; mapIndex++)
                    if (map == orderedPermsS4[mapIndex])
                        break;
                permChars[currGluingPos] = LETTER(mapIndex);

                currGluingPos++;
            }

            newTetBit++;
            if (newTetBit == 8) {
                newTetPos++;
                newTetBit = 0;
                newTet[newTetPos] = 0;
            }
        }
    }

    // We have all we need.  Tidy up the strings and put them all
    // together.
    if (newTetBit > 0) {
        // We're partway through a bitset; assume it's finished and
        // point to the next unused slot in the array.
        newTetPos++;
        newTetBit = 0;
    }

    // At this stage we should have currGluingPos == nTets + 1.
    destChars[currGluingPos] = 0;
    permChars[currGluingPos] = 0;

    std::string ans;
    ans += LETTER(nTets);
    for (unsigned i = 0; i < newTetPos; i++) {
        ans += LETTER(newTet[i] >> 4);
        ans += LETTER(newTet[i] & 15);
    }
    ans += destChars;
    ans += permChars;

    // Done!
    delete[] permChars;
    delete[] destChars;
    delete[] vertexMap;
    delete[] preImage;
    delete[] image;

    return ans;
}

} // namespace regina

