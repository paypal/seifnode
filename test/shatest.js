var addon = require('../index.js');
var assert = require("assert");

// Mocha tests for SEIFSHA3 object.
describe("seifnode SEIFSHA3 hash object", function () {

    // Test should return correct hash value of known input.
    it("should compute correct SHA3-256 hash value", function () {
        var test = new addon.SEIFSHA3();
        var hash = test.hash("abc");

        var testarr =
                [58, 152, 93, 167, 79, 226, 37, 178, 4, 92, 23, 45, 107, 211,
                144, 189, 133, 95, 8, 110, 62, 157, 82, 91, 70, 191, 226, 69,
                17, 67, 21, 50];

        var testhash = new Buffer(testarr);

        // Comparing returned hash buffer with the known hash value buffer.
        assert.equal(true, hash.equals(testhash));
    });
});
