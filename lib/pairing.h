/**
 *  Copyright (C) 2018  Juho Vähä-Herttua
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 */

#ifndef PAIRING_H
#define PAIRING_H

typedef struct pairing_s pairing_t;
typedef struct pairing_session_s pairing_session_t;

pairing_t *pairing_init_generate();
pairing_t *pairing_init_seed(const unsigned char seed[32]);
void pairing_get_public_key(pairing_t *pairing, unsigned char public_key[32]);

pairing_session_t *pairing_session_init(pairing_t *pairing);
void pairing_session_set_setup_status(pairing_session_t *session);
int pairing_session_check_handshake_status(pairing_session_t *session);
int pairing_session_handshake(pairing_session_t *session, const unsigned char ecdh_key[32], const unsigned char ed_key[32]);
int pairing_session_get_public_key(pairing_session_t *session, unsigned char ecdh_key[32]);
int pairing_session_get_signature(pairing_session_t *session, unsigned char signature[64]);
int pairing_session_finish(pairing_session_t *session, const unsigned char signature[64]);
void pairing_session_destroy(pairing_session_t *session);

void pairing_destroy(pairing_t *pairing);

void pairing_get_ecdh_secret_key(pairing_session_t *session, unsigned char ecdh_secret[32]);

#endif
