let addon = require('seifnode');
let assert = require("assert");

// Mocha tests for SEIFSHA3 object.
describe("seifnode SEIFSHA3 hash object", function() {

	// Test should return correct hash value of known input.
	it("should compute correct SHA3-256 hash value", function() {
		let test = new addon.SEIFSHA3();
		let hash = test.hash("abc");

		let testarr =
		[0x3a, 0x98, 0x5d, 0xa7, 0x4f, 0xe2, 0x25, 0xb2, 0x04, 0x5c,
		0x17, 0x2d, 0x6b, 0xd3, 0x90, 0xbd, 0x85, 0x5f, 0x08, 0x6e,
		0x3e, 0x9d, 0x52, 0x5b, 0x46, 0xbf, 0xe2, 0x45, 0x11, 0x43, 0x15, 0x32];

		let testhash = new Buffer(testarr);

		// Comparing returned hash buffer with the known hash value buffer.
		assert.equal(true, hash.equals(testhash));
	});
});
