/** @file seifsha3.cc
 *  @brief Definition of the class functions provided in seifsha3.h
 *
 *  @author Aashish Sheshadri
 *  @author Rohit Harchandani
 *  
 *  The MIT License (MIT)
 *  
 *  Copyright (c) 2015 PayPal
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to 
 *  deal in the Software without restriction, including without limitation the 
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER  
 *  DEALINGS IN THE SOFTWARE.
 */

// -----------------
// standard includes
// -----------------
#include <iostream>
#include <string>
#include <array>

// ----------------------
// node.js addon includes
// ----------------------
#include <node_buffer.h>

// -----------------
// cryptopp includes
// -----------------
#include <cryptopp/sha3.h>
using CryptoPP::SHA3_256;

// ----------------
// library includes
// ----------------
#include <isaacRandomPool.h>

#include "seifsha3.h"
#include "util.h"


// javascript object constructor
Nan::Persistent<v8::Function> SEIFSHA3::constructor;



// ---
// New
// ---
/**
 * @brief Creates the node object and corresponding underlying object.
 *
 * Invoked as:
 * 'var obj = new SEIFSHA3()' or 
 * 'var obj = SEIFSHA3()'
 *
 * @param info node.js arguments wrapper
 *
 * @return void
 */
NAN_METHOD(SEIFSHA3::New) {

    if (info.IsConstructCall()) {
        
        // Invoked as constructor: 'var obj = new SEIFSHA3()'.
        SEIFSHA3* obj = new SEIFSHA3();

        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());

    } else {

        // Invoked as plain function `SEIFSHA3()`, turn into construct call.
        const int argc = info.Length();

        // Creating argument array for construct call.
        std::vector<v8::Local<v8::Value> > argv;
        argv.reserve(argc);
        for (int i = 0; i < argc; ++i) {
            argv.push_back(info[i]);
        }

        v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
        info.GetReturnValue().Set(cons->NewInstance(argc, argv.data()));
    
    }
}



// ----
// hash
// ----
/**
 * @brief Unwraps the arguments to get the string data and returns 
 *        the hash of the given input as a buffer object.
 *
 * Invoked as:
 * 'var hash = obj.hash(stringData)' where
 * 'stringData' is the string data to be hashed 
 * 'hash' is the output buffer containing the SHA3-256 hash
 * To generate the output we are using Crypto++ SHA3-256 hash functions
 *
 * @param info node.js arguments wrapper containing string value to be 
 *        hashed
 *
 * @return void
 */
NAN_METHOD(SEIFSHA3::hash) {

    // Checking arguments and unwrapping them to get the string data.
    if (info[0]->IsUndefined()) {
        Nan::ThrowError("Incorrect Arguments. Value to be hashed not "
                        "provided");
        return;
    }

    // Output buffer containing the hash
    std::vector<uint8_t> digest(CryptoPP::SHA3_256::DIGESTSIZE);

    // Check if first argument is a buffer or a string and hash accordingly.
    if (!node::Buffer::HasInstance(info[0])) {
        v8::String::Utf8Value str(info[0]->ToString());

        /* Using crypto++ sha3-256 hash function to hash data and store in 
         * 'digest' array. Definition of 'hashString' can be found in 'util.h'.
         */
        hashString(digest, *str);
    } else {
        // Unwrap the first argument to get the input buffer to be hashed
        v8::Local<v8::Object> bufferObj = 
            Nan::To<v8::Object>(info[0]).ToLocalChecked();
        
        uint8_t* bufferData = (uint8_t*)node::Buffer::Data(bufferObj);
        size_t bufferLength = node::Buffer::Length(bufferObj);

        /* Using crypto++ sha3-256 hash function to hash data and store in 
         * 'digest' array. Definition of 'hashBuffer' can be found in 'util.h'.
         */
        hashBuffer(digest, bufferData, bufferLength);
    }

    // Copying digest hash value to node.js buffer.
    auto slowBuffer = Nan::CopyBuffer((const char*)digest.data(), 
        digest.size()).ToLocalChecked();

    // Set node.js buffer as return value of the function.
    info.GetReturnValue().Set(slowBuffer);
}



// ----
// Init
// ----
/**
 * @brief Initialization function for node.js object wrapper exported  
 *        by the addon.
 *
 * @param exports node.js module exports
 *
 * @return void
 */
void SEIFSHA3::Init(v8::Handle<v8::Object> exports) {

    Nan::HandleScope scope;

    // Prepare constructor template.
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("SEIFSHA3").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype
    Nan::SetPrototypeMethod(tpl, "hash", hash);
    
    constructor.Reset(tpl->GetFunction());

    // Setting node.js module.exports.
    exports->Set(Nan::New("SEIFSHA3").ToLocalChecked(), tpl->GetFunction());
}
