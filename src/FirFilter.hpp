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

#include <cstddef>

/**
 * This template class implements a FIR filter (finite impulse response). 
 * The template parameter defines the type of input and output data.
 */

template <class T> class FirFilter {
public:
	/**
	 * Constructs the FIR filter
	 * \param N specifies the filter length.
	 */
	FirFilter(const size_t N);

	/**
	 * Destructs the FIR filter.
	 */
	~FirFilter(void);
	
	/**
	 * Initializes the filter coefficients
	 * The parameter is an array of filter coefficients in ascending order.
	 * The array has to have the same length as the filter.
	 */
	void setCoefficients(const T* const coeffs);

	/**
	 * Filter one sample. The parameter is the input value which is stored 
	 * in the buffer. The return value is the resulting value of convuluting
	 * the buffer content and the coefficients.
	 */
	T filterSample(const T sample);

	/**
	 * Dump the current buffer content. This is for debugging purposes.
	 */
	void getBuffer(T* const bufferCopy) const;
private:
	/**
	 * The filter length.
	 */
	size_t N;

	/**
	 * Pointer to the array of coefficients.
	 * The coefficients are stored in descending order.
	 */
	T* coeffs;

	/**
	 * Pointer to last element of coefficients array.
	 */
	T* coeffsLast;

        /**
	 * Pointer to buffer which is used circular.
	 */
	T* buffer;

	/**
	 * Pointer to last element of buffer array.
	 */
	T* bufferLast;

	/** 
	 * Pointer to oldest entry of buffer.
	 */
	T* bufferCurrent;
};

template <class T> FirFilter<T>::FirFilter(const size_t N) : N(N)
{
	coeffs=new T[N];
	coeffsLast=coeffs+(N-1);
	buffer=new T[N];
	bufferLast=buffer+(N-1);
	bufferCurrent=buffer;
	for(size_t i=0; i<N; i++) {
		coeffs[i]=0;
		buffer[i]=0;
	}
}

template <class T> FirFilter<T>::~FirFilter(void)
{
	delete[] coeffs;
	delete[] buffer;
}

template <class T> void FirFilter<T>::setCoefficients(const T* const coeffs)
{
	for(size_t i=0; i<N; i++) {
		this->coeffs[i]=coeffs[i];
	}
}

template <class T> inline T FirFilter<T>::filterSample(const T sample)
{
	T* c=coeffs;
	T result=0;

	// replace oldest value with current
	*bufferCurrent=sample;

	// convolution: sum c_i*x_{k-i}
	for(size_t i=0; i<N; i++) {
		result+=(*c)*(*bufferCurrent);
		if(++c>coeffsLast) {
			c=coeffs;
		}
		if(--bufferCurrent<buffer) {
			bufferCurrent=bufferLast;
		}
	}

	// point again to oldest value
	if(++bufferCurrent>bufferLast) {
		bufferCurrent=buffer;;
	}

	return result;
}

template <class T> void FirFilter<T>::getBuffer(T* const bufferCopy) const
{
	T* b=bufferCurrent;

	// newest value
	if(--b<buffer) {
		b=bufferLast;
	}
	
	for(size_t i=0; i<N; i++) {
		bufferCopy[i]=*b;
		if(--b<buffer) {
			b=bufferLast;
		}
	}
}

#endif
