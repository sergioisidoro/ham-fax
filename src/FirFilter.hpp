// HamFax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2002
// Christof Schmitt, DH1CS <cschmitt@users.sourceforge.net>
//  
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef FIRFILTER_HPP
#define FIRFILTER_HPP

#include <valarray>

/**
 * This template class implements a FIR filter (finite impulse response). 
 * The template parameter defines the type of input and output data.
 */

template <class T> class FirFilter {
public:
	/**
	 * Convenient name for the coefficients.
	 */
	typedef valarray<T> coeff_t;

	/**
	 * Convenient name for the buffer.
	 */
	typedef valarray<T> buffer_t;

	/**
	 * Create the FirFilter.
	 * \param b is the initial value for the buffer and
	 * \param c are the coefficients.
	 * Both must be the same size!
	 */
	FirFilter(const buffer_t& b, const coeff_t& c);

	/**
	 * Create the FirFilter.
	 * \param n is the size of buffer and coefficient array
	 */
	FirFilter(size_t n);

	/**
	 * Set new coefficients and don't change the size!
	 */
	void setCoeffs(const coeff_t& c);

	/**
	 * Set new buffer content, e.g. all zero; don't change
	 * the buffer size!
	 */
	void setBuffer(const buffer_t& b);

        /**
	 * Pass one sample through the filter and get the result as return value.
	 */
	T filterSample(const T& sample);

	/**
	 * Get current buffer; useful for debugging purposes.
	 */
	buffer_t getBuffer(void) const;
private:
        coeff_t coeffs;
        buffer_t buffer;
        T* current;
};

template <class T>
inline FirFilter<T>::FirFilter(const buffer_t& b, const coeff_t& c) 
	: coeffs(c), buffer(b)
{
	current=&buffer[0];
}

template <class T> FirFilter<T>::FirFilter(size_t n)
	: coeffs(n), buffer(n)
{
	current=&buffer[0];
}

template <class T> void FirFilter<T>::setCoeffs(const coeff_t& c)
{
	coeffs=c;
}

template <class T> inline void FirFilter<T>::setBuffer(const buffer_t& b)
{
	buffer=b;
}

template <class T> inline T FirFilter<T>::filterSample(const T& sample)
{
        const T* c=&coeffs[0];
	T* const c_end=&coeffs[coeffs.size()];
	T* const b_begin=&buffer[0];
	T* const b_end=&buffer[buffer.size()];
        T sum=0;

        // replace oldest value with current
        *current=sample;

	// convolution
	while(current!=b_end)
		sum+=(*current++)*(*c++);
	current=b_begin;
	while(c!=c_end)
		sum+=(*current++)*(*c++);

        // point again to oldest value
        if(--current<b_begin)
                current=b_end-1;

        return sum;
}

template <class T>
inline FirFilter<T>::buffer_t FirFilter<T>::getBuffer(void) const
{
	return buffer;
}
#endif
