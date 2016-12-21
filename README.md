seifnode
========

Node.js Implementation of SEIF protocol helper modules

Getting Started
===============

From your project directory, run (see below for requirements):

```
$ npm install seifnode
```

Alternatively, to use the latest development version from Github:

```
$ npm install <git-path>
```

Requirements
============

1. opencv
2. portaudio
3. cryptopp

Test
====

Please refer to the "test" directory to view the "mocha" unit tests. To run the tests, please run the following command from the top-level directory.

```
$ npm test
```

Examples
========

Please refer to the "examples" directory to see examples of how to use the various modules exposed.

Interface
=========

The module exposes four different interfaces useful for different purposes.

###1. RNG

This module exposes the isaac random number generator to node.js. We haven't made any changes to the random number generation process as such. The only enhancement is that we are accessing the random number generator state and encrypting it before persisting it to the disk.

**Initialization:**

```javascript
var seifnode = require("seifnode");
var obj = seifnode.RNG();
```

**Usage:**

The functions exposed are as follows:

**function isInitialized(key, filename, function callback(result){...})**

Creates an async worker to check if the RNG has been initialized by checking if the state file exists and can be decrypted using the given key. Once the async work is complete, the RNG is initialized with the state on the disk if present or an appropriate error is given to the callback.

```javascript
obj.isInitialized(key, filename, function(result) {

	console.log(result.code);
	console.log(result.message);

});
// 'key' is a buffer containing the disk encryption/decryption key
// 'filename' is the name of the RNG saved state file on disk
// 'result' is a js object containing the code('code') and message('message')
```

**function initialize(key, filename)**

Initilizes the RNG by gathering entropy from the available sources (Please look at the "rng" repo for more details at <>). Once the entropy generation is complete, the RNG is initialized using the generated seed and it is ready to be used.

```javascript
obj.initialize(key, filename);
// 'key' is a buffer containing the disk encryption/decryption key
// 'filename' is the name of the RNG saved state file on disk
```

**function getBytes(n)**

Gets the number of random bytes required and returns a buffer with the random output. If the RNG has not been initialized an error will be thrown.

```javascript
var numbytes = 32;
var buffer = obj.getBytes(numBytes);
// 'numBytes' is the number of required random bytes
// 'buffer' is a node.js buffer
```

**function saveState()**

Encrypts and saves the RNG state to disk.

```javascript
obj.saveState(function(result) {

	console.log(result.code);
	console.log(result.message);

});
```

**function destroy()**

Destroys the underlying RNG object thus saving the state to disk.

```javascript
obj.destroy();
```


###2. ECCISAAC

This module is responsible for exposing Crypto++ ECC functions using our implementation of isaac random number generator.

**Initialization:**

```javascript
var seifnode = require("seifnode");
var obj = seifnode.ECCISAAC(diskKey, folder);
// 'diskKey' is the key used to encrypt the keys and rng state
// 'folder' is the folder where the keys and rng state are saved on disk
```

**Usage:**

The functions exposed are as follows:

**function loadKeys()**

Creates an async worker to load keys from the disk (encrypted using the key provided during initialization) and invokes the callback function with the error object (if applicable) and/or the object containing the keys.

```javascript
obj.loadKeys(function(status, keys) {

	// 'status' (if applicable) is of the form: {code: [statusCode], message: [statusMessage]}
	console.log(status);
	// 'keys' (if available) is of the form: {enc: [publicKey], dec: [privateKey]}
	console.log(keys);

});
```

**function generateKeys()**

Initializes the isaac RNG and uses it to generate the public/private keys and return them to the caller. These keys are also encrypted and saved to the disk.

```javascript
var keys = obj.generateKeys();
// 'keys' (if available) is of the form: {enc: [publicKey], dec: [privateKey]}
```

**function encrypt(publicKey, message)**

Encrypts the message buffer using the public key to return the cipher string (We are using Cryptopp ECIES for this purpose and the curve used is the NIST approved SECP521r1. For more details, please refer <>).

```javascript
obj.loadKeys(function(status, keys) {

	if (status === undefined && keys !== undefined) {
			
		var cipher = obj.encrypt(keys.enc, message);
		// 'keys.enc' is the string containing the hex encoded ECC public key
		// 'message' is the buffer containing the message to be encrypted
		// 'cipher' is the string containing the encrypted cipher

	}

});
```

**function decrypt(privateKey, cipher)**

