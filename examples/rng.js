var addon = require('seifnode');

var hash = new Buffer([0xB6,0x8F,0xE4,0x3F,0x0D,0x1A]);
// var test = new addon.RNG(hash, "/Users/rharchandani/Documents/gitstuff/seifnode/");

// console.log(test.getBytes(32));

var test = new addon.RNG();

test.isInitialized(hash, "/Users/rharchandani/Documents/gitstuff/seifnode/tests/rng1", function(result) {
	console.log("done");
	console.log(result);
	//console.log(test.getBytes(32));
	test.initialize(hash, "/Users/rharchandani/Documents/gitstuff/seifnode/tests/rng1");
});

console.log("outside");