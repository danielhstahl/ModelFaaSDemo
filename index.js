'use strict';  
// Requiring the add-on that we'll build in a moment...
const addon = require('./build/Release/buffer_example');

// Allocates memory holding ASCII "ABC" outside of V8.
const buffer = Buffer.from("ABC");

// synchronous, rotates each character by +13
addon.rotate(buffer, buffer.length, 13);

console.log(buffer.toString('ascii'));  