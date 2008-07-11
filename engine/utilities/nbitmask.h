
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

/*! \file nbitmask.h
 *  \brief Provides optimised bitmasks of arbitrary length.
 */

#ifndef __NBITMASK_H
#ifndef __DOXYGEN
#define __NBITMASK_H
#endif

#include <algorithm>
#include <iostream>

namespace regina {

/**
 * \weakgroup utilities
 * @{
 */

/**
 * A bitmask that can store arbitrarily many true-or-false bits.
 *
 * This bitmask packs the bits together, so that (unlike an array of bools)
 * many bits can be stored in a single byte.  As a result, operations on
 * this class are fast because the CPU can work on many bits simultaneously.
 *
 * Nevertheless, this class still has overhead because the bits must be
 * allocated on the heap, and because every operation requires looping
 * through the individual bytes.  For reasonably small bitmasks, see the
 * highly optimised NBitmask1 and NBitmask2 classes instead.
 *
 * Once a bitmask is created, its length (the number of bits) cannot be
 * changed.
 */
class NBitmask {
    private:
        unsigned pieces;
            /**< The number of machine-native pieces into which this bitmask
                 is split. */
        unsigned* mask;
            /**< The array of pieces, each of which stores 8 * sizeof(unsigned)
                 individual bits. */

    public:
        /**
         * Creates a new bitmask of the given length with all bits set to
         * \c false.
         *
         * @param length the number of bits stored in this bitmask; this must
         * be at least one.
         */
        NBitmask(unsigned length);

        /**
         * Creates a clone of the given bitmask.
         *
         * @param cloneMe the bitmask to clone.
         */
        NBitmask(const NBitmask& cloneMe);

        /**
         * Destroys this bitmask.
         */
        ~NBitmask();

        /**
         * Returns the value of the given bit of this bitmask.
         *
         * @param index indicates which bit to query; this must be at least
         * zero and strictly less than the length of this bitmask.
         * @return the value of the (\a index)th bit.
         */
        bool get(unsigned index) const;

        /**
         * Sets the given bit of this bitmask to the given value.
         *
         * @param index indicates which bit to set; this must be at least zero
         * and strictly less than the length of this bitmask.
         * @param value the value that will be assigned to the (\a index)th bit.
         */
        void set(unsigned index, bool value);

        /**
         * Sets this to the intersection of this and the given bitmask.
         * Every bit that is unset in \a other will be unset in this bitmask.
         *
         * \pre This and the given bitmask have the same length.
         *
         * @param other the bitmask to intersect with this.
         * @return a reference to this bitmask.
         */
        NBitmask& operator &= (const NBitmask& other);

        /**
         * Sets this to the union of this and the given bitmask.
         * Every bit that is set in \a other will be set in this bitmask.
         *
         * \pre This and the given bitmask have the same length.
         *
         * @param other the bitmask to union with this.
         * @return a reference to this bitmask.
         */
        NBitmask& operator |= (const NBitmask& other);

        /**
         * Determines whether this and the given bitmask are identical.
         *
         * \pre This and the given bitmask have the same length.
         *
         * @param other the bitmask to compare against this.
         * @return \c true if and only if this and the given bitmask are
         * identical.
         */
        bool operator == (const NBitmask& other) const;

        /**
         * Determines whether this bitmask is entirely contained within
         * the given bitmask.
         *
         * For this routine to return \c true, every bit that is set
         * in this bitmask must also be set in the given bitmask.
         *
         * \pre This and the given bitmask have the same length.
         *
         * @param other the bitmask to compare against this.
         * @return \c true if and only if this bitmask is entirely contained
         * within the given bitmask.
         */
        bool operator <= (const NBitmask& other) const;

        /**
         * Determines whether this bitmask is entirely contained within
         * the union of the two given bitmasks.
         *
         * For this routine to return \c true, every bit that is set
         * in this bitmask must also be set in either \a x or \a y.
         *
         * \pre Both \a x and \a y are the same length as this bitmask.
         *
         * @param x the first bitmask used to form the union.
         * @param y the first bitmask used to form the union.
         * @return \c true if and only if this bitmask is entirely contained
         * within the union of \a x and \a y.
         */
        bool inUnion(const NBitmask& x, const NBitmask& y) const;

        /**
         * Determines whether this bitmask contains the intersection of
         * the two given bitmasks.
         *
         * For this routine to return \c true, every bit that is set in
         * \e both \a x and \a y must be set in this bitmask also.
         *
         * \pre Both \a x and \a y are the same length as this bitmask.
         *
         * @param x the first bitmask used to form the intersection.
         * @param y the first bitmask used to form the intersection.
         * @return \c true if and only if this bitmask entirely contains
         * the intersection of \a x and \a y.
         */
        bool containsIntn(const NBitmask& x, const NBitmask& y) const;

