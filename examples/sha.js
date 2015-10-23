var addon = require('seifnode');
//var addon = require('./build/Debug/isaacrng');

var test = new addon.SEIFSHA3();

console.log(test.hash("abc"));



