/** @file addon.cc
 *  @brief Main file containing the module initialization function
 *
 *  @author Aashish Sheshadri
 *  @author Rohit Harchandani
 *
 *	The MIT License (MIT)
 *
 *	Copyright (c) 2015, 2016, 2017 PayPal
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

// ----------------------
// node.js addon includes
// ----------------------
#include <node.h>

// ----------------
// library includes
// ----------------
#include "seifecc.h"
#include "aesxor.h"
#include "rng.h"
#include "seifsha3.h"


// ----------
// Initialize
// ----------
/**
 * @brief Executed when the module is required. It initialized the different
 *		  native classes being wrapped by this addon.
 * 		  The below function and macro are equivalent to:
 *		  'module.exports = Initialize()'
 * @param target refers to the node.js module exports object
 * @return void
 */
void Initialize(v8::Handle<v8::Object> target) {
	SEIFECC::Init(target);
	AESXOR256::Init(target);
	RNG::Init(target);
	SEIFSHA3::Init(target);
}


NODE_MODULE(seifnode, Initialize)