    private:
        /**
         * Disable the assignment operator by making it private.
         */
        NBitmask& operator = (const NBitmask&);
};

/**
 * A small but extremely fast bitmask class that can store up to
 * 8 * sizeof(\a T) true-or-false bits.
 *
 * This bitmask packs all of the bits together into a single variable of
 * type \a T.  This means that operations on bitmasks are extremely
 * fast, because all of the bits can be processed at once.
 *
 * The downside of course is that the number of bits that can be stored
 * is limited to 8 * sizeof(\a T), where \a T must be a native unsigned
 * integer type (such as unsigned char, unsigned int, or unsigned long
 * long).
 *
 * For another extremely fast bitmask class that can store twice as
 * many bits, see NBitmask2.  For a bitmask class that can store
 * arbitrarily many bits, see NBitmask.
 *
 * \pre Type \a T is an unsigned integral numeric type.
 */
template <typename T>
class NBitmask1 {
    private:
        T mask;
            /**< Contains all 8 * sizeof(\a T) bits of this bitmask. */

    public:
        /**
         * Creates a new bitmask with all bits set to \c false.
         */
        inline NBitmask1() : mask(0) {
        }

        /**
         * Creates a new bitmask with all bits set to \c false.
         *
         * The integer argument is merely for compatibility with
         * the NBitmask constructor, and will be ignored.
         */
        inline NBitmask1(unsigned) : mask(0) {
        }

        /**
         * Creates a clone of the given bitmask.
         *
         * @param cloneMe the bitmask to clone.
         */
        inline NBitmask1(const NBitmask1<T>& cloneMe) : mask(cloneMe.mask) {
        }

        /**
         * Returns the value of the given bit of this bitmask.
         *
         * @param index indicates which bit to query; this must be between
         * 0 and (8 * sizeof(\a T) - 1) inclusive.
         * @return the value of the (\a index)th bit.
         */
        inline bool get(unsigned index) const {
            return (mask & (static_cast<T>(1) << index));
        }

        /**
         * Sets the given bit of this bitmask to the given value.
         *
         * @param index indicates which bit to set; this must be between
         * 0 and (8 * sizeof(\a T) - 1) inclusive.
         * @param value the value that will be assigned to the (\a index)th bit.
         */
        inline void set(unsigned index, bool value) {
            mask |= (static_cast<T>(1) << index);
            if (! value)
                mask ^= (static_cast<T>(1) << index);
        }

        /**
         * Sets this to the intersection of this and the given bitmask.
         * Every bit that is unset in \a other will be unset in this bitmask.
         *
         * @param other the bitmask to intersect with this.
         * @return a reference to this bitmask.
         */
        inline NBitmask1<T>& operator &= (const NBitmask1<T>& other) {
            mask &= other.mask;
            return *this;
        }

        /**
         * Sets this to the union of this and the given bitmask.
         * Every bit that is set in \a other will be set in this bitmask.
         *
         * @param other the bitmask to union with this.
         * @return a reference to this bitmask.
         */
        inline NBitmask1<T>& operator |= (const NBitmask1<T>& other) {
            mask |= other.mask;
            return *this;
        }

        /**
         * Determines whether this and the given bitmask are identical.
         *
         * @param other the bitmask to compare against this.
         * @return \c true if and only if this and the given bitmask are
         * identical.
         */
        inline bool operator == (const NBitmask1<T>& other) const {
            return (mask == other.mask);
        }

        /**
         * Determines whether this bitmask is entirely contained within
         * the given bitmask.
         *
         * For this routine to return \c true, every bit that is set
         * in this bitmask must also be set in the given bitmask.
         *
         * @param other the bitmask to compare against this.
         * @return \c true if and only if this bitmask is entirely contained
         * within the given bitmask.
         */
        inline bool operator <= (const NBitmask1<T>& other) const {
            return ((mask | other.mask) == other.mask);
        }

        /**
         * Determines whether this bitmask is entirely contained within
         * the union of the two given bitmasks.
         *
         * For this routine to return \c true, every bit that is set
         * in this bitmask must also be set in either \a x or \a y.
         *
         * @param x the first bitmask used to form the union.
         * @param y the first bitmask used to form the union.
         * @return \c true if and only if this bitmask is entirely contained
         * within the union of \a x and \a y.
         */
        inline bool inUnion(const NBitmask1<T>& x, const NBitmask1<T>& y)
                const {
            return ((mask & (x.mask | y.mask)) == mask);
        }

