var addon = require('seifnode');
var assert = require("assert");

// Mocha tests for SEIFSHA3 object.
describe("seifnode SEIFSHA3 hash object", function() {

	// Test should return correct hash value of known input.
	it("should compute correct SHA3-256 hash value", function() {
		var test = new addon.SEIFSHA3();
		var hash = test.hash("abc");
		
		var testarr = 
		[0x4e, 0x03, 0x65, 0x7a, 0xea, 0x45, 0xa9, 0x4f, 0xc7, 
		0xd4, 0x7b, 0xa8, 0x26, 0xc8, 0xd6, 0x67, 0xc0, 0xd1, 0xe6, 0xe3, 
		0x3a, 0x64, 0xa0, 0x36, 0xec, 0x44, 0xf5, 0x8f, 0xa1, 0x2d, 0x6c, 0x45]

		var testhash = new Buffer(testarr);

		// Comparing returned hash buffer with the known hash value buffer.
		assert.equal(true, hash.equals(testhash));
	})
})
