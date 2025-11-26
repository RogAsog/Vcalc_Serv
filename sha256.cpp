#include "sha256.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>

static inline uint32_t ROTR(uint32_t x, int n) { return (x >> n) | (x << (32-n)); }
static inline uint32_t SHR(uint32_t x, int n) { return x >> n; }

static const uint32_t k[64] = {
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static inline uint32_t ch(uint32_t x,uint32_t y,uint32_t z){return (x&y) ^ (~x & z);}
static inline uint32_t maj(uint32_t x,uint32_t y,uint32_t z){return (x & y) ^ (x & z) ^ (y & z);}
static inline uint32_t bsig0(uint32_t x){return ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22);}
static inline uint32_t bsig1(uint32_t x){return ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25);}
static inline uint32_t ssig0(uint32_t x){return ROTR(x,7) ^ ROTR(x,18) ^ SHR(x,3);}
static inline uint32_t ssig1(uint32_t x){return ROTR(x,17) ^ ROTR(x,19) ^ SHR(x,10);}

void sha256(const uint8_t *data, size_t len, uint8_t out[32]) {
    uint32_t h[8] = {
        0x6a09e667UL,0xbb67ae85UL,0x3c6ef372UL,0xa54ff53aUL,
        0x510e527fUL,0x9b05688cUL,0x1f83d9abUL,0x5be0cd19UL
    };
    size_t num_blocks = ((len + 9) + 63) / 64;
    uint8_t *msg = (uint8_t*)calloc(num_blocks*64,1);
    memcpy(msg, data, len);
    msg[len] = 0x80;
    uint64_t bit_len = (uint64_t)len * 8;
    uint8_t *p = msg + num_blocks*64 - 8;
    for (int i = 7; i >= 0; i--) { p[i] = bit_len & 0xFF; bit_len >>= 8; }
    for (size_t b = 0; b < num_blocks; b++) {
        uint32_t w[64];
        const uint8_t *chunk = msg + b*64;
        for (int t = 0; t < 16; t++) {
            w[t] = (chunk[t*4] << 24) | (chunk[t*4+1] << 16) | (chunk[t*4+2] << 8) | (chunk[t*4+3]);
        }
        for (int t = 16; t < 64; t++) {
            w[t] = ssig1(w[t-2]) + w[t-7] + ssig0(w[t-15]) + w[t-16];
        }
        uint32_t a=h[0], b0=h[1], c=h[2], d=h[3], e=h[4], f=h[5], g=h[6], hh=h[7];
        for (int t = 0; t < 64; t++) {
            uint32_t T1 = hh + bsig1(e) + ch(e,f,g) + k[t] + w[t];
            uint32_t T2 = bsig0(a) + maj(a,b0,c);
            hh = g; g = f; f = e; e = d + T1; d = c; c = b0; b0 = a; a = T1 + T2;
        }
        h[0]+=a; h[1]+=b0; h[2]+=c; h[3]+=d; h[4]+=e; h[5]+=f; h[6]+=g; h[7]+=hh;
    }
    free(msg);
    for (int i = 0; i < 8; i++) {
        out[i*4] = (h[i] >> 24) & 0xFF;
        out[i*4+1] = (h[i] >> 16) & 0xFF;
        out[i*4+2] = (h[i] >> 8) & 0xFF;
        out[i*4+3] = (h[i]) & 0xFF;
    }
}
