/** @file aesxor.cc
 *  @brief Definition of the class functions provided in aesxor.h and helper
 *         functions to convert bytes container to a number and vice versa
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
#include <sstream>
#include <fstream>
#include <string>
#include <vector>

// ----------------------
// node.js addon includes
// ----------------------
#include <node_buffer.h>


// -----------------
// cryptopp includes
// -----------------
#include <cryptopp/filters.h>
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::ArraySink;
using CryptoPP::ArraySource;
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <cryptopp/gcm.h>

// ----------------
// library includes
// ----------------
#include "aesxor.h"


// javascript object constructor
Nan::Persistent<v8::Function> AESXOR256::constructor;
// AES key length
const int AESXOR256::AESNODE_DEFAULT_KEY_LENGTH_BYTES = 32;


// -------------
// uInt64toBytes
// -------------
/**
 * @brief Convert uint64 array to an array of bytes.
 *
 * We traverse the input container from 'begin' to 'end' and store the 
 * extracted bytes into 'out'
 *
 * @param begin beginning of container of uint64 values
 * @param end end of container of uint64 values
 * @param out beginning of container of resulting byte values
 *
 * @return void
 */
template <typename II, typename OI>
void uInt64toBytes(II begin, II end, OI out) {
    // traverse bytes in uint64 values and store in 'out'
    while (begin != end) {
        for (int i = 0; i < 8; ++i) {
            *out = static_cast<uint8_t> ( 
                ((*begin) & ((0x00000000000000FF) << (8 * i))) >> (8 * i)
            );
            ++out;
        }
        ++begin;
    }
}


// -------------
// bytesToUInt64
// -------------
/**
 * @brief Convert array of bytes to a uint64 value
 *
 * @param bufferData container of byte values
 * @param bufferLength size of container of byte values
 *
 * @return uint64 value containing the result of the conversion
 */
uint64_t bytesToUInt64(uint8_t* bufferData, size_t bufferLength) {
    uint64_t returnVal = 0;
    int idx = 0;

    while (bufferLength > 0) {

        returnVal = (returnVal << 8) + (uint8_t)bufferData[idx];
        idx = idx + 1;
        bufferLength = bufferLength - 1;

    }

    return returnVal;
}


// -----------
// Constructor
// -----------
/**
 * Constructor
 * @brief Initializes the object including the PCG random number 
 *        generator using the provided seed and cipher block size.
 *
 * @param seed seed for pcg rng
 * @param blockSize aes encryption block size
 *
 * @return 
 */
AESXOR256::AESXOR256(uint64_t seed): 
    _rng(new pcg64_once_insecure(seed)) {

}


// ---------
// getRandom
// ---------
/**
 * @brief Gets random uint64 values from pcg and stores them in a byte 
 *        array.
 *
 * @param random container for resulting random bytes
 * @param len number of random bytes required
 *
 * @return void
 */
void AESXOR256::getRandom(uint8_t* random, int len) {
    /* Getting the number of random uint64 values required based on 
     * the number of bytes asked for.
     */
    int numRand = len % 8 == 0 ? len / 8 : len / 8 + 1;

    std::vector<uint64_t> randomVector(numRand);

    for (int i = 0; i < numRand; ++i) {
        /* Getting random uint64 value from pcg random number generator 
         * using operator().
         */
        randomVector[i] = (*_rng)();
    }

    // Convert uint64 container to a byte array containing random values.
    uInt64toBytes(randomVector.begin(), randomVector.end(), random);
}



// ------------
// encryptBlock
// ------------
/**
 * @brief Encrypts the given message using AES in GCM mode to provide 
 *        confidentiality and authenticity using the given key resulting
 *        in the cipher block.
 *
 * @param cipher byte container for the resulting cipher
 * @param key byte container for the AES key
 * @param message byte container for the message
 *
 * @throw Cryptopp:Exception in case of encryption errors
 *
 * @return void
 */