Decrypts the cipher string using the private key to return the original message buffer (We are using Cryptopp ECIES for this purpose and the curve used is the NIST approved SECP521r1. For more details, please refer <>).

```javascript
obj.loadKeys(function(status, keys) {

	if (status === undefined && keys !== undefined) {
			
		var message = obj.decrypt(keys.dec, cipher);
		// 'keys.dec' is the string containing the hex encoded ECC private key
		// 'cipher' is the string containing the cipher to be decrypted
		// 'message' is the buffer containing the decrypted message

	}

});	
```


###3. AESXOR

This module is responsible for exposing our implementation of link encryption. We are exposing the Cryptopp AES implementation in the GCM mode with slight modifications to enhance security as explained below. Similary, after the cipher bytes have been decrypted they are XOR'd with PCG random bytes to get the original message. 

**Initialization:**

```javascript
var seifnode = require("seifnode");
var obj = seifnode.AESXOR(seed);
// 'seed' is a buffer containing bytes representing the uint64 pcg seed
```

**Usage:**

The functions exposed are as follows:

**function encrypt(key, message)**

Encrypts the message using the given key to return the cipher. As part of this process, the message bytes are first XOR'd with equal number of random bytes generated using PCG <> and then encrypted using the AES-GCM mode.

```javascript
var cipher = obj.encrypt(key, message);
// 'key' is the buffer containing the AES key
// 'message' is the buffer containing the message to be encrypted
// 'cipher' is the buffer containing the encrypted cipher
```

**function decrypt(key, cipher)**

Decrypts the cipher to return the original message. As part of this process, after the cipher bytes have been decrypted using the AES-GCM mode, the decrypted buffer is XOR'd with as many PCG random bytes to get the original message.

```javascript
var message = obj.decrypt(key, cipher);
// 'key' is the buffer containing the AES key
// 'cipher' is the buffer containing the cipher to be decrypted
// 'message' is the buffer containing the decrypted message
```


###4. SEIFSHA3

This module is responsible for exposing Crypto++ SHA3 function

**Initialization:**

```javascript
var seifnode = require("seifnode");
var obj = seifnode.SEIFSHA3();
```

**Usage:**

The functions exposed are as follows:

**function hash(data)**

Gets the string data and returns the hash (using Cryptopp implementation of SHA3-256) of the given input as a buffer object.

```javascript
var hash = obj.hash(stringData);
// 'stringData' is the string data to be hashed 
// 'hash' is the output buffer containing the SHA3-256 hash
```


Dependencies
============

###1. Cryptopp/crypto++ 
https://www.cryptopp.com/

Used for all cryptographic functions. Library installed version 5.6.4 

**License:**
Crypto++ Library is copyrighted as a compilation and (as of version 5.6.4) licensed under the Boost Software License 1.0, while the individual files in the compilation are all public domain.
https://www.cryptopp.com/License.txt

###2. PCG C++ Implementation
http://www.pcg-random.org/

Used as the RNG for link encryption. Code included in our node module: https://github.com/imneme/pcg-cpp

**License:** 
Apache License Version 2.0, January 2004
https://github.com/imneme/pcg-cpp/blob/master/LICENSE.txt

###3. OpenCV
http://opencv.org/

Portable solution to access the device camera. This is a required library dependency.

**License:**
OpenCV is released under a BSD license and hence it’s free for both academic and commercial use.

###4. Node modules: nan
https://github.com/nodejs/nan

This is basically a header file containing macros and utilities to store all logic necessary to develop native Node.js addons without having to inspect NODE_MODULE_VERSION

**License & copyright:**
https://github.com/nodejs/nan/blob/master/LICENSE.md

###5. PortAudio
http://www.portaudio.com/

Portable solution to access device audio I/O. This is a required library dependency.

**License:**
We can use PortAudio for free in our projects or applications, even commercial applications.
This license is compatible with the GNU General Public License. In other words, PortAudio can be included in a GNU project without violating the GNU license. In terms of legal compatibility, the PortAudio licence is now a plain MIT licence
http://www.portaudio.com/license.html 

###6. Isaac
http://burtleburtle.net/bob/rand/isaacafa.html

Fast cryptopgraphic random number generator
We are using the C++ implementation with some modifications to access the RNG state.

**License:** 
Public Domain


License
=======

The MIT License (MIT)

Copyright (c) 2015 PayPal

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

