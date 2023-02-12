/*
 * aes.h
 *
 *  Created on: 1 feb 2023
 *      Author: klaslofstedt
 */

#ifndef _AES_H_
#define _AES_H_

#define AES_KEY_SIZE            16

void aes_ctr_init();
void aes_ctr_deinit();
//void aes_ctr_crypto(char *input, unsigned char *output);
void aes_ctr_crypto(char *encrypted_string, char *decrypted_string, size_t size);


#endif /* _AES_H_ */
