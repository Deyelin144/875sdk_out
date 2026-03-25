#ifndef _REALIZE_UNIT_ENCRYPT_H_
#define _REALIZE_UNIT_ENCRYPT_H_
#include <stdio.h>

/*****************************************************************************/
/***********************************ssl api***********************************/
/*****************************************************************************/
void *realize_unit_encrypt_ssl_establish(const char *host, unsigned short port, const char *ca_crt, size_t ca_crt_len);
int realize_unit_encrypt_ssl_destroy(void ** handle);
int realize_unit_encrypt_ssl_write(void * handle, const char *buf, int len, int timeout_ms);
int realize_unit_encrypt_ssl_read(void * handle, char *buf, int len, int timeout_ms);


/*****************************************************************************/
/***********************************md5 api***********************************/
/*****************************************************************************/
void *realize_unit_encrypt_md5_init(void);
int realize_unit_encrypt_md5_append(void *hdl, unsigned char *data, unsigned int size);
int realize_unit_encrypt_md5_finish(void *hdl, unsigned char *digest);

/*****************************************************************************/
/******************aes api, 目前只需要支持cbc模式*****************************/
/*****************************************************************************/
#define ENCRYPT_UNIT_AES_ENCRYPT 1
#define ENCRYPT_UNIT_AES_DECRYPT 0

void *realize_unit_encrypt_aes_init(const unsigned char *key, unsigned int keybits);
int realize_unit_encrypt_aes_crypt_cbc(void *ctx, int mode, size_t length, unsigned char iv[16], const unsigned char *input, unsigned char *output);
int realize_unit_encrypt_aes_finish(void *ctx);
void *realize_unit_decrypt_aes_init(const unsigned char *key, unsigned int keybits);
int realize_unit_evp_bytes_to_key(int key_len, int iv_len, unsigned char *salt, unsigned char *data, int count, unsigned char *key, unsigned char *iv);

/*****************************************************************************/
/******************rsa api *****************************/
/*****************************************************************************/
// RSA256 signing function
int realize_unit_encrypt_rsa_sign(unsigned char *data, size_t data_len, unsigned char *privateKey, unsigned char **sig, size_t *sig_len);

// RSA256 verification function  
int realize_unit_encrypt_rsa_verify(unsigned char *data, size_t data_len, unsigned char *sig, unsigned char *publicKey);

/*****************************************************************************/
/*********************************base64 api**********************************/
/*****************************************************************************/
int realize_unit_encrypt_base64_decode(unsigned char *dst, size_t dlen, size_t *olen, const unsigned char *src, size_t slen);
int realize_unit_encrypt_base64_encode(unsigned char *dst, size_t dlen, size_t *olen, const unsigned char *src, size_t slen);

/*****************************************************************************/
/*******************************hmac-sha1加密*********************************/
/*****************************************************************************/
int realize_unit_encrypt_hmac_sha(int mbedtls_md_type, const char *key, int keylen, const char *content, int contentlen, char *out);

/*****************************************************************************/
/**********************************应用接口***********************************/
/*****************************************************************************/
int realize_unit_encrypt_file_md5(const char *filename, char *outmd5);
int realize_unit_encrypt_aes_cbc_to_nstr(const char *encrypt_data, const char *key, const char *iv, char **outbuf);
int realize_unit_encrypt_aes_cbc_to_base64(const char *encrypt_data, const char *key, const char *iv, char **outbuf);

#endif