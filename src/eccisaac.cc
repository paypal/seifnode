/** @file eccisaac.cc
 *  @brief Definition of the class functions provided in eccisaac.h and helper
 *         functions to print the public/private ECC keys
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
#include <exception>
#include <mutex>

// ----------------------
// node.js addon includes
// ----------------------
#include <node_buffer.h>

// -----------------
// cryptopp includes
// -----------------
#include <cryptopp/hex.h>

#include <cryptopp/filters.h>
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::ArraySink;
using CryptoPP::ArraySource;
using CryptoPP::PK_EncryptorFilter;
using CryptoPP::PK_DecryptorFilter;

#include <cryptopp/pubkey.h>
using CryptoPP::PublicKey;
using CryptoPP::PrivateKey;

#include <cryptopp/eccrypto.h>
using CryptoPP::ECP; 
using CryptoPP::ECIES;
using CryptoPP::ECPPoint;
using CryptoPP::DL_GroupParameters_EC;
using CryptoPP::DL_FixedBasePrecomputation;

#include <cryptopp/pubkey.h>
using CryptoPP::DL_PrivateKey_EC;
using CryptoPP::DL_PublicKey_EC;

#include <cryptopp/asn.h>
#include <cryptopp/oids.h>
namespace ASN1 = CryptoPP::ASN1;

#include <cryptopp/cryptlib.h>

#include <cryptopp/sha3.h>
using CryptoPP::SHA3_256;

#include <cryptopp/osrng.h>
using CryptoPP::AutoSeededRandomPool;


// ----------------
// library includes
// ----------------
#include "eccisaac.h"
#include "util.h"

namespace {  
    // Strings represnting names of files to be stored to the disk.
    // rng state file name
    const std::string RNG_STATE_FILE_NAME = ".ecies.rng"; 
    // private key file name
    const std::string PRIV_KEY_FILE_NAME = ".ecies.private.key";
    // public key file name
    const std::string PUB_KEY_FILE_NAME = ".ecies.public.key";
}

// javascript object constructor
Nan::Persistent<v8::Function> ECCISAAC::constructor; 

// Helper functions for printing the public and private keys.
void PrintPrivateKey(const DL_PrivateKey_EC<ECP>& key, 
    std::ostream& out = std::cout);
void PrintPublicKey(const DL_PublicKey_EC<ECP>& key, 
    std::ostream& out = std::cout);


// ---------------
// PrintPrivateKey
// ---------------
/**
 * @brief Prints the private key components
 *
 * @param key private key object
 * @param out output stram
 * @params flags output stream flags
 *
 * @return void
 */
void PrintPrivateKey(const DL_PrivateKey_EC<ECP>& key, std::ostream& out) {

    const std::ios_base::fmtflags flags = out.flags();
    
    // Group parameters
    const DL_GroupParameters_EC<ECP>& params = key.GetGroupParameters();
    // Base precomputation
    const DL_FixedBasePrecomputation<ECPPoint>& bpc = 
        params.GetBasePrecomputation();
    // Public Key (just do the exponentiation)
    const ECPPoint point = bpc.Exponentiate(params.GetGroupPrecomputation(), 
        key.GetPrivateExponent());
    
    out << "Modulus: " << std::hex << 
        params.GetCurve().GetField().GetModulus() << std::endl;
    out << "Cofactor: " << std::hex << params.GetCofactor() << std::endl;
    
    out << "Coefficients" << std::endl;
    out << "  A: " << std::hex << params.GetCurve().GetA() << std::endl;
    out << "  B: " << std::hex << params.GetCurve().GetB() << std::endl;
    
    out << "Base Point" << std::endl;
    out << "  x: " << std::hex << params.GetSubgroupGenerator().x << std::endl;
    out << "  y: " << std::hex << params.GetSubgroupGenerator().y << std::endl;
    
    out << "Public Point" << std::endl;
    out << "  x: " << std::hex << point.x << std::endl;
    out << "  y: " << std::hex << point.y << std::endl;
    
    out << "Private Exponent (multiplicand): " << std::endl;
    out << "  " << std::hex << key.GetPrivateExponent() << std::endl;

    out << std::endl;
    out.flags(flags);
}


