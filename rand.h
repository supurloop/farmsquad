#ifndef RAND_H_INCLUDE
#define RAND_H_INCLUDE

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

extern const uint8_t randSeq[256];
extern uint8_t ri;
extern uint8_t rs;
#define rf() (randSeq[ri++] ^ rs)
#define rfv(r) (r = rf())

#ifdef __cplusplus
}
#endif

#endif /* RAND_H_INCLUDE */
