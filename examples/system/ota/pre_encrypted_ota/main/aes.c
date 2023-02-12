#include "mbedtls/aes.h"
#include "esp_log.h"
#include <string.h>
#include "aes.h"
#include "nvs.h"
#include "misc.h"

static const char *TAG = "AES";

mbedtls_aes_context aes;
unsigned char key[AES_KEY_SIZE];// = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
unsigned char key2[AES_KEY_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

void aes_ctr_init()
{
    uint8_t nvs_aes_key[AES_KEY_SIZE];
    nvs_get_aes_key_arr(nvs_aes_key);
    memcpy(key, nvs_aes_key, AES_KEY_SIZE);
    // TODO initialise the key here with a generated 128bit
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, key2, AES_KEY_SIZE * 8);
}

void aes_ctr_deinit()
{
    mbedtls_aes_free(&aes);
}

void aes_ctr_crypto(char *encrypted_string, char *decrypted_string, size_t size)
{
    char encrypted_u8[size];
    unsigned char decrypted_u8[size];
    u8_array_from_hex_string(encrypted_string, encrypted_u8, size);
    const unsigned char *input = (const unsigned char *)encrypted_u8;
    size_t nc_off = 0;
    unsigned char nonce_counter[16] = {0};
    unsigned char stream_block[16] = {0};
    mbedtls_aes_crypt_ctr(&aes, size, &nc_off, nonce_counter, stream_block, input, decrypted_u8);
    ascii_string_from_u8_array(decrypted_u8, decrypted_string, size);
}

