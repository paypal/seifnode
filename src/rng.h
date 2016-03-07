/** @file rng.h
 *  @brief Class header for native object wrapped in javascript object
 *		   responsible for generating random numbers (isaac) based on a given 
 *		   seed and saving/restoring the RNG state to the disk. The class also 
 *		   provides the ability to accept a secret key to encrypt the state 
 *		   saved to the disk 
 *
 *  @author Aashish Sheshadri
 *  @author Rohit Harchandani
 *	
 *	The MIT License (MIT)
 *	
 *	Copyright (c) 2015 PayPal
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

#ifndef RNG_H
#define RNG_H

#include <string>
#include <vector>

// ----------------------
// node.js addon includes
// ----------------------
#include <node.h>
#include <node_object_wrap.h>
#include <nan.h>

// ----------------
// library includes
// ----------------
#include <isaacRandomPool.h>


// ---
// RNG
// ---

/*
 * @class This class represents the C++ object exposing our implementation of
 *		  isaac random number generator, wrapped inside the javascript object.
 *
 * 		  The functions exposed to node.js are:
 *		  function isInitialized(key, filename, callback)
 *		  function initialize(key, filename)
 *		  function getBytes(n) -> returns node.js buffer with 'n' random bytes
 *		  function destroy() -> save RNG state to disk and destroy the object
 */
class RNG : public Nan::ObjectWrap {

	private:

		// javascript object constructor
		static Nan::Persistent<v8::Function> constructor;

		// isaac RNG object
		IsaacRandomPool prng;

		// ------
		// Worker
		// ------
		/*
		 * @class This class represents the node.js async worker responsible for 
		 *		  checking if the RNG has saved state on the disk and invoke the 
		 *		  given callback function with the status of the operation and 
		 */
		class Worker: public Nan::AsyncWorker {
		    
		    private:
		    	// ----
				// data
				// ----
				// pointer to isaac RNG object
				IsaacRandomPool* _prng;
				// file identifier of RNG state on disk
		        std::string _fileId; 
		        // key used to encrypt/decrypt RNG state on disk
		        std::vector<uint8_t> _digest; 
		        // status of operation of checking if RNG state is saved on disk
		        IsaacRandomPool::STATUS _result;

		    public:
		    	// -----------
				// Constructor
				// -----------
				/**
				 * Constructor
				 * @brief Initilizes and constructs internal data.
				 *
				 * @param initCallback callback to be invoked after async 
				 *		  operation 
				 * @param prng isaac RNG object pointer
				 * @param fileId file identifier of RNG state on disk
				 * @param digest key used to encrypt/decrypt RNG state on disk
				 */
		        Worker(Nan::Callback* initCallback, 
		        	IsaacRandomPool* prng,
		        	const std::string& fileId, 
            		const std::vector<uint8_t>& digest);


		        // ----------------
				// HandleOKCallback
				// ----------------
		        /**
		         * @brief Executed when the async work is complete without
		         * 		  error, this function will be run inside the main event  
		         * 		  loop, invoking the given callback with result of the 
		         *		  async operation of checking saved RNG state.
		         *
		         * The keys are returned as the second argument to the callback  
		         * {enc: [publicKey], dec: [privateKey]}
		         *
		         * @return void
		         */
		        void HandleOKCallback();


		        // -------------------
				// HandleErrorCallback
				// -------------------
		        /**
		         * @brief Executed when the async work is complete with
		         * 		  error, this function will be run inside the main event  
		         * 		  loop, invoking the given callback with the 
		         *		  corresponding error.
		         *
		         * The error is returned as the first argument to the callback 
		         * {code: [statusCode], message: [errorMessage]}
		         *
		         * @return void
		         */
		        void HandleErrorCallback();


		        // -------
				// Execute
				// -------
				/**
		         * @brief Executed in a separate thread, asynchronously 
		         *		  checking if RNG has saved state on the disk and 
		         *		  communicating the status of the operation via the 
		         *		  Worker class.
		         *
		         * @return void
		         */
		        void Execute();
		};


		// ---
		// New
		// ---
	    /**
		 * @brief Creates the node object and corresponding underlying object.
		 *
		 * Invoked as:
		 * 'var obj = new RNG()' or 
		 * 'var obj = RNG()'
		 *
		 * @param info node.js arguments wrapper
		 * @return void
		 */
		static NAN_METHOD(New);


		// -------------
		// isInitialized
		// -------------
		/**
		 * @brief Unwraps the arguments to get the key and the file name 
		 *        for saving the RNG state and creates an async worker to check 
		 *        if the RNG has been initialized by checking if the state file  
		 *        exists. Once the async work is complete the given callback is 
		 *        invoked with the result of the operation.
		 *
		 * Invoked as: 
		 * 'obj.isInitialized(key, filename, function(result){})' 
		 * 'key' is a buffer containing the disk encryption/decryption key
		 * 'filename' is the name of the RNG saved state file on disk
		 * 'result' is a js object containing the code('code') and 
		 * 	message('message')
		 *
		 * @param info node.js arguments wrapper containing file id, folder 
		 *        path for rng state and the callback function
		 *
		 * @return void
		 */
		static NAN_METHOD(isInitialized);


		// ----------
		// initialize
		// ----------
		/**
		 * @brief Unwraps the arguments to get the file id and the folder path 
		 *        for rng state and initilizes the RNG by gathering entropy.
		 *
		 * Invoked as:
		 * 'obj.initialize(key, filename)' where
		 * 'key' is a buffer containing the disk encryption/decryption key
		 * 'filename' is the name of the RNG saved state file on disk
		 * @param info node.js arguments wrapper containing key and filename 
		 *        for saving the rng state
		 * @return void
		 */
		static NAN_METHOD(initialize);


		// --------
		// getBytes
		// --------
		/**
		 * @brief Unwraps the arguments to get the number of random bytes 
		 *        required and returns a buffer with the random output.
		 *
		 * Invoked as:
		 * 'var buffer = obj.getBytes(numBytes)' where
		 * 'numBytes' is the number of required random bytes
		 * 'buffer' is a node.js buffer
		 *
		 * @param info node.js arguments wrapper containing number of random 
		 *        bytes required
		 *
		 * @return void
		 */
		static NAN_METHOD(getBytes);


		// -------
		// destroy
		// -------
		/**
		 * @brief Destroys the underlying RNG object thus saving the state 
		 *		  to disk.
		 *
		 * Invoked as:
		 * 'obj.destroy()' 
		 *
		 * @return void
		 */
		static NAN_METHOD(destroy);

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