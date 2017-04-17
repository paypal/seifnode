let addon = require('seifnode');
let fs = require("fs");
let assert = require("assert");

let hash = new Buffer([0xB6,0x8F,0xE4,0x3F,0x0D,0x1A]);
let stateFile = __dirname + "/rng1";
let numBytes = 32;

// Mocha tests for RNG object.
describe("seifnode RNG object", function() {

	before(function() {
		if (fs.existsSync(stateFile)) {
			fs.unlinkSync(stateFile);
		}
	});

	// Testing 'isInitialized' functionality before initializing the RNG.
	describe("#isInitialized() before", function() {

		/* Since RNG is not initialized, no state will be found on the disk and
		 * the corresponding error should be returned.
		 */
		it("should return file not found error when checking if the rng is " +
		   "initialized", function(done) {
			let test = new addon.RNG();

			test.isInitialized(hash, stateFile, function(result) {
				// Checking that the result code indicates file not found error.
				assert.equal(-1, result.code);
				done();
			});
		});
	});

	// Testing 'entropyStrength' functionality.
	describe("#entropyStrength()", function() {

		it("should describe strenght of entropy mining as WEAK, MEDIUM or " +
		   "STRONG", function(done) {
			let test = new addon.RNG();

			let strength = test.entropyStrength();
			let result = 0;
			if (strength === "WEAK" ||
				strength === "MEDIUM" ||
				strength === "STRONG"
			) {
				result = 1;
			}
			assert.equal(1, result);
			done();
		});
	});

	// Testing 'initialize' functionality.
	describe("#initialize()", function() {

		// RNG should be initialized successfully without any errors.
		it("should initialize rng object successfully", function(done) {
			let test = new addon.RNG();
			let hash = new Buffer([0xB6,0x8F,0xE4,0x3F,0x0D,0x1A]);
			this.timeout(150000);

			assert.doesNotThrow(function() {
					test.initialize(hash, stateFile);
				},
				function(err) {
					if ((err instanceof Error)) {
	      				console.log(err);
	      				return true;
	      			}
				},
				"Error thrown"
			);

			test.isInitialized(hash, stateFile, function(result) {
				// Checking that the result code indicates Success.
				assert.equal(0, result.code);
				// Destroy RNG to ensure state file is saved to disk.
				test.destroy();
				done();
			});
		});
	});

	// Testing 'isInitialized' functionality after initializing the RNG.
	describe("#isInitialized() after", function() {

		/* When using a wrong key a decryption error should be returned when
		 * trying to initialize the RNG from state stored on the disk.
		 */
		it("should return decryption error due to incorrect argument",
			function(done) {

			let test = new addon.RNG();
			/* Using a different key instead of the one used before to save
			 * state.
			 */
			let testhash = new Buffer([0xB2,0x8F,0xE4,0x3F,0x0D]);

			test.isInitialized(testhash, stateFile, function(result) {
				// Checking that the result code indicated decryption error.
				assert.equal(-2, result.code);
				done();
			});
		});
	});

	// Testing 'getBytes' functionality.
	describe("#getBytes()", function() {

		// A non-zero buffer of given length should be returned.
		it("should return buffer with given number of random bytes",
			function(done) {

			let test = new addon.RNG();

			test.isInitialized(hash, stateFile, function(result) {
				assert.equal(0, result.code);

				let testBuffer;
				assert.doesNotThrow(function() {
						testBuffer = test.getBytes(numBytes);
					},
					function(err) {
						if ((err instanceof Error)) {
		      				console.log(err);
		      				return true;
		      			}
					},
					"Error thrown"
				);

				// Checking the length of the random bytes buffer.
				assert.equal(numBytes, testBuffer.length);
				let i;
				for (i = 0; i < testBuffer.length; ++i) {
					if (testBuffer[i] !== 0) {
						break;
					}
				}
				// Checking that all bytes in output buffer are not 0.
				assert.notEqual(numBytes, i);

				done();
			});
		});
	});

	after(function() {
		if (fs.existsSync(stateFile)) {
			fs.unlinkSync(stateFile);
		}
	});
});