// --------------
// PrintPublicKey
// --------------
/**
 * @brief Prints the public key components.
 *
 * @param key public key object
 * @param out output stram
 * @params flags output stream flags
 *
 * @return void
 */
void PrintPublicKey(const DL_PublicKey_EC<ECP>& key, std::ostream& out)
{
    const std::ios_base::fmtflags flags = out.flags();
    
    // Group parameters
    const DL_GroupParameters_EC<ECP>& params = key.GetGroupParameters();
    // Public key
    const ECPPoint& point = key.GetPublicElement();
    
    out << "Modulus: " << std::hex << 
        params.GetCurve().GetField().GetModulus() << std::endl;
    out << "Cofactor: " << std::hex << params.GetCofactor() << std::endl;
    
    out << "Coefficients" << std::endl;
    out << "  A: " << std::hex << params.GetCurve().GetA() << std::endl;
    out << "  B: " << std::hex << params.GetCurve().GetB() << std::endl;
    
    out << "Base Point" << std::endl;
    out << "  x: " << std::hex << params.GetSubgroupGenerator().x << std::endl;
    out << "  y: " << std::hex << params.GetSubgroupGenerator().y << std::endl;
    
    out << "Public Point" << std::endl;
    out << "  x: " << std::hex << point.x << std::endl;
    out << "  y: " << std::hex << point.y << std::endl;

    out << std::endl;
    out.flags(flags);
}



// -----------
// Constructor
// -----------
/**
 * Constructor
 * @brief Initilizes and constructs internal data.
 *
 * @param initCallback callback to be invoked after async 
 *        operation 
 * @param key disk access key for public/private keys and 
 *        rng state 
 * @param folderPath folder containing keys and rng state files
 */
ECCISAAC::Worker::Worker(
    Nan::Callback* initCallback, 
    // IsaacRandomPool* prng,
    const std::vector<uint8_t>& key, 
    const std::string& folderPath
): Nan::AsyncWorker(initCallback), 
_wfolderPath(folderPath),
_wkey(key) {

}



// ----------------
// HandleOKCallback
// ----------------
/**
 * @brief Executed when the async work is complete without
 *        error, this function will be run inside the main event  
 *        loop, invoking the given callback with the loaded 
 *        keys as an argument.
 *
 * The keys are returned as the second argument to the callback  
 * {enc: [publicKey], dec: [privateKey]}
 *
 * @return void
 */
void ECCISAAC::Worker::HandleOKCallback () {
    Nan::HandleScope scope;

    /* Creating js status object with 'code' set as the status code(0) and 
     * 'message' as "Success".
     */
    v8::Local<v8::Object> status = Nan::New<v8::Object>();
    Nan::Set(status, 
        Nan::New<v8::String>("code").ToLocalChecked(), 
        Nan::New<v8::Integer>(0)
    );
    Nan::Set(status, 
        Nan::New<v8::String>("message").ToLocalChecked(), 
        Nan::New<v8::String>("Success").ToLocalChecked()
    );

    /* Creating js object with 'enc' set as the public key and 
     * 'dec' as the private key.
     */
    v8::Local<v8::Object> ret = Nan::New<v8::Object>();
    Nan::Set(ret, 
        Nan::New<v8::String>("enc").ToLocalChecked(), 
        Nan::New<v8::String>(_encodedPub).ToLocalChecked()
    );
    Nan::Set(ret, 
        Nan::New<v8::String>("dec").ToLocalChecked(), 
        Nan::New<v8::String>(_encodedPriv).ToLocalChecked()
    );

    // Invoking given callback with an undefined error and keys object.
    v8::Local<v8::Value> argv[] = {status, ret};

    callback->Call(2, argv);
}



// -------------------
// HandleErrorCallback
// -------------------
/**
 * @brief Executed when the async work is complete with
 *        error, this function will be run inside the main event  
 *        loop, invoking the given callback with the 
 *        corresponding error.
 *
 * The error is returned as the first argument to the callback 
 * {code: [statusCode], message: [errorMessage]}
 *
 * @return void
 */
