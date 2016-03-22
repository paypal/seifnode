var addon = require('../index.js');
var fs = require("fs");
var glob = require("glob")
var assert = require("assert");

var hash = new Buffer([0xB6,0x8F,0xE4,0x3F,0x0D,0x1A,0x0D,0x7A,0xEF,0x12,0x37,
	0x22,0x67,0x0B,0xE5,0x02,0x68,0xE1,0x53,0x65,0x40,0x1C,0x44,0x2F,0x88,0x06,
	0xEF,0x83,0xB6,0x12,0x97,0x6B]);

var eccFolder = __dirname;

var msg = new Buffer([0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
	0x41,0x41,0x41,0x41,0x41]);

// variable representing the keys loaded from disk
var loadedKeys;

// variable representing buffer containing encrypted message
var cipher;

// Mocha tests for ECCISAAC object.
describe("seifnode ECCISAAC object", function() {

	// Deleting all files stored during prior tests.
	before(function() {
		var filenames = glob.sync(eccFolder + "/.ecies*");
		filenames.forEach(function(val, index, arr) {
			fs.unlinkSync(val);
		});
	});

	// Testing 'loadKeys' functionality before generating the keys.
	describe("#loadKeys() before", function() {

		/* Test should set the status object with an error when trying to load  
		 * keys from the disk when no files are present.
		 */
		it("should return a file not found error when trying to load keys", 
			function(done) {

			var test = new addon.ECCISAAC(hash, eccFolder);

			test.loadKeys(function(status, keys) {
				// Checking if status code indicates file not found error.
				assert.equal(-1, status.code);
				done();
			});
		});

	});

	// Testing 'generateKeys' functionality.
	describe("#generateKeys()", function() {

		/* Test should generate the keys and set the status object indicating 
		 * success.
		 */
		it("should generate keys and store them on the disk so that they can " +
		   "be loaded", function(done) {

			var test = new addon.ECCISAAC(hash, eccFolder);

			this.timeout(150000);
			

			assert.doesNotThrow(function() {
					var generatedKeys = test.generateKeys();
				}, 
				function(err) {
					if ((err instanceof Error)) {
	      				console.log(err);	
	      				return true;
	      			}
				}, 
				"Error thrown"
			);

			test.loadKeys(function(status, keys) {
				// Checking if status code indicates success.
				assert.equal(0, status.code);
				done();
			});

		});

	});

	// Testing 'loadKeys' functionality after keys have been generated.
	describe("#loadKeys() after", function() {

		/* Test should set the status object with a decryption error when 
		 * trying to load ECC keys from the disk when an incorrect key is given.
		 */
		it("should return an error when trying to load keys with wrong key", 
			function(done) {

			var testhash = new Buffer([0xB2,0x8F,0xE4,0x3F,0x0D]);
			var test = new addon.ECCISAAC(testhash, eccFolder);

			test.loadKeys(function(status, keys) {
				assert.equal(-2, status.code);
				done();
			});
		});

	});

	// Testing 'encrypt' functionality.
	describe("#encrypt()", function() {

		/* Test should return cipher string without throwing any exception when 
		 * encrypting message.
		 */
		it("should encrypt msg and return the cipher without any errors", 
			function(done) {
			
			var test = new addon.ECCISAAC(hash, eccFolder);

			test.loadKeys(function(status, keys) {

				// Checking that no exception is thrown when encrypting message.
				assert.doesNotThrow(function() {
					cipher = test.encrypt(keys.enc, msg);
				}, 
				function(err) {
					if ((err instanceof Error)) {
	      				console.log(err);	
	      				return true;
	      			}
				}, 
				"Error thrown");

				done();
			});

		});
	});

	// Testing 'decrypt' functionality.
	describe("#decrypt()", function() {

		/* Test should return message buffer without throwing any exception when 
		 * decrypting cipher and this message should be same as the original.
		 */
		it("should decrypt cipher and return the original message without any" +
		   " errors", 
			function(done) {
			
			var test = new addon.ECCISAAC(hash, eccFolder);

			test.loadKeys(function(status, keys) {

				var decryptedMsg;

				// Checking that no exception is thrown when decrypting.
				assert.doesNotThrow(function() {
					decryptedMsg = test.decrypt(keys.dec, cipher);
				}, 
				function(err) {
					if ((err instanceof Error)) {
	      				console.log(err);	
	      				return true;
	      			}
				}, 
				"Error thrown");

				// Checking that decrypted message is same as the original.
				assert.equal(true, decryptedMsg.equals(msg));
				done();
			});

		});

		/* Test should take a different key from the one used to encrypt the  
		 * message and throw an exception when trying to decrypt the corresponding 
		 * cipher with it
		 */
		it("should give an error when decrypting with wrong key", 
			function(done) {
			
			var test = new addon.ECCISAAC(hash, eccFolder);

			test.loadKeys(function(status, keys) {
				// Modifying the loaded private key.
				keys.dec = keys.dec.substr(0, keys.dec.length - 1);
				keys.dec = keys.dec + "A";

				/* Checking that an exception is thrown when decrypting with 
				 * the wrong private key.
				 */
				assert.throws(function() {
					var decryptedMsg = test.decrypt(keys.dec, cipher);
					console.log(decryptedMsg);
				}, 
				function(err) {
					if ((err instanceof Error)) {
	      				console.log(err);	
	      				return true;
	      			}
				}, 
				"Error thrown");
			
				done();
			});

		});
	});

	after(function() {
		var filenames = glob.sync(eccFolder + "/.ecies*");
		filenames.forEach(function(val, index, arr) {
			fs.unlinkSync(val);
		});
	});
});

