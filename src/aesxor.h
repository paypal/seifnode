/** @file aesxor.h
 *  @brief Class header for native object wrapped in javascript object
 *		   responsible for performing Crypto++ AES cryptography functions
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

#ifndef AESXOR_H
#define AESXOR_H

// ----------------------
// node.js addon includes
// ----------------------
#include <node.h>
#include <node_object_wrap.h>
#include <nan.h>

// -----------------
// cryptopp includes
// -----------------
#include <cryptopp/aes.h>
using CryptoPP::AES;

// ----------------
// pcg rng includes
// ----------------
#include "pcg_random.hpp"


// ---------
// AESXOR256
// ---------

/*
 * @class This class represents a C++ object exposing Crypto++ AES functions,
 *		  wrapped in a javascript object.
 *
 * 		  The functions exposed to node.js are:
 *		  function encrypt(key, message) -> returns cipher
 *		  function decrypt(key, cipher) -> returns message
 */
class AESXOR256 : public Nan::ObjectWrap {

	private:

		// javascript object constructor
		static Nan::Persistent<v8::Function> constructor;
		

		// ----
		// data
		// ----
		//PCG randomg number generator
	 	pcg64_once_insecure* _rng; 
	 	// AES key length
	 	static const int AESNODE_DEFAULT_KEY_LENGTH_BYTES; 


	 	// -----------
		// Constructor
		// -----------
		/**
		 * Constructor
		 * @brief Initializes the object including the PCG random number 
		 * 		  generator using the provided seed and cipher block size.
		 *
		 * @param seed seed for pcg rng
		 * @param blockSize aes encryption block size
		 *
		 * @return 
		 */
	    explicit AESXOR256(uint64_t seed);


	    // ---------
		// getRandom
		// ---------
	 	/**
		 * @brief Gets random uint64 values from pcg and stores them in a byte 
		 *		  array.
		 *
		 * @param random container for resulting random bytes
		 * @param len number of random bytes required
		 *
		 * @return void
		 */
	 	void getRandom(uint8_t* random, int len);


	 	// ------------
		// encryptBlock
		// ------------
		/**
		 * @brief Encrypts the given message using AES in GCM mode to provide 
		 *        confidentiality and authenticity using the given key resulting
		 *        in the cipher block.
		 *
		 * @param cipher byte container for the resulting cipher
		 * @param key byte container for the AES key
		 * @param message byte container for the message
		 *
		 * @throw Cryptopp:Exception in case of encryption errors
		 *
		 * @return void
		 */
	 	void encryptBlock(std::vector<uint8_t>& cipher, 
	 		const std::vector<uint8_t>& key, 
	 		const std::vector<uint8_t>& message);


	 	// ------------
		// decryptBlock
		// ------------
		/**
		 * @brief Decrypts the given cipher using AES in GCM mode to provide 
		 *        confidentiality and authenticity using the given key resulting 
		 *        in the message block.
		 *
		 * @param message byte container for the resulting decrypted message
		 * @param cipher byte container for the input cipher
		 * @param key byte container for the AES key
		 *
		 * @throw Cryptopp:Exception in case of decryption errors
		 *
		 * @return void
		 */
	 	void decryptBlock(std::vector<uint8_t>& message, 
	 		const std::vector<uint8_t>& key, 
	 		const std::vector<uint8_t>& cipher);


	 	// -------------
		// xorRandomData
		// -------------
		/**
		 * @brief XORs the given input container with requal number of random 
		 *        bytes obtained using the PCG random number generator.
		 *
		 * @param output byte container for the resulting XOR'd output
		 * @param input byte container for the input data to be XOR'd with 
		 *		  random bytes
		 *
		 * @return void
		 */
	 	void xorRandomData(std::vector<uint8_t>& output, const std::vector<uint8_t>& input);


		// ---
		// New
		// ---
	    /**
		 * @brief Creates the wrapped object and corresponding underlying 
		 * 		  object with provided arguments - seed buffer
		 *
		 * Invoked as:
		 * 'var obj = new AESXOR256(seed)' or 
		 * 'var obj = AESXOR256(seed)'
		 * 'seed' is a buffer containing bytes representing the uint64 pcg seed
		 *
		 * @param info node.js arguments wrapper containing seed buffer and 
		 *		  block size
		 *
		 * @return void
		 */
		static NAN_METHOD(New);


		// -------
		// encrypt
		// -------
		/**
		 * @brief Unwraps the arguments to get the AES encryption key and 
		 * 		  message, and encrypts the message to return 
		 *		  the cipher.
		 *
		 * Invoked as:
		 * 'var cipher = obj.encrypt(key, message)' 
		 * 'key' is the buffer containing the AES key
		 * 'message' is the buffer containing the message to be encrypted
		 * 'cipher' is the buffer containing the encrypted cipher
 		 * PreCondition: key buffer size == AESNODE_DEFAULT_KEY_LENGTH_BYTES
 		 *
		 * @param info node.js arguments wrapper for AES key and message to be 
		 *		  encrypted
		 *
		 * @return void
		 */
		static NAN_METHOD(encrypt);


		// -------
		// decrypt
		// -------
		/**
		 * @brief Unwraps the arguments to get the AES decryption key and  
		 * 		  cipher, and decrypts the cipher to return the 
		 *		  original message.
		 *
		 * Invoked as:
		 * 'var message = obj.decrypt(key, cipher)' 
		 * 'key' is the buffer containing the AES key
		 * 'cipher' is the buffer containing the cipher to be decrypted
		 * 'message' is the buffer containing the original decypted message
 		 * PreCondition: key buffer size == AESNODE_DEFAULT_KEY_LENGTH_BYTES
 		 *
		 * @param info node.js arguments wrapper for AES key and cipher to be 
		 *		  decrypted
		 *
		 * @return void
		 */
		static NAN_METHOD(decrypt);

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