/** @file seifsha3.h
 *  @brief Class header for native object wrapped in javascript object
 *		   responsible for performing Crypto++ SHA3 hash function
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

#ifndef SEIFSHA3_H
#define SEIFSHA3_H

// ----------------------
// node.js addon includes
// ----------------------
#include <node.h>
#include <node_object_wrap.h>
#include <nan.h>


// --------
// SEIFSHA3
// --------

/*
 * @class This class represents native object wrapped inside a javascript
 * 		  object, exposing Crypto++ SHA3 function.
 *
 *		  The functions exposed to node.js are:
 *		  function hash(data) -> returns SHA3-256 hash of the data
 */
class SEIFSHA3 : public Nan::ObjectWrap {

	private:

		// javascript object constructor
		static Nan::Persistent<v8::Function> constructor;

		// ---
		// New
		// ---
	    /**
		 * @brief Creates the node object and corresponding underlying object.
		 *
		 * Invoked as:
		 * 'let obj = new SEIFSHA3()' or
		 * 'let obj = SEIFSHA3()'
		 *
		 * @param info node.js arguments wrapper
		 *
		 * @return void
		 */
		static NAN_METHOD(New);


		// ----
		// hash
		// ----
		/**
		 * @brief Unwraps the arguments to get the string data and returns
		 *        the hash of the given input as a buffer object.
		 *
		 * Invoked as:
		 * 'let hash = obj.hash(stringData)' where
		 * 'stringData' is the string data to be hashed
		 * 'hash' is the output buffer containing the SHA3-256 hash
		 * To generate the output we are using Crypto++ SHA3-256 hash functions
		 *
		 * @param info node.js arguments wrapper containing string value to be
		 *        hashed
		 *
		 * @return void
		 */
		static NAN_METHOD(hash);

	public:

		// ----
		// Init
		// ----
		/**
		 * @brief Initialization function for node.js object wrapper exported
		 * 		  by the addon.
		 *
		 * @param exports node.js module exports
		 *
		 * @return void
		 */
    	static void Init(v8::Handle<v8::Object> exports);

};

#endif