void ECCISAAC::Worker::HandleErrorCallback () {
    Nan::HandleScope scope;

    /* Creating js error object with 'code' set as the status code and 
     * 'message' as the error message.
     */
    v8::Local<v8::Object> error = Nan::New<v8::Object>();
    Nan::Set(error, 
        Nan::New<v8::String>("code").ToLocalChecked(), 
        Nan::New<v8::Integer>((int)_status)
    );
    Nan::Set(error, 
        Nan::New<v8::String>("message").ToLocalChecked(), 
        Nan::New<v8::String>(ErrorMessage()).ToLocalChecked()
    );

    // Invoke callback with the above error object and undefined keys.
    v8::Local<v8::Value> argv[] = {error, Nan::Undefined()};
    
    callback->Call(2, argv);
}



// -------
// Execute
// -------
/**
 * @brief Executed in a separate thread, asynchronously loading 
 *        the public/private keys from the disk and 
 *        communicating the status of the operation via the 
 *        Worker class.
 *
 * @return void
 */
void ECCISAAC::Worker::Execute() {
                    
    try {
        // Try to load keys from the disk into the string arguments.
        _status = ECCISAAC::loadKeys(
            _encodedPub, 
            _encodedPriv, 
            _wkey, 
            _wfolderPath
        );
        
        if (_status == STATUS::SUCCESS) {
            return;
        } 

        // In case of error set the error message with appropriate message.
        if (_status == STATUS::FILE_NOT_FOUND) {
            SetErrorMessage("File Not Found");
        } else if (_status == STATUS::DECRYPTION_ERROR) {
            SetErrorMessage("Decryption Error");
        } else {
            SetErrorMessage("Unknown Error");
        }
        return;

    } catch (...) { 
        // Exception thrown while loading keys.
        SetErrorMessage("Unknown Error");
        return;
    }

}


// -----------
// Constructor
// -----------
/**
 * Constructor
 * @brief Initilizes and constructs internal data.
 *
 * @param keyData byte vector corresponding to disk access key 
 * @param folderPath folder containing the encrypted keys
 */
ECCISAAC::ECCISAAC(
    const std::vector<uint8_t>& keyData, 
    const std::string& folderPath
): _key(keyData), _folderPath(folderPath) {

}



// --------------
// SavePrivateKey
// --------------
/**
 * @brief Encrypt and save the private key to the disk with the given 
 *        file name.
 *
 * @param privateKey private key object
 * @param file file name of encrypted private key
 * @param key disk access key for public/private keys and 
 *        rng state 
 * @param folderPath folder containing keys and rng state files
 *
 * @return void
 */
void ECCISAAC::SavePrivateKey(
    const PrivateKey& privateKey, 
    const std::string& file, 
    const std::vector<uint8_t>& key, 
    const std::string& folderPath
)
{
    // Create file encryptor object.
    FileCryptopp fileEncryptor(folderPath + file);

    // Save the private key into a string using CryptoPP StringSink.
    std::string keyStr;
    StringSink keySink(keyStr);

    privateKey.Save(keySink);

    // Write the string form of the private key to the file.
    std::stringstream fss(keyStr);
    fileEncryptor.writeFile(fss, key);
}


// -------------
// SavePublicKey
// -------------
/**
 * @brief Encrypt and save the public key to the disk with the given 
 *        file name.
 *
 * @param publicKey public key object
 * @param file file name of encrypted public key
 * @param key disk access key for public/private keys and 
 *        rng state 
 * @param folderPath folder containing keys and rng state files
 *
 * @return void
 */
void ECCISAAC::SavePublicKey(
    const PublicKey& publicKey, 
    const std::string& file, 
    const std::vector<uint8_t>& key, 
    const std::string& folderPath
)
{
    // Create file encryptor object.
    FileCryptopp fileEncryptor(folderPath + file);

    // Save the public key into a string using CryptoPP StringSink.
    std::string keyStr;
    StringSink keySink(keyStr);

    publicKey.Save(keySink);

    // Write the string form of the public key to the file.
    std::stringstream fss(keyStr);
    fileEncryptor.writeFile(fss, key);
}