        /**
         * Determines whether this bitmask contains the intersection of
         * the two given bitmasks.
         *
         * For this routine to return \c true, every bit that is set in
         * \e both \a x and \a y must be set in this bitmask also.
         *
         * @param x the first bitmask used to form the intersection.
         * @param y the first bitmask used to form the intersection.
         * @return \c true if and only if this bitmask entirely contains
         * the intersection of \a x and \a y.
         */
        inline bool containsIntn(const NBitmask1<T>& x, const NBitmask1<T>& y)
                const {
            return ((mask | (x.mask & y.mask)) == mask);
        }

    private:
        /**
         * Disable the assignment operator by making it private.
         */
        NBitmask1<T>& operator = (const NBitmask1<T>&);
};

/**
 * A small but extremely fast bitmask class that can store up to
 * 8 * sizeof(\a T) + 8 * sizeof(\a U) true-or-false bits.
 *
 * This bitmask packs all of the bits together into a single variable of
 * type \a T and a single variable of type \a U.  This means that operations
 * on entire bitmasks are extremely fast, because all of the bits can be
 * processed in just two "native" operations.
 *
 * The downside of course is that the number of bits that can be stored
 * is limited to 8 * sizeof(\a T) + 8 * sizeof(\a U), where \a T and \a U
 * must be native unsigned integer types (such as unsigned char, unsigned int,
 * or unsigned long long).
 *
 * For an even faster bitmask class that can only store half as many bits,
 * see NBitmask1.  For a bitmask class that can store arbitrarily many bits,
 * see NBitmask.
 *
 * \pre Types \a T and \a U are unsigned integral numeric types.
 */
template <typename T, typename U = T>
class NBitmask2 {
    private:
        T low;
            /**< Contains the first 8 * sizeof(\a T) bits of this bitmask. */
        U high;
            /**< Contains the final 8 * sizeof(\a U) bits of this bitmask. */

    public:
        /**
         * Creates a new bitmask with all bits set to \c false.
         */
        inline NBitmask2() : low(0), high(0) {
        }

        /**
         * Creates a new bitmask with all bits set to \c false.
         *
         * The integer argument is merely for compatibility with
         * the NBitmask constructor, and will be ignored.
         */
        inline NBitmask2(unsigned) : low(0), high(0) {
        }

        /**
         * Creates a clone of the given bitmask.
         *
         * @param cloneMe the bitmask to clone.
         */
        inline NBitmask2(const NBitmask2<T>& cloneMe) :
                low(cloneMe.low), high(cloneMe.high) {
        }

        /**
         * Returns the value of the given bit of this bitmask.
         *
         * @param index indicates which bit to query; this must be between
         * 0 and (8 * sizeof(\a T) + 8 * sizeof(\a U) - 1) inclusive.
         * @return the value of the (\a index)th bit.
         */
        inline bool get(unsigned index) const {
            if (index < 8 * sizeof(T))
                return (low & (static_cast<T>(1) << index));
            else
                return (high & (static_cast<U>(1) << (index - 8 * sizeof(T))));
        }

        /**
         * Sets the given bit of this bitmask to the given value.
         *
         * @param index indicates which bit to set; this must be between
         * 0 and (8 * sizeof(\a T) + 8 * sizeof(\a U) - 1) inclusive.
         * @param value the value that will be assigned to the (\a index)th bit.
         */
        inline void set(unsigned index, bool value) {
            if (index < 8 * sizeof(T)) {
                low |= (static_cast<T>(1) << index);
                if (! value)
                    low ^= (static_cast<T>(1) << index);
            } else {
                high |= (static_cast<U>(1) << (index - 8 * sizeof(T)));
                if (! value)
                    high ^= (static_cast<U>(1) << (index - 8 * sizeof(T)));
            }
        }

        /**
         * Sets this to the intersection of this and the given bitmask.
         * Every bit that is unset in \a other will be unset in this bitmask.
         *
         * @param other the bitmask to intersect with this.
         * @return a reference to this bitmask.
         */
        inline NBitmask2<T, U>& operator &= (const NBitmask2<T, U>& other) {
            low &= other.low;
            high &= other.high;
            return *this;
        }

        /**
         * Sets this to the union of this and the given bitmask.
         * Every bit that is set in \a other will be set in this bitmask.
         *
         * @param other the bitmask to union with this.
         * @return a reference to this bitmask.
         */
        inline NBitmask2<T, U>& operator |= (const NBitmask2<T, U>& other) {
            low |= other.low;
            high |= other.high;
            return *this;
        }

