/** @file eccisaac.h
 *  @brief Class header for native object wrapped in javascript object
 *		   responsible for performing Crypto++ ECC cryptography functions
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

#ifndef ECCISAAC_H
#define ECCISAAC_H

// -----------------
// standard includes
// -----------------
#include <string>
#include <vector>

// ----------------------
// node.js addon includes
// ----------------------
#include <node.h>
#include <node_object_wrap.h>
#include <nan.h>

// -----------------
// cryptopp includes
// -----------------
#include <cryptopp/pubkey.h>
using CryptoPP::PublicKey;
using CryptoPP::PrivateKey;



// --------
// ECCISAAC
// --------

/*
 * @class This class represents a C++ object wrapped inside a javascript object, 
 *		  exposing Crypto++ ECC functions using our implementation of 
 *		  isaac random number generator.
 *
 * 		  The functions exposed to node.js are:
 *		  function loadKeys() -> returns public/private key object
 *		  function generateKeys() -> returns public/private key object
 *		  function encrypt(publicKey, message) -> returns cipher
 *		  function decrypt(privateKey, cipher) -> returns message
 */
class ECCISAAC : public Nan::ObjectWrap {

	private:

		// javascript object constructor
		static Nan::Persistent<v8::Function> constructor;

		// Status enum for different types of errors
		enum class STATUS:int {
			SUCCESS = 0, 			// Success
			FILE_NOT_FOUND = -1, 	// RNG state file not found
			DECRYPTION_ERROR = -2, 	// Error Decrypting RNG state file
			ENTROPY_ERROR = -3,		// Error gathering entropy
			RNG_INIT_ERROR = -4		// RNG not initialized
		};

		// ----
		// data
		// ----
		// Disk encrypt/decrypt key
	 	std::vector<uint8_t> _key; 
	 	// Folder containing keys + RNG state
	 	std::string _folderPath; 



	 	// ------
		// Worker
		// ------
		/*
		 * @class This class represents the node.js async worker responsible for 
		 *		  loading public/private keys from the disk and invoke the 
		 *		  given callback function with the status of the operation and 
		 *		  keys if available
		 */
		class Worker: public Nan::AsyncWorker {
		    
		    private:
		    	// ----
				// data
				// ----

				// folder containing keys and rng state files
		        std::string _wfolderPath;
		        // disk access key for public/private keys and rng state 
		        std::vector<uint8_t> _wkey;
		        // status of loading keys
		        ECCISAAC::STATUS _status; 
		        // encoded public key 
		        std::string _encodedPub; 
		        // encoded private key
		        std::string _encodedPriv; 

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
				 * @param key disk access key for public/private keys and 
				 * 		  rng state 
				 * @param folderPath folder containing keys and rng state files
				 */
		        // Worker(Nan::Callback* initCallback, ECCISAAC* obj);
				Worker(
					Nan::Callback* initCallback, 
					const std::vector<uint8_t>& key, 
					const std::string& folderPath
				);	


		        // ----------------
				// HandleOKCallback
				// ----------------
		        /**
		         * @brief Executed when the async work is complete without
		         * 		  error, this function will be run inside the main event  
		         * 		  loop, invoking the given callback with the loaded 
		         *		  keys as an argument.
		         *
		         * The keys are returned as the second argument to the callback  
		         * {enc: [publicKey], dec: [privateKey]}
		         *
		         * @return void
		         */
		        void HandleOKCallback ();


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
		        void HandleErrorCallback ();


		        // -------
				// Execute
				// -------
		        /**
		         * @brief Executed in a separate thread, asynchronously loading 
		         *		  the public/private keys from the disk and 
		         *		  communicating the status of the operation via the 
		         *		  Worker class.
		         *
		         * @return void
		         */
		        void Execute();
		};



	 	// -----------
		// Constructor
		// -----------
		/**
		 * Constructor
		 * @brief Initilizes and constructs internal data.
		 *
		 * @param keyData byte vector corresponding to disk access key 
		 * @param folderPath folder containing the encrypted keys
		 */
	    explicit ECCISAAC(const std::vector<uint8_t>& keyData, 
	    	const std::string& folderPath);


	    // --------------
		// SavePrivateKey
		// --------------
	    /**
		 * @brief Encrypt and save the private key to the disk with the given 
		 *		  file name.
		 *
		 * @param privateKey private key object
		 * @param file file name of encrypted private key
		 * @param key disk access key for public/private keys and 
		 * 		  rng state 
		 * @param folderPath folder containing keys and rng state files
		 *
		 * @return void
		 */
		static void SavePrivateKey(
			const PrivateKey& privateKey, 
			const std::string& file, 
			const std::vector<uint8_t>& key, 
			const std::string& folderPath
		);


		// -------------
		// SavePublicKey
		// -------------
		/**
		 * @brief Encrypt and save the public key to the disk with the given 
		 *		  file name.
		 *
		 * @param publicKey public key object
		 * @param file file name of encrypted public key
		 * @param key disk access key for public/private keys and 
		 * 		  rng state 
		 * @param folderPath folder containing keys and rng state files
		 *
		 * @return void
		 */
		static void SavePublicKey(
			const PublicKey& publicKey, 
			const std::string& file, 
			const std::vector<uint8_t>& key, 
			const std::string& folderPath
		);