// --------------
// LoadPrivateKey
// --------------
/**
 * @brief Decrypt the file with the given name and load it as the 
 *        private key.
 *
 * @param privateKey private key object
 * @param file file name of encrypted private key
 * @param key disk access key for public/private keys and 
 *        rng state 
 * @param folderPath folder containing keys and rng state files
 *
 * @return status code indicating success or cause of error
 */
ECCISAAC::STATUS ECCISAAC::LoadPrivateKey(
    PrivateKey& privateKey, 
    const std::string& file, 
    const std::vector<uint8_t>& key, 
    const std::string& folderPath
) 
{
    // Create file decryptor object.
    FileCryptopp fileDecryptor(folderPath + file);

    // If private key file does not exist then return appropriate error.
    if (!fileDecryptor.fileExists()) {
        return ECCISAAC::STATUS::FILE_NOT_FOUND;
    }

    // Read the encrypted file to get the private key string.
    std::stringstream fss;
    if (!fileDecryptor.readFile(fss, key)) {
        // If decryption fails return an error.
        return ECCISAAC::STATUS::DECRYPTION_ERROR;
    }

    std::string keyStr = fss.str();

    // Use CryptoPP StringSource to get the private key object from the string.
    StringSource keySource(keyStr, true);

    privateKey.Load(keySource);

    return ECCISAAC::STATUS::SUCCESS;
}


// -------------
// LoadPublicKey
// -------------
/**
 * @brief Decrypt the file with the given name and load it as the 
 *        public key.
 *
 * @param publicKey public key object
 * @param file file name of encrypted public key
 * @param key disk access key for public/private keys and 
 *        rng state 
 * @param folderPath folder containing keys and rng state files
 *
 * @return status code indicating success or cause of error
 */
ECCISAAC::STATUS ECCISAAC::LoadPublicKey(
    PublicKey& publicKey, 
    const std::string& file, 
    const std::vector<uint8_t>& key, 
    const std::string& folderPath
) 
{
    // Create file decryptor object.
    FileCryptopp fileDecryptor(folderPath + file);

    // If public key file does not exist then return appropriate error.
    if (!fileDecryptor.fileExists()) {
        return ECCISAAC::STATUS::FILE_NOT_FOUND;
    }

    // Read the encrypted file to get the public key string.
    std::stringstream fss;
    if (!fileDecryptor.readFile(fss, key)) {
        // If decryption fails return an error.
        return ECCISAAC::STATUS::DECRYPTION_ERROR;
    }

    std::string keyStr = fss.str();

    // Use CryptoPP StringSource to get the public key object from the string.
    StringSource keySource(keyStr, true);
    
    publicKey.Load(keySource);

    return ECCISAAC::STATUS::SUCCESS;
}



// --------
// loadKeys
// --------
/**
 * @brief Loads the keys by decrypting existing files and initializes 
 *        the random number generator.
 *
 * @param encodedPub public key to be loaded from encrypted file
 * @param encodedPriv private key to be loaded from encrypted file
 * @param key disk access key for public/private keys and 
 *        rng state 
 * @param folderPath folder containing keys and rng state files
 *
 * @return status code indicating success or cause of error
 */