        /**
         * Determines whether this and the given bitmask are identical.
         *
         * @param other the bitmask to compare against this.
         * @return \c true if and only if this and the given bitmask are
         * identical.
         */
        inline bool operator == (const NBitmask2<T, U>& other) const {
            return (low == other.low && high == other.high);
        }

        /**
         * Determines whether this bitmask is entirely contained within
         * the given bitmask.
         *
         * For this routine to return \c true, every bit that is set
         * in this bitmask must also be set in the given bitmask.
         *
         * @param other the bitmask to compare against this.
         * @return \c true if and only if this bitmask is entirely contained
         * within the given bitmask.
         */
        inline bool operator <= (const NBitmask2<T, U>& other) const {
            return ((low | other.low) == other.low &&
                (high | other.high) == other.high);
        }

        /**
         * Determines whether this bitmask is entirely contained within
         * the union of the two given bitmasks.
         *
         * For this routine to return \c true, every bit that is set
         * in this bitmask must also be set in either \a x or \a y.
         *
         * @param x the first bitmask used to form the union.
         * @param y the first bitmask used to form the union.
         * @return \c true if and only if this bitmask is entirely contained
         * within the union of \a x and \a y.
         */
        inline bool inUnion(const NBitmask2<T, U>& x, const NBitmask2<T, U>& y)
                const {
            return ((low & (x.low | y.low)) == low &&
                (high & (x.high | y.high)) == high);
        }

        /**
         * Determines whether this bitmask contains the intersection of
         * the two given bitmasks.
         *
         * For this routine to return \c true, every bit that is set in
         * \e both \a x and \a y must be set in this bitmask also.
         *
         * @param x the first bitmask used to form the intersection.
         * @param y the first bitmask used to form the intersection.
         * @return \c true if and only if this bitmask entirely contains
         * the intersection of \a x and \a y.
         */
        inline bool containsIntn(const NBitmask2<T, U>& x,
                const NBitmask2<T, U>& y) const {
            return ((low | (x.low & y.low)) == low &&
                (high | (x.high & y.high)) == high);
        }

    private:
        /**
         * Disable the assignment operator by making it private.
         */
        NBitmask2<T, U>& operator = (const NBitmask2<T, U>&);
};

/*@}*/

// Inline functions for NBitmask

inline NBitmask::NBitmask(unsigned length) :
        pieces((length - 1) / (8 * sizeof(unsigned)) + 1),
        mask(new unsigned[pieces]) {
    std::fill(mask, mask + pieces, 0);
}

inline NBitmask::NBitmask(const NBitmask& cloneMe) :
        pieces(cloneMe.pieces),
        mask(new unsigned[cloneMe.pieces]) {
    std::copy(cloneMe.mask, cloneMe.mask + pieces, mask);
}

inline NBitmask::~NBitmask() {
    delete[] mask;
}

inline bool NBitmask::get(unsigned index) const {
    return (mask[index / (8 * sizeof(unsigned))] &
        (static_cast<unsigned>(1) << (index % (8 * sizeof(unsigned)))));
}

inline void NBitmask::set(unsigned index, bool value) {
    mask[index / (8 * sizeof(unsigned))] |=
        (static_cast<unsigned>(1) << (index % (8 * sizeof(unsigned))));
    if (! value)
        mask[index / (8 * sizeof(unsigned))] ^=
            (static_cast<unsigned>(1) << (index % (8 * sizeof(unsigned))));
}

inline NBitmask& NBitmask::operator &= (const NBitmask& other) {
    for (unsigned i = 0; i < pieces; ++i)
        mask[i] &= other.mask[i];
    return *this;
}

inline NBitmask& NBitmask::operator |= (const NBitmask& other) {
    for (unsigned i = 0; i < pieces; ++i)
        mask[i] |= other.mask[i];
    return *this;
}

inline bool NBitmask::operator == (const NBitmask& other) const {
    return std::equal(mask, mask + pieces, other.mask);
}

inline bool NBitmask::operator <= (const NBitmask& other) const {
    for (unsigned i = 0; i < pieces; ++i)
        if ((mask[i] | other.mask[i]) != other.mask[i])
            return false;
    return true;
}

inline bool NBitmask::inUnion(const NBitmask& x, const NBitmask& y) const {
    for (unsigned i = 0; i < pieces; ++i)
        if ((mask[i] & (x.mask[i] | y.mask[i])) != mask[i])
            return false;
    return true;
}

inline bool NBitmask::containsIntn(const NBitmask& x, const NBitmask& y) const {
    for (unsigned i = 0; i < pieces; ++i)
        if ((mask[i] | (x.mask[i] & y.mask[i])) != mask[i])
            return false;
    return true;
}

} // namespace regina

#endif