void AESXOR256::encryptBlock(std::vector<uint8_t>& cipher, 
    const std::vector<uint8_t>& key, const std::vector<uint8_t>& message) {
    
    // initial vector (IV) for AES to XOR
    std::vector<uint8_t> iv(CryptoPP::AES::BLOCKSIZE);

    std::string ciphertext; //store encrypted message


    // Initialize AES with GCM mode
    CryptoPP::GCM<AES>::Encryption e;

    // Set AES Key and load IV.
    e.SetKeyWithIV(key.data(), key.size(), iv.data());
    
    /* Apply the Cryptopp AuthenticatedEncryptionFilter to the message buffer 
     * using ArraySource, to transform it into an encrypted string 'ciphertext'.
     */
    ArraySource ss1(
        message.data(), 
        message.size(),
        true,
        new CryptoPP::AuthenticatedEncryptionFilter(e, 
            new StringSink( ciphertext )
        ) // AuthenticatedEncryptionFilter
    ); // ArraySource

    // Store the string cipher into the cipher byte vector.
    cipher.resize(ciphertext.size());
    StringSource ss2(
        ciphertext,
        true,
        new ArraySink(cipher.data(), cipher.size())
    );


    return;
}


// ------------
// decryptBlock
// ------------
/**
 * @brief Decrypts the given cipher using AES in GCM mode to provide 
 *        confidentiality and authenticity using the given key resulting 
 *        in the message block.
 *
 * @param message byte container for the resulting decrypted message
 * @param cipher byte container for the input cipher
 * @param key byte container for the AES key
 *
 * @throw Cryptopp:Exception in case of decryption errors
 *
 * @return void
 */
void AESXOR256::decryptBlock(std::vector<uint8_t>& message, 
    const std::vector<uint8_t>& key, const std::vector<uint8_t>& cipher) {

    // initial vector (IV) for AES to XOR
    std::vector<uint8_t> iv(CryptoPP::AES::BLOCKSIZE);

    std::string decryptedtext; //store decrypted message


    // initialize AES
    CryptoPP::GCM< AES >::Decryption e;

    // Set AES Key and load IV.
    e.SetKeyWithIV(key.data(), key.size(), iv.data());
    
    /* Apply the Cryptopp AuthenticatedDecryptionFilter to the cipher buffer 
     * using ArraySource, to transform it into the decrypted string.
     */
    ArraySource ss1(
        cipher.data(),
        cipher.size(),
        true,
        new CryptoPP::AuthenticatedDecryptionFilter(e, 
            new StringSink( decryptedtext )
        ) // AuthenticatedDecryptionFilter
    ); // ArraySource
    
    // Store the decrypted string into the message byte vector.
    message.resize(decryptedtext.size());
    StringSource ss2(
        decryptedtext,
        true,
        new ArraySink(message.data(), message.size())
    );


    return;
}



// -------------
// xorRandomData
// -------------
/**
 * @brief XORs the given input container with requal number of random 
 *        bytes obtained using the PCG random number generator.
 *
 * @param output byte container for the resulting XOR'd output
 * @param input byte container for the input data to be XOR'd with 
 *        random bytes
 *
 * @return void
 */
void AESXOR256::xorRandomData(std::vector<uint8_t>& output, 
    const std::vector<uint8_t>& input) {

    // Get PCG random bytes based on the size of the input vector.
    std::vector<uint8_t> random(input.size());
    getRandom(random.data(), random.size());

    /* XOR the random bytes with the input vector and store the result in the 
     * output vector
     */
    std::transform(input.begin(), input.begin() + input.size(), 
        random.data(), output.begin(), std::bit_xor<uint8_t>());
}


// ---
// New
// ---
/**
 * @brief Creates the wrapped object and corresponding underlying 
 *        object with provided arguments - seed buffer and block size.
 *
 * Invoked as:
 * 'var obj = new AESXOR256(seed)' or 
 * 'var obj = AESXOR256(seed)'
 * 'seed' is a buffer containing bytes representing the uint64 pcg seed
 *
 * @param info node.js arguments wrapper containing seed buffer and 
 *        block size
 *
 * @return void
 */