		// --------------
		// LoadPrivateKey
		// --------------
		/**
		 * @brief Decrypt the file with the given name and load it as the 
		 *		  private key.
		 *
		 * @param key private key object
		 * @param file file name of encrypted private key
		 * @param key disk access key for public/private keys and 
		 * 		  rng state 
		 * @param folderPath folder containing keys and rng state files
		 *
		 * @return status code indicating success or cause of error
		 */
		static STATUS LoadPrivateKey(
			PrivateKey& privateKey, 
			const std::string& file, 
			const std::vector<uint8_t>& key, 
			const std::string& folderPath
		);


		// -------------
		// LoadPublicKey
		// -------------
		/**
		 * @brief Decrypt the file with the given name and load it as the 
		 *		  public key.
		 *
		 * @param key public key object
		 * @param file file name of encrypted public key
		 * @param key disk access key for public/private keys and 
		 * 		  rng state 
		 * @param folderPath folder containing keys and rng state files
		 *
		 * @return status code indicating success or cause of error
		 */
		static STATUS LoadPublicKey(
			PublicKey& publicKey, 
			const std::string& file, 
			const std::vector<uint8_t>& key, 
			const std::string& folderPath
		);


		// --------
		// loadKeys
		// --------
		/**
		 * @brief Loads the keys by decrypting existing files and initializes 
		 *		  the random number generator.
		 *
		 * @param encodedPub public key to be loaded from encrypted file
		 * @param encodedPriv private key to be loaded from encrypted file
		 * @param key disk access key for public/private keys and 
		 * 		  rng state 
		 * @param folderPath folder containing keys and rng state files
		 *
		 * @return status code indicating success or cause of error
		 */
		static STATUS loadKeys(
			std::string& encodedPub, 
			std::string& encodedPriv, 
			const std::vector<uint8_t>& key, 
			const std::string& folderPath
		);


		// ------------
		// generateKeys
		// ------------
		/**
		 * @brief Initialize the RNG and use it to generate the public and 
		 *		  private keys, encrypt them and save to disk.
		 *
		 * @param encodedPub public key to be generated
		 * @param encodedPriv private key to be generated
		 * @param key disk access key for public/private keys and 
		 * 		  rng state 
		 * @param folderPath folder containing keys and rng state files
		 *
		 * @return boolean indicating success/failure
		 */
		static bool generateKeys(
			std::string& encodedPub, 
			std::string& encodedPriv, 
			const std::vector<uint8_t>& key, 
    		const std::string& folderPath
    	);



		// ---
		// New
		// ---
		/**
		 * @brief Creates the node object and corresponding underlying object 
		 *		  with provided arguments - disk access key and folder 
		 *		  containing the encrypted keys.
		 *
		 * Invoked as:
		 * 'var obj = new ECCISAAC(diskKey, folder)' or 
		 * 'var obj = ECCISAAC(diskKey, folder)' where
		 * 'diskKey' is the key used to encrypt the keys and rng state
		 * 'folder' is the folder where the keys and rng state are saved on disk
		 *
		 * @param info node.js arguments wrapper containing the disk access key 
		 * 		  and folder path
		 *
		 * @return 
		 */
		static NAN_METHOD(New);


		// --------
		// loadKeys
		// --------
		/**
		 * @brief Creates an async worker to load keys from the disk and 
		 *		  invokes the callback function with the error object (if 
		 *		  applicable) and/or the object containing the keys.
		 *
		 * Invoked as:
		 * 'obj.loadKeys(function(status, keys){})' where
		 * 'status' (if applicable) is of the form: 
		 * {code: [statusCode], message: [statusMessage]}
		 * 'keys' (if available) is of the form:
		 * {enc: [publicKey], dec: [privateKey]}
		 *
		 * @param info node.js arguments wrapper
		 *
		 * @return void
		 */
		static NAN_METHOD(loadKeys);


		// ------------
		// generateKeys
		// ------------
		/**
		 * @brief Initializes the isaac RNG and uses it to generate the 
		 *		  public/private keys and return them to the caller.
		 *
		 * Invoked as:
		 * 'var keys = obj.generateKeys()' where
		 * 'keys' (if available) is of the form:
		 * {enc: [publicKey], dec: [privateKey]}
		 *
		 * @param info node.js arguments wrapper
		 *
		 * @return void
		 */
		static NAN_METHOD(generateKeys);


		// -------
		// encrypt
		// -------
		/**
		 * @brief Unwraps the arguments to get the public key and message and 
		 *		  encrypt the message using the public key to return the cipher.
		 *
		 * Invoked as:
		 * 'var cipher = obj.encrypt(key, message)' 
		 * 'key' is the string containing the hex encoded ECC public key
 		 * 'message' is the buffer containing the message to be encrypted
 		 * 'cipher' is the string containing the encrypted cipher
 		 *
		 * @param info node.js arguments wrapper containing public key string 
		 *		  and message
		 *
		 * @return void
		 */
		static NAN_METHOD(encrypt);


		// -------
		// decrypt
		// -------
		/**
		 * @brief Unwraps the arguments to get private key and cipher and  
		 *		  decrypt the cipher using the private key to return the 
		 *		  original message.
		 *
		 * Invoked as:
		 * 'var message = obj.decrypt(key, cipher)' 
		 * 'key' is the string containing the hex encoded ECC private key
 		 * 'cipher' is the string containing the cipher to be decrypted
 		 * 'message' is the buffer containing the decrypted message
 		 *
		 * @param info node.js arguments wrapper containing private key string 
		 *		  and cipher.
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