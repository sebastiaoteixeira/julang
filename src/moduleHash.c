#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symbolsNaming.h"
#include "token.h"
#if defined(__APPLE__)
#  define COMMON_DIGEST_FOR_OPENSSL
#  include <CommonCrypto/CommonDigest.h>
#  define SHA1 CC_SHA1
#else
#  include <openssl/evp.h>
#endif

#define MD5_DIGEST_LENGTH EVP_MD_size(EVP_md5())

unsigned char* namingHash(char* input) {
    EVP_MD_CTX *mdctx;
    unsigned char *md5_digest;
    unsigned int md5_digest_len = EVP_MD_size(EVP_md5());

    // MD5_Init
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);

    // MD5_Update
    EVP_DigestUpdate(mdctx, input, strlen(input));

    // MD5_Final
    md5_digest = (unsigned char *)OPENSSL_malloc(md5_digest_len);
    EVP_DigestFinal_ex(mdctx, md5_digest, &md5_digest_len);
    EVP_MD_CTX_free(mdctx);

    return md5_digest;
}

char* extractTLMText(Token* tl, char* text) {
    text[0] = '\0';
    unsigned int length = 1;
    unsigned int capacity = 1024;
    for (int i = 0; tl[i].type != EOF; i++) {
        length += strlen(tl[i].text);
        if (length >= capacity) {
            capacity <<= 1;
            text = (char*)realloc(text, sizeof(char) * capacity);
        }
        strcat(text, tl[i].text);
    }
    text[length] = '\0';
    return text;
}

// Adapted base64 character set to valid symbol names
char base64[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_.";


char* base64Encode(unsigned char* hash, char* encoded) {
    int length = 0;
    const int a1 = 0b11111100;
    const int b1 = 0b00000011;
    const int b2 = 0b11110000;
    const int c1 = 0b00001111;
    const int c2 = 0b11000000;
    const int d1 = 0b00111111;

    for (int i = 0; i < MD5_DIGEST_LENGTH; i+=3) {
        length += 2;

        __u_char a = hash[i];
        __u_char b = i < MD5_DIGEST_LENGTH - 1 ? hash[i+1] : 0;
        __u_char c = i < MD5_DIGEST_LENGTH - 2 ? hash[i+2] : 0;

        __u_char a_ = (a & a1) >> 2;
        __u_char b_ = ((a & b1) << 4) | ((b & b2) >> 4);
        encoded[length-2] = base64[a_];
        encoded[length-1] = base64[b_];
        if (i < MD5_DIGEST_LENGTH - 1) {
            length++;

            __u_char c_ = ((b & c1) << 2) | ((c & c2) >> 6);
            encoded[length-1] = base64[c_];
            
            if (i < MD5_DIGEST_LENGTH - 2) {
                length++;

                __u_char d_ = c & d1;
                encoded[length-1] = base64[d_];
            }
        }
    }
    encoded[length] = '\0';

    return encoded;
}

char* generateSymbolHash(char* input, char* encoded) {
    unsigned char* hash_ = namingHash(input);
    base64Encode(hash_, encoded);
    return encoded;
}

char* generateParametersHash(short* input, unsigned int length) {
    char* text = (char*) malloc(sizeof(char) * (length + 1));
    for (int i = 0; i < length; i++) {
        text[i] = (char) (input[i] & 0xFF);
    }
    text[length] = '\0';
    char* encoded = (char*) malloc(sizeof(char) * (1 + MD5_DIGEST_LENGTH * 2));
    encoded[0] = '_';
    generateSymbolHash(text, encoded + 1);
    free(text);
    return encoded;
}

char* generateModuleHash(Token* input) {
    char* text = (char*) malloc(sizeof(char) * 1024);
    extractTLMText(input, text);
    char* encoded = (char*) malloc(sizeof(char) * ((MD5_DIGEST_LENGTH * 4 / 3) + 3));
    generateSymbolHash(text, encoded);
    free(text);
    return encoded;
}