NAN_METHOD(AESXOR256::New) {

    if (info.IsConstructCall()) {
        // Invoked as constructor: `new AESXOR256(...)`.
        
        // Checking if first argument (seed) is a valid node.js buffer.
        if (!node::Buffer::HasInstance(info[0])) {

            Nan::ThrowError("Incorrect Arguments. Seed buffer not provided");
            return;
        }

        // Generating the uin64 seed from the node.js seed buffer.
        v8::Local<v8::Object> bufferObj = 
            Nan::To<v8::Object>(info[0]).ToLocalChecked();
        
        uint8_t* bufferData = (uint8_t*)node::Buffer::Data(bufferObj);
        size_t bufferLength = node::Buffer::Length(bufferObj);

        uint64_t seed = bytesToUInt64(bufferData, bufferLength);

        // Initializing the wrapped object with given seed.
        AESXOR256* obj = new AESXOR256(seed);

        obj->Wrap(info.This());

        info.GetReturnValue().Set(info.This());

    } else {
        // Invoked as plain function `AESXOR256(...)`, turn into construct call.
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




// -------
// encrypt
// -------
/**
 * @brief Unwraps the arguments to get the AES encryption key and 
 *        message, and encrypts the message block by block to return 
 *        the cipher.
 *
 * Invoked as:
 * 'var cipher = obj.encrypt(key, message)' 
 * 'key' is the buffer containing the AES key
 * 'message' is the buffer containing the message to be encrypted
 * 'cipher' is the buffer containing the encrypted cipher
 * PreCondition: key buffer size == AESNODE_DEFAULT_KEY_LENGTH_BYTES
 *
 * @param info node.js arguments wrapper for AES key and message to be 
 *        encrypted
 *
 * @return void
 */
NAN_METHOD(AESXOR256::encrypt) {

    AESXOR256* obj = ObjectWrap::Unwrap<AESXOR256>(info.Holder());

    // Checking arguments.
    if (info.Length() < 2 
        || !node::Buffer::HasInstance(info[0]) 
        || !node::Buffer::HasInstance(info[1])) {

        Nan::ThrowError("Incorrect Arguments. Please provide buffers for 'key' "
                        "and 'message' function encrypt(key, message)'");
        return;
    }

    /* Unwrap the first argument to get the AES key buffer and validate 
     * that the key length = AESNODE_DEFAULT_KEY_LENGTH_BYTES.
     */
    v8::Local<v8::Object> bufferObj0 = 
                Nan::To<v8::Object>(info[0]).ToLocalChecked();
    uint8_t* keyData = (uint8_t *)node::Buffer::Data(bufferObj0);
    size_t keyLength = node::Buffer::Length(bufferObj0);

    if (keyLength != AESNODE_DEFAULT_KEY_LENGTH_BYTES) {
        Nan::ThrowError("Incorrect Arguments. Please provide a key of size "
                        "32 bytes");
        return;
    }

    // Unwrap the second argument to get the message buffer.
    v8::Local<v8::Object> bufferObj1 = 
                Nan::To<v8::Object>(info[1]).ToLocalChecked();
    uint8_t* messageData = (uint8_t *)node::Buffer::Data(bufferObj1);
    size_t messageLength = node::Buffer::Length(bufferObj1);
    
    // XOR random bytes with the given message buffer before encrypting it.
    std::vector<uint8_t> temp(messageLength);
    obj->xorRandomData(temp, 
        std::vector<uint8_t>(messageData, messageData + messageLength)
    );


    // Encrypt the XOR'd bytes using the given key and store in ciper data.
    std::vector<uint8_t> cipherData;
    try {

        obj->encryptBlock(cipherData, 
            std::vector<uint8_t>(keyData, keyData + keyLength), temp
        );

    } catch (const CryptoPP::Exception& e) {
        
        // Throw an error to node.js in case of encryption errors.
        Nan::ThrowError(e.what());
        return;
    }

    // Copy cipherData vector into a node.js buffer.
    auto slowBuffer = Nan::CopyBuffer((const char*)cipherData.data(), 
        cipherData.size()).ToLocalChecked();

    // Set node.js buffer as return value of the function
    info.GetReturnValue().Set(slowBuffer);
}



// -------
// decrypt
// -------
/**
 * @brief Unwraps the arguments to get the AES decryption key and  
 *        cipher, and decrypts the cipher block by block to return the 
 *        original message.
 *
 * Invoked as:
 * 'var message = obj.decrypt(key, cipher)' 
 * 'key' is the buffer containing the AES key
 * 'cipher' is the buffer containing the cipher to be decrypted
 * 'message' is the buffer containing the original decypted message
 * PreCondition: key buffer size == AESNODE_DEFAULT_KEY_LENGTH_BYTES
 *
 * @param info node.js arguments wrapper for AES key and cipher to be 
 *        decrypted
 *
 * @return void
 */
NAN_METHOD(AESXOR256::decrypt) {

    AESXOR256* obj = ObjectWrap::Unwrap<AESXOR256>(info.Holder());

    // Checking arguments.
    if (info.Length() < 2 
        || !node::Buffer::HasInstance(info[0]) 
        || !node::Buffer::HasInstance(info[1])) {

        Nan::ThrowError("Incorrect Arguments. Please provide buffers for 'key' "
                        "and 'message' -> 'function encrypt(key, message)'");
        return;
    }

    /* Unwrap the first argument to get the AES key buffer and validate 
     * that the key length = AESNODE_DEFAULT_KEY_LENGTH_BYTES.
     */
    v8::Local<v8::Object> bufferObj0 = 
                Nan::To<v8::Object>(info[0]).ToLocalChecked();
    uint8_t* keyData = (uint8_t *)node::Buffer::Data(bufferObj0);
    size_t keyLength = node::Buffer::Length(bufferObj0);

    if (keyLength != AESNODE_DEFAULT_KEY_LENGTH_BYTES) {
        Nan::ThrowError("Incorrect Arguments. Please provide a key of size "
                        "32 bytes");
        return;
    }

    // Unwrap the second argument to get the message buffer.
    v8::Local<v8::Object> bufferObj1 = 
                Nan::To<v8::Object>(info[1]).ToLocalChecked();
    uint8_t* cipherData = (uint8_t *)node::Buffer::Data(bufferObj1);
    size_t cipherLength = node::Buffer::Length(bufferObj1);


    // Decrypt the given cipher buffer using the given key.
    std::vector<uint8_t> temp;
    try {

        obj->decryptBlock(temp, 
            std::vector<uint8_t>(keyData, keyData + keyLength), 
            std::vector<uint8_t>(cipherData, cipherData + cipherLength)
        );

    } catch (const CryptoPP::Exception& e) {

        // throw an error to node.js in case of decryption errors
        Nan::ThrowError(e.what());
        return;
    }

    // XOR random bytes with the decrypted message buffer.
    std::vector<uint8_t> messageData(temp.size());
    obj->xorRandomData(messageData, temp);

    // Copy messageData vector into a node.js buffer.
    auto slowBuffer = Nan::CopyBuffer((const char*)messageData.data(), 
        messageData.size()).ToLocalChecked();;

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
void AESXOR256::Init(v8::Handle<v8::Object> exports) {

    Nan::HandleScope scope;

    // Prepare constructor template.
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("AESXOR256").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(2);

    // Prototype
    Nan::SetPrototypeMethod(tpl, "encrypt", encrypt);
    Nan::SetPrototypeMethod(tpl, "decrypt", decrypt);
    
    constructor.Reset(tpl->GetFunction());

    // Setting node.js module.exports.
    exports->Set(Nan::New("AESXOR256").ToLocalChecked(), tpl->GetFunction());
}

