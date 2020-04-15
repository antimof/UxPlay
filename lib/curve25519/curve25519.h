#ifndef CURVE25519_DONNA_H
#define CURVE25519_DONNA_H

static const unsigned char kCurve25519BasePoint[32] = { 9 };

int curve25519_donna(unsigned char *mypublic, const unsigned char *secret, const unsigned char *basepoint);

#endif
