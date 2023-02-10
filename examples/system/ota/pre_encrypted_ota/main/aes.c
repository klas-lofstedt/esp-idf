#include "mbedtls/aes.h"
#include "esp_log.h"
#include <string.h>
#include "aes.h"
#include "nvs.h"

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
void aes_ctr_crypto(char *input, unsigned char *output)
{
    const unsigned char *const_input = (const unsigned char *)input;
    size_t input_len = strlen((const char *)input);
    ESP_LOGI(TAG, "input_len %d", input_len);

    size_t nc_off = 0;
    unsigned char nonce_counter[16] = {0};
    unsigned char stream_block[16] = {0};
    mbedtls_aes_crypt_ctr(&aes, input_len, &nc_off, nonce_counter, stream_block, const_input, output);
    //ESP_LOGI(TAG, "AES KEY: ");
    //ESP_LOG_BUFFER_HEX(TAG, key, sizeof(key));
}

// void aes_ctr_crypto(char *encrypted_string, char *decrypted_string, size_t size)
// {
//     char encrypted_u8[size];
//     unsigned char decrypted_u8[size];
//     //unsigned char decrypted_string[ctxt->om->om_len / 2 ];
//     ESP_LOGI(TAG, "encrypted_string: ");
//     ESP_LOG_BUFFER_HEX(TAG, encrypted_string, sizeof(encrypted_string));

//     u8_array_from_hex_string(encrypted_string, encrypted_u8, size);
//     ESP_LOGI(TAG, "encrypted_u8: ");
//     ESP_LOG_BUFFER_HEX(TAG, encrypted_u8, sizeof(encrypted_u8));

//     //nvs_get_aes_key_str(aes_key);
//     const unsigned char *input = (const unsigned char *)encrypted_u8;
//     size_t input_size = strlen((const char *)encrypted_u8);

//     size_t nc_off = 0;
//     unsigned char nonce_counter[16] = {0};
//     unsigned char stream_block[16] = {0};
//     mbedtls_aes_crypt_ctr(&aes, input_size, &nc_off, nonce_counter, stream_block, input, decrypted_u8);
//     ESP_LOGI(TAG, "decrypted_u8: ");
//     ESP_LOG_BUFFER_HEX(TAG, decrypted_u8, sizeof(decrypted_u8));
//     ascii_string_from_u8_array(decrypted_u8, decrypted_string, size);
//     ESP_LOGI(TAG, "decrypted_string: %s", decrypted_string);
//     ESP_LOG_BUFFER_HEX(TAG, decrypted_string, sizeof(decrypted_string));

//     //ESP_LOGI(TAG, "AES KEY: ");
//     //ESP_LOG_BUFFER_HEX(TAG, key, sizeof(key));
// }