ECCISAAC::STATUS ECCISAAC::loadKeys(
    std::string& encodedPub, 
    std::string& encodedPriv, 
    const std::vector<uint8_t>& key, 
    const std::string& folderPath
) 
{
    // ECC Decryption object containing the private key.
    ECIES<ECP>::Decryptor d0;
    
    // Load the private key from encrypted file on disk.
    ECCISAAC::STATUS rc;
    if ((rc = LoadPrivateKey(d0.AccessPrivateKey(), PRIV_KEY_FILE_NAME, 
                             key, folderPath)) 
        != ECCISAAC::STATUS::SUCCESS) {
        /* If loading the key fails return the error which could be due to 
         * the file not being present on the disk or due to a decryption error.
         */
        return rc;
    }

    // ECC Decryption object containing the public key
    ECIES<ECP>::Encryptor e0;

    // Load the public key from encrypted file on disk.
    if ((rc = LoadPublicKey(e0.AccessPublicKey(), PUB_KEY_FILE_NAME, 
                            key, folderPath)) 
        != ECCISAAC::STATUS::SUCCESS) {
        /* If loading the key fails return the error which could be due to 
         * the file not being present on the disk or due to a decryption error.
         */
        return rc;
    }  

    /* Get the string versions of the keys from the encryption and decryption 
     * objects using CryptoPP StringSink.
     */
    std::string pubStr, privStr;
    StringSink pubSs(pubStr), privSs(privStr);
    e0.GetPublicKey().Save(pubSs);
    d0.GetPrivateKey().Save(privSs);

    // Hex encode the string keys using CryptoPP StringSource and HexEncoder.
    StringSource ss1(pubStr, true, 
        new CryptoPP::HexEncoder(new StringSink(encodedPub)));
 
    StringSource ss2(privStr, true, 
        new CryptoPP::HexEncoder(new StringSink(encodedPriv)));

    // Hash the hex encoded private key string using CryptoPP SHA3_256.
    std::vector<uint8_t> digest(CryptoPP::SHA3_256::DIGESTSIZE);
    hashString(digest, encodedPriv);
    
    // Using the default file name for the RNG saved state.
    std::string fileName = RNG_STATE_FILE_NAME;

    std::string fileId = folderPath + fileName;

    return ECCISAAC::STATUS::SUCCESS;
}


// ------------
// generateKeys
// ------------
/**
 * @brief Initialize the RNG and use it to generate the public and 
 *        private keys, encrypt them and save to disk.
 *
 * @param encodedPub public key to be generated
 * @param encodedPriv private key to be generated
 * @param key disk access key for public/private keys and 
 *        rng state 
 * @param folderPath folder containing keys and rng state files
 *
 * @return boolean indicating success/failure
 */
bool ECCISAAC::generateKeys(
    std::string& encodedPub, 
    std::string& encodedPriv, 
    const std::vector<uint8_t>& key, 
    const std::string& folderPath
) 
{
    // Using the default file name for the RNG saved state.
    std::string fileName = RNG_STATE_FILE_NAME;

    std::string fileId = folderPath + fileName;

    IsaacRandomPool prng;
    
    /* Initialize the global Isaac rng object and check if 
     * initialization succeeded. if it fails, increase the multiplier argument 
     * which causes more data to be collected to get higher entropy.
     */
    int multiplier = 0;

    try {
        for (; multiplier < 6; ++multiplier) {
            if (prng.Initialize(fileId, multiplier)) {
                break;
            }
        }
    } catch (const std::exception& ex) {

        // If there is any hardware error, catch and throw the error to node.js
        Nan::ThrowError(ex.what());
        return false;
    }

    // If initialization fails after max retries, throw an error to node.js.
    if (multiplier == 6) {
        Nan::ThrowError("Not enough entropy!");
        return false;
    }

    // ECC Decryption object created using our Isaac RNG and secp521r1 curve.
    ECIES<ECP>::Decryptor d0(prng, CryptoPP::ASN1::secp521r1());

    // ECC Encryption object corresponding to the above decryptor object.
    ECIES<ECP>::Encryptor e0(d0);

    /* Generate the private and public keys using our Isaac RNG and 
     * save them to encrypted files on the disk in the given folder.
     */
    try {
        d0.GetPrivateKey().ThrowIfInvalid(prng, 3);
        e0.GetPublicKey().ThrowIfInvalid(prng, 3);
        SavePrivateKey(d0.GetPrivateKey(), PRIV_KEY_FILE_NAME, key, folderPath);
        SavePublicKey(e0.GetPublicKey(), PUB_KEY_FILE_NAME, key, folderPath);
    } catch (...) {
        return false;
    }

    // Get the string versions of the public and private keys using StringSink.
    std::string pubStr, privStr;
    StringSink pubSs(pubStr), privSs(privStr);
    e0.GetPublicKey().Save(pubSs);
    d0.GetPrivateKey().Save(privSs);

    // Hex encode the string keys using CryptoPP StringSource and HexEncoder.
    StringSource ss1(pubStr, true, 
        new CryptoPP::HexEncoder(new StringSink(encodedPub)));

    StringSource ss2(privStr, true, 
        new CryptoPP::HexEncoder(new StringSink(encodedPriv)));

    return true;
}





