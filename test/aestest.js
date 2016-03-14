var addon = require('../index.js');
var assert = require("assert");

// buffer containing seed for ocg random number generator used by AESXOR
var seedBuffer = new Buffer([0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff]);

// 16 byte message block to be encrypted
var msg = new Buffer([0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0,0,0,0,0,0,0,0]);

// 32 byte key to be used to encrypt message
var key = new Buffer([0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff]);

// variable representing buffer containing encrypted message
var cipher;

// Mocha tests for AESXOR object.
describe("seifnode AESXOR object", function() {

	// Testing 'encrypt' functionality.
	describe("#encrypt()", function() {

		/* Test should encrypt message and return cipher buffer of same length  
		 * as original message block and should contain different data.
		 */
		it("should encrypt msg and return a buffer with the cipher", 
			function() {
			
			var test = addon.AESXOR256(seedBuffer);

			cipher = test.encrypt(key, msg);

			// checking if the cipher is differs from the message
			assert.notEqual(true, cipher.equals(msg));
		});

		/* Test should take a shorter key and throw an exception when trying to 
		 * encrypt a message with it.
		 */
		it("should give an error when encrypting a message with key of wrong " +
		   "length", function() {

			var test = addon.AESXOR256(seedBuffer);

			var wrongKey = new Buffer([0xff,0xff,0xff,0xff,0xff,0,0,0,0,0,0]);

			// Checking if 'encrypt' throws an exception.
			assert.throws(function() {
				var wrongCipher = test.encrypt(wrongKey, msg);
			}, 
			function(err) {
				if ((err instanceof Error) && /Incorrect Arguments/.test(err)) {
      				console.log(err);	
      				return true;
      			}
			}, 
			"Error thrown");
		});
	});

	// Testing 'decrypt' functionality.
	describe("#decrypt()", function() {

		/* Test should decrypt cipher and return message buffer which should 
		 * be exactly same as the original buffer.
		 */
		it("should decrypt cipher and return the same message", function() {
			
			var test = addon.AESXOR256(seedBuffer);

			var decryptedMsg = test.decrypt(key, cipher);

			// Checking that the decrypted buffer is same as original message.
			assert.equal(true, decryptedMsg.equals(msg));
		});

		/* Test should take a shorter key and throw an exception when trying to 
		 * decrypt a cipher with it.
		 */
		it("should give an error when decrypting a message with key of wrong " +  
		   "length", function() {

			var test = addon.AESXOR256(seedBuffer);

			var wrongKey = new Buffer([0xff,0xff,0xff,0xff,0xff,0,0,0,0,0,0]);

			// Checking if 'decrypt' throws an exception.
			assert.throws(function() {
				var wrongCipher = test.decrypt(wrongKey, cipher);
			}, 
			function(err) {
				if ((err instanceof Error) && /Incorrect Arguments/.test(err)) {
      				console.log(err);	
      				return true;
      			}
			}, 
			"Error thrown");
		});

		/* Test should take a different key from the one used to encrypt the  
		 * message and throw an exception when trying to decrypt the  
		 * corresponding cipher with it.
		 */
		it("should give an error when decrypting a message with wrong key",  
		    function() {

			var test = addon.AESXOR256(seedBuffer);

			// Modifying the first byte of the key.
			var wrongKey = key;
			wrongKey[0] = 0;

			// Checking if a decryption error is thrown.
			assert.throws(function() {
				var wrongCipher = test.decrypt(wrongKey, cipher);
			}, 
			function(err) {
				if ((err instanceof Error)) {
      				console.log(err);	
      				return true;
      			}
			}, 
			"Error thrown");
		});
	});
});



