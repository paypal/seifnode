/** @file util.h
 *  @brief header/implementation file for utility functions used by the node
 *		   module
 *
 *  @author Aashish Sheshadri
 *  @author Rohit Harchandani
 *
 *	The MIT License (MIT)
 *
 *  Copyright (c) 2015, 2016, 2017 PayPal
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to
 *	deal in the Software without restriction, including without limitation the
 *	rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *	sell copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in
 *	all copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *	DEALINGS IN THE SOFTWARE.
 */

#ifndef SEIFNODE_UTIL_H
#define SEIFNODE_UTIL_H

// -----------------
// standard includes
// -----------------
#include <vector>
#include <string>

// -----------------
// cryptopp includes
// -----------------
#include "sha3.h"
using CryptoPP::SHA3_256;


// ----------
// hashString
// ----------
/**
 * @brief creates a SHA3-256 hash of the given string using CryptoPP
 * @param digest output vector in which the hash to be stored
 * @param str input string to be hashed
 * PreCondition: 'digest' should be of appropriate size
 * (CryptoPP::SHA3_256::DIGESTSIZE)
 * @return void
 */
static void hashString(std::vector<uint8_t>& digest, const std::string& str) {
    CryptoPP::SHA3_256 hash;
    hash.Update(reinterpret_cast<const uint8_t*>(str.c_str()), str.size());
    hash.Final(digest.data());
}


// ----------
// hashBuffer
// ----------
/**
 * @brief creates a SHA3-256 hash of the given buffer using CryptoPP
 * @param digest output vector in which the hash to be stored
 * @param input input buffer to be hashed
 * @param inputLen length of input buffer to be hashed
 * PreCondition: 'digest' should be of appropriate size
 * (CryptoPP::SHA3_256::DIGESTSIZE)
 * @return void
 */
static void hashBuffer(
	std::vector<uint8_t>& digest,
	const uint8_t* input,
	int inputLen
) {
    CryptoPP::SHA3_256 hash;
    hash.Update(input, inputLen);
    hash.Final(digest.data());
}


#endif