// ---
// New
// ---
/**
 * @brief Creates the node object and corresponding underlying object 
 *        with provided arguments - disk access key and folder 
 *        containing the encrypted keys.
 *
 * Invoked as:
 * 'var obj = new ECCISAAC(diskKey, folder)' or 
 * 'var obj = ECCISAAC(diskKey, folder)' where
 * 'diskKey' is the key used to encrypt the keys and rng state
 * 'folder' is the folder where the keys and rng state are saved on disk
 *
 * @param info node.js arguments wrapper containing the disk access key 
 *        and folder path
 *
 * @return 
 */
NAN_METHOD(ECCISAAC::New) {

    if (info.IsConstructCall()) {

        // Invoked as constructor: 'var obj = new ECCISAAC()'.

        // Check arguments.
        if (!node::Buffer::HasInstance(info[0])) {

            Nan::ThrowError("Incorrect Arguments. Disk access key buffer "
                            "not provided");
            return;
        }

        /* Unwrap the first argument to get the key buffer used to store 
         * RNG state to the disk.
         */
        v8::Local<v8::Object> bufferObj = 
            Nan::To<v8::Object>(info[0]).ToLocalChecked();
        
        uint8_t* bufferData = (uint8_t*)node::Buffer::Data(bufferObj);
        size_t bufferLength = node::Buffer::Length(bufferObj);

        /* If the size of key buffer is less than AES key size then hash the  
         * given data to get key of the required size.
         */
        std::vector<uint8_t> digest;
        if (bufferLength < 32) {
            digest.resize(CryptoPP::SHA3_256::DIGESTSIZE);
            std::string bufferString(reinterpret_cast<const char*>(bufferData), 
                reinterpret_cast<const char*>(bufferData) + bufferLength); 
            hashString(digest, bufferString);

        } else {
            digest.reserve(CryptoPP::SHA3_256::DIGESTSIZE);
            std::copy(bufferData, bufferData + bufferLength, 
                std::back_inserter(digest));
        }

        /* Unwrap the second argument to get the folder on disk where the RNG 
         * state and keys will be encrypted and stored.
         */
        std::string folder = "./";
        if (!info[1]->IsUndefined()) {
            v8::String::Utf8Value str(info[1]->ToString());
            folder = *str;
            if (folder.back() != '/') {
                folder = folder + "/";
            }
        }

        // Create the wrapped object using the disk access key and given folder.
        ECCISAAC* obj = new ECCISAAC(digest, folder);

        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());

    } else {

        // Invoked as plain function `ECCISAAC(...)`, turn into construct call.
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




// --------
// loadKeys
// --------
/**
 * @brief Creates an async worker to load keys from the disk and 
 *        invokes the callback function with the error object (if 
 *        applicable) and/or the object containing the keys.
 *
 * Invoked as:
 * 'obj.loadKeys(function(status, keys){})' where
 * 'status' (if applicable) is of the form: 
 * {code: [statusCode], message: [statusMessage]}
 * 'keys' (if available) is of the form:
 * {enc: [publicKey], dec: [privateKey]}
 *
 * @param info node.js arguments wrapper
 *
 * @return void
 */
NAN_METHOD(ECCISAAC::loadKeys) {

    // Get a reference to the wrapped object from the argument.
    ECCISAAC* obj = ObjectWrap::Unwrap<ECCISAAC>(info.Holder());

    // Unwrap the first argument to get given callback function.
    Nan::Callback *callback = new Nan::Callback(info[0].As<v8::Function>());

    // Initialize the async worker and queue it.
    Worker* worker = new Worker(
        callback, 
        obj->_key, 
        obj->_folderPath
    );

    Nan::AsyncQueueWorker(worker);
}



// ------------
// generateKeys
// ------------
/**
 * @brief Initializes the isaac RNG and uses it to generate the 
 *        public/private keys and return them to the caller.
 *
 * Invoked as:
 * 'var keys = obj.generateKeys()' where
 * 'keys' (if available) is of the form:
 * {enc: [publicKey], dec: [privateKey]}
 *
 * @param info node.js arguments wrapper
 *
 * @return void
 */
NAN_METHOD(ECCISAAC::generateKeys) {

    // Get a reference to the wrapped object from the argument.
    ECCISAAC* obj = ObjectWrap::Unwrap<ECCISAAC>(info.Holder());

    // Generate the public and private keys as strings and save them to disk.
    std::string encodedPub, encodedPriv;
    if (!obj->generateKeys(
            encodedPub, 
            encodedPriv,
            obj->_key, 
            obj->_folderPath
        )
    ) {

        return;
    }

    /* Creating js object with 'enc' set as the public key and 
     * 'dec' as the private key.
     */
    v8::Local<v8::Object> ret = Nan::New<v8::Object>();
    Nan::Set(ret, 
        Nan::New<v8::String>("enc").ToLocalChecked(), 
        Nan::New<v8::String>(encodedPub).ToLocalChecked());
    Nan::Set(ret, 
        Nan::New<v8::String>("dec").ToLocalChecked(), 
        Nan::New<v8::String>(encodedPriv).ToLocalChecked());

    // Set the above object as the value to be returned to node.js.
    info.GetReturnValue().Set(ret);
}



// -------
// encrypt
// -------
/**
 * @brief Unwraps the arguments to get the public key and message and 
 *        encrypt the message using the public key to return the cipher.
 *
 * Invoked as:
 * 'var cipher = obj.encrypt(key, message)' 
 * 'key' is the string containing the hex encoded ECC public key
 * 'message' is the buffer containing the message to be encrypted
 * 'cipher' is the string containing the encrypted cipher
 *
 * @param info node.js arguments wrapper containing public key string 
 *        and message
 *
 * @return void
 */
NAN_METHOD(ECCISAAC::encrypt) {

    // Check arguments.
    if (info[0]->IsUndefined()) {
        Nan::ThrowError("Incorrect Arguments. Missing Public key string");
        return;
    }

    if (!node::Buffer::HasInstance(info[1])) {

        Nan::ThrowError("Incorrect Arguments. Message buffer not provided");
        return;
    }

    ECCISAAC* obj = ObjectWrap::Unwrap<ECCISAAC>(info.Holder());
    
    // Unwrap the first argument to get the hex encoded public key string.
    v8::String::Utf8Value str(info[0]->ToString());
    std::string pubStr(*str);

    // Unwrap the second argument to get the message buffer to be encrypted.
    v8::Local<v8::Object> bufferObj = 
            Nan::To<v8::Object>(info[1]).ToLocalChecked();
        
    uint8_t* messageData = (uint8_t*)node::Buffer::Data(bufferObj);
    size_t messageLength = node::Buffer::Length(bufferObj);

    // string containing hex encoded encrypted cipher
    std::string encoded;

    try {
        /* Hex decode the string to get the public key string using 
         * CryptoPP StringSource and HexDecoder and store in 'em'.
         */
        std::string em;
        StringSource ss0(pubStr, true, 
            new CryptoPP::HexDecoder(new StringSink(em)));

        /* This decoded string can now be converted to public key object wrapped 
         * in the ECC encryption object using StringSource.
         */
        ECIES<ECP>::Encryptor e1;
        StringSource ss(em, true);
        e1.AccessPublicKey().Load(ss);

        AutoSeededRandomPool prng;
    
        /* Apply the CryptoPP PK_EncryptorFilter transformer to encrypt the  
         * message buffer; store the result in a string 'em0' using ArraySource.
         */
        std::string em0;
        ArraySource ss1 (messageData, messageLength, true, 
            new PK_EncryptorFilter(prng, e1, new StringSink(em0) ) );

        // Hex encode the string cipher using CryptoPP StringSource & HexEncoder
        StringSource ss6(em0, true, 
            new CryptoPP::HexEncoder(new StringSink(encoded)));

    } catch (const std::exception& ex) {
        Nan::ThrowError(ex.what());
        return;
    } catch (...) {
        Nan::ThrowError("Unknown error while encrypting message");
        return;
    }

    // Set the encoded cipher output as the value to be returned to node.js.
    info.GetReturnValue().Set(
        Nan::New<v8::String>(encoded.c_str()).ToLocalChecked());
}


// -------
// decrypt
// -------
/**
 * @brief Unwraps the arguments to get private key and cipher and  
 *        decrypt the cipher using the private key to return the 
 *        original message.
 *
 * Invoked as:
 * 'var message = obj.decrypt(key, cipher)' 
 * 'key' is the string containing the hex encoded ECC private key
 * 'cipher' is the string containing the cipher to be decrypted
 * 'message' is the buffer containing the decrypted message
 *
 * @param info node.js arguments wrapper containing private key string 
 *        and cipher.
 *
 * @return void
 */
NAN_METHOD(ECCISAAC::decrypt) {

    // Check arguments.
    if (info[0]->IsUndefined()) {
        Nan::ThrowError("Incorrect Arguments. Missing Public key string");
        return;
    }
    
    if (info[1]->IsUndefined()) {
        Nan::ThrowError("Incorrect Arguments. Missing encrypted cipher string");
        return;
    }

    ECCISAAC* obj = ObjectWrap::Unwrap<ECCISAAC>(info.Holder());

    // Unwrap the first argument to get the hex encoded private key string.
    v8::String::Utf8Value str(info[0]->ToString());
    std::string privStr(*str);

    // Unwrap the second argument to get the hex encoded cipher string.
    v8::String::Utf8Value cipherStr(info[1]->ToString());
    std::string encodedCipher(*cipherStr);

    // string containing decrypted string message
    std::string dm0;
    
    try {
    
        /* Hex decode the string to get the private key string using 
         * CryptoPP StringSource and HexDecoder and store it in string 'em'.
         */
        std::string em;
        StringSource ss0(privStr, true, 
            new CryptoPP::HexDecoder(new StringSink(em)));

        /* This decoded string can now be converted to private key object  
         * wrapped in the ECC decryption object using StringSource.
         */
        ECIES<ECP>::Decryptor d1;
        StringSource ss(em, true);
        d1.AccessPrivateKey().Load(ss);
        
        /* Hex decode the string to get the cipher string ('em0') to be 
         * decrypted using CryptoPP StringSource and HexDecoder.
         */
        std::string em0;
        StringSource ss6(encodedCipher, true, 
            new CryptoPP::HexDecoder(new StringSink(em0)));

        AutoSeededRandomPool prng;
    
        /* Apply the CryptoPP PK_DecryptorFilter transformer to decrypt the  
         * cipher string and store the result in a string using StringSource.
         */
        StringSource ss2 (em0, true, 
            new PK_DecryptorFilter(prng, d1, new StringSink(dm0) ) );

    } catch (const std::exception& ex) {

        Nan::ThrowError(ex.what());
        return;

    } catch (...) {

        Nan::ThrowError("Unknown error while encrypting message");
        return;
    }

    // Copy the decrypted string message into a node.js buffer.
    auto slowBuffer = Nan::CopyBuffer((const char*)dm0.data(), 
        dm0.size()).ToLocalChecked();

    // Set the buffer as the value to returned to node.js.
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
void ECCISAAC::Init(v8::Handle<v8::Object> exports) {

    Nan::HandleScope scope;

    // Prepare constructor template.
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("ECCISAAC").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(4);

    // Prototype
    Nan::SetPrototypeMethod(tpl, "loadKeys", loadKeys);
    Nan::SetPrototypeMethod(tpl, "generateKeys", generateKeys);
    Nan::SetPrototypeMethod(tpl, "encrypt", encrypt);
    Nan::SetPrototypeMethod(tpl, "decrypt", decrypt);
    
    constructor.Reset(tpl->GetFunction());

    // Setting node.js module.exports.
    exports->Set(Nan::New("ECCISAAC").ToLocalChecked(), tpl->GetFunction());
}


