#ifndef _ENCRYPT_WRAPPER_H_
#define _ENCRYPT_WRAPPER_H_
#include <stdio.h>

//api:ssl 建立
typedef void *(*ssl_establish_t)(const char *host, unsigned short port, const char *ca_crt, size_t ca_crt_len);
//api:ssl 销毁
typedef int (*ssl_destroy_t)(void ** handle);
//api:ssl 写数据
typedef int (*ssl_write_t)(void * handle, const char *buf, int len, int timeout_ms);
//api:ssl 读数据
typedef int (*ssl_read_t)(void * handle, char *buf, int len, int timeout_ms);

typedef void *(*md5_init_t)(void);
typedef int (*md5_append_t)(void *hdl, unsigned char *data, unsigned int size);
typedef int (*md5_finish_t)(void *hdl, unsigned char *digest);

typedef void *(*aes_init_t)(const unsigned char *key, unsigned int keybits);
typedef void *(*aes_dec_init_t)(const unsigned char *key, unsigned int keybits);
typedef int (*aes_crypt_cbc_t)(void *ctx, int mode, size_t length, unsigned char iv[16], const unsigned char *input, unsigned char *output);
typedef int (*aes_finish_t)(void *ctx);

typedef int (*ras256_sign_t)(unsigned char *data, size_t data_len, unsigned char *privateKey, unsigned char **sig, size_t *sig_len);
typedef int (*ras256_verify_t)(unsigned char *data, size_t data_len, unsigned char *sig, unsigned char *publicKey);

typedef int (*base64_decode_t)(unsigned char *dst, size_t dlen, size_t *olen, const unsigned char *src, size_t slen);
typedef int (*base64_encode_t)(unsigned char *dst, size_t dlen, size_t *olen, const unsigned char *src, size_t slen);

typedef int (*hmac_sha_t)(int mbedtls_md_type, const char *key, int keylen, const char *content, int contentlen, char *out);


typedef struct {
	ssl_establish_t ssl_establish;
	ssl_destroy_t ssl_destroy;
	ssl_write_t ssl_write;
	ssl_read_t ssl_read;

	md5_init_t md5_init;
	md5_append_t md5_append;
	md5_finish_t md5_finish;
	
	aes_init_t aes_init;
    aes_dec_init_t aes_dec_init;
	aes_crypt_cbc_t aes_crypt_cbc;
	aes_finish_t aes_finish;

	ras256_sign_t ras256_sign;
	ras256_verify_t ras256_verify;
	
	base64_decode_t base64_decode;
	base64_encode_t base64_encode;

	hmac_sha_t hmac_sha;
} encrypt_wrapper_t;

#endif

