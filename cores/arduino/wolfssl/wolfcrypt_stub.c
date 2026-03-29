/*
 * cores/arduino/wolfssl/wolfcrypt_stub.c
 *
 * Stub implementations of the wolfCrypt HMAC-SHA256 and Base64 APIs used
 * by AzureIoTCrypto.cpp for SAS token generation and key derivation.
 *
 * PURPOSE
 * -------
 * Allows the framework to compile and link without the real wolfSSL/wolfCrypt
 * source tree.  The HMAC and Base64 implementations here are correct software
 * implementations (so the Azure IoT authentication path actually works at
 * runtime) but they do NOT benefit from wolfSSL's performance optimisations
 * or hardware-acceleration hooks.
 *
 * REPLACING WITH REAL wolfSSL
 * ---------------------------
 * See cores/arduino/wolfssl/wolfssl_tls_stub.c for full instructions.
 * Once real wolfSSL sources are in place, delete this file and adjust
 * platform.txt to compile wolfcrypt/src/hmac.c and wolfcrypt/src/coding.c
 * from the vendored wolfSSL tree.
 */

#include "wolfssl/wolfcrypt/hmac.h"
#include "wolfssl/wolfcrypt/coding.h"
#include <string.h>
#include <stdlib.h>

/* ── Portable SHA-256 implementation ─────────────────────────────────────── */
/*
 * This is a minimal, self-contained SHA-256 used only by the HMAC stub.
 * It is replaced by wolfCrypt's optimised SHA-256 when real wolfSSL sources
 * are vendored.
 */

#define SHA256_BLOCK_SIZE  64
#define SHA256_DIGEST_SIZE 32

typedef struct {
    uint32_t state[8];
    uint32_t count[2];
    uint8_t  buf[SHA256_BLOCK_SIZE];
} SHA256_CTX;

/* Ensure the inner-state slot in Hmac is large enough for SHA256_CTX. */
typedef char _hmac_inner_size_check[
    (HMAC_INNER_CTX_SIZE >= (int)sizeof(SHA256_CTX)) ? 1 : -1];

static const uint32_t K256[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,
    0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
    0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
    0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,
    0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
    0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,
    0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,
    0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
    0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

#define ROTR32(x,n) (((x)>>(n))|((x)<<(32-(n))))
#define CH(e,f,g)   (((e)&(f))^(~(e)&(g)))
#define MAJ(a,b,c)  (((a)&(b))^((a)&(c))^((b)&(c)))
#define SIG0(a) (ROTR32(a,2)^ROTR32(a,13)^ROTR32(a,22))
#define SIG1(e) (ROTR32(e,6)^ROTR32(e,11)^ROTR32(e,25))
#define sig0(x) (ROTR32(x,7)^ROTR32(x,18)^((x)>>3))
#define sig1(x) (ROTR32(x,17)^ROTR32(x,19)^((x)>>10))

static void sha256_transform(SHA256_CTX *ctx, const uint8_t *data)
{
    uint32_t W[64], a,b,c,d,e,f,g,h,T1,T2;
    int i;
    for (i = 0; i < 16; i++)
        W[i] = ((uint32_t)data[i*4]<<24)|((uint32_t)data[i*4+1]<<16)
              |((uint32_t)data[i*4+2]<<8)|(uint32_t)data[i*4+3];
    for (i = 16; i < 64; i++)
        W[i] = sig1(W[i-2])+W[i-7]+sig0(W[i-15])+W[i-16];
    a=ctx->state[0]; b=ctx->state[1]; c=ctx->state[2]; d=ctx->state[3];
    e=ctx->state[4]; f=ctx->state[5]; g=ctx->state[6]; h=ctx->state[7];
    for (i = 0; i < 64; i++) {
        T1 = h+SIG1(e)+CH(e,f,g)+K256[i]+W[i];
        T2 = SIG0(a)+MAJ(a,b,c);
        h=g; g=f; f=e; e=d+T1; d=c; c=b; b=a; a=T1+T2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void sha256_init(SHA256_CTX *ctx)
{
    ctx->state[0]=0x6a09e667; ctx->state[1]=0xbb67ae85;
    ctx->state[2]=0x3c6ef372; ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f; ctx->state[5]=0x9b05688c;
    ctx->state[6]=0x1f83d9ab; ctx->state[7]=0x5be0cd19;
    ctx->count[0] = ctx->count[1] = 0;
}

static void sha256_update(SHA256_CTX *ctx, const uint8_t *data, size_t len)
{
    uint32_t lo = ctx->count[0];
    if ((ctx->count[0] += (uint32_t)(len << 3)) < lo)
        ctx->count[1]++;
    ctx->count[1] += (uint32_t)(len >> 29);

    size_t idx = (lo >> 3) & 0x3f;
    size_t part = SHA256_BLOCK_SIZE - idx;
    size_t i = 0;
    if (len >= part) {
        memcpy(&ctx->buf[idx], data, part);
        sha256_transform(ctx, ctx->buf);
        for (i = part; i + SHA256_BLOCK_SIZE <= len; i += SHA256_BLOCK_SIZE)
            sha256_transform(ctx, data + i);
        idx = 0;
    }
    memcpy(&ctx->buf[idx], data + i, len - i);
}

static void sha256_final(SHA256_CTX *ctx, uint8_t *digest)
{
    uint8_t bits[8];
    uint32_t idx, pad;
    for (int i = 0; i < 4; i++) {
        bits[i]   = (ctx->count[1] >> (24 - i*8)) & 0xff;
        bits[i+4] = (ctx->count[0] >> (24 - i*8)) & 0xff;
    }
    idx = (ctx->count[0] >> 3) & 0x3f;
    pad = (idx < 56) ? (56 - idx) : (120 - idx);
    static const uint8_t PADDING[64] = { 0x80 };
    sha256_update(ctx, PADDING, pad);
    sha256_update(ctx, bits, 8);
    for (int i = 0; i < 8; i++) {
        digest[i*4]   = (ctx->state[i] >> 24) & 0xff;
        digest[i*4+1] = (ctx->state[i] >> 16) & 0xff;
        digest[i*4+2] = (ctx->state[i] >>  8) & 0xff;
        digest[i*4+3] =  ctx->state[i]        & 0xff;
    }
}

/* ── HMAC-SHA256 ──────────────────────────────────────────────────────────── */

int wc_HmacInit(Hmac* hmac, void* heap, int devId)
{
    (void)heap; (void)devId;
    if (!hmac) return -1;
    memset(hmac, 0, sizeof(*hmac));
    return 0;
}

int wc_HmacSetKey(Hmac* hmac, int type, const byte* key, word32 keySz)
{
    if (!hmac || type != WC_SHA256) return -1;
    hmac->macType = type;

    /* Derive ipad / opad keys. */
    uint8_t k[HMAC_BLOCK_SIZE];
    memset(k, 0, sizeof(k));
    if (keySz > HMAC_BLOCK_SIZE) {
        /* Hash the key if it's longer than one block. */
        SHA256_CTX sha;
        sha256_init(&sha);
        sha256_update(&sha, key, keySz);
        sha256_final(&sha, k);
    } else {
        memcpy(k, key, keySz);
    }
    for (word32 i = 0; i < HMAC_BLOCK_SIZE; i++) {
        hmac->ipad[i] = k[i] ^ 0x36;
        hmac->opad[i] = k[i] ^ 0x5c;
    }

    /* Initialise inner hash with K⊕ipad so wc_HmacUpdate feeds data in. */
    SHA256_CTX *inner_sha = (SHA256_CTX *)hmac->inner;
    sha256_init(inner_sha);
    sha256_update(inner_sha, hmac->ipad, HMAC_BLOCK_SIZE);
    return 0;
}

/* Feed data into the running inner hash. */
int wc_HmacUpdate(Hmac* hmac, const byte* in, word32 sz)
{
    if (!hmac || !in) return -1;
    SHA256_CTX *inner_sha = (SHA256_CTX *)hmac->inner;
    sha256_update(inner_sha, in, sz);
    return 0;
}

int wc_HmacFinal(Hmac* hmac, byte* hash)
{
    if (!hmac || !hash) return -1;

    /* Finish inner hash. */
    SHA256_CTX *inner_sha = (SHA256_CTX *)hmac->inner;
    uint8_t inner[SHA256_DIGEST_SIZE];
    sha256_final(inner_sha, inner);

    /* Outer hash: H( K⊕opad || inner ) */
    SHA256_CTX outer_sha;
    sha256_init(&outer_sha);
    sha256_update(&outer_sha, hmac->opad, HMAC_BLOCK_SIZE);
    sha256_update(&outer_sha, inner, SHA256_DIGEST_SIZE);
    sha256_final(&outer_sha, hash);

    return 0;
}

void wc_HmacFree(Hmac* hmac)
{
    if (hmac)
        memset(hmac, 0, sizeof(*hmac));
}

/* ── Base64 encode / decode ───────────────────────────────────────────────── */

static const char B64_TABLE[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int Base64_Encode_NoNl(const byte* in, word32 inLen, byte* out, word32* outLen)
{
    if (!in || !out || !outLen) return -1;
    word32 needed = ((inLen + 2) / 3) * 4 + 1;
    if (*outLen < needed) { *outLen = needed; return -1; }

    word32 i, o = 0;
    for (i = 0; i < inLen; i += 3)
    {
        int rem = (int)inLen - (int)i;   /* bytes left in this group (1–3) */
        uint32_t b = (uint32_t)in[i] << 16;
        if (rem > 1) b |= (uint32_t)in[i+1] << 8;
        if (rem > 2) b |= (uint32_t)in[i+2];

        out[o++] = (byte)B64_TABLE[(b >> 18) & 0x3f];
        out[o++] = (byte)B64_TABLE[(b >> 12) & 0x3f];
        out[o++] = (rem > 1) ? (byte)B64_TABLE[(b >> 6) & 0x3f] : '=';
        out[o++] = (rem > 2) ? (byte)B64_TABLE[ b       & 0x3f] : '=';
    }
    out[o] = '\0';
    *outLen = o;
    return 0;
}

int Base64_Encode(const byte* in, word32 inLen, byte* out, word32* outLen)
{
    /* wolfSSL's Base64_Encode adds a newline every 64 output characters. */
    if (!in || !out || !outLen) return -1;
    word32 rawLen = ((inLen + 2) / 3) * 4;
    word32 needed = rawLen + (rawLen / 64) + 2;
    if (*outLen < needed) { *outLen = needed; return -1; }

    word32 i, o = 0, col = 0;
    for (i = 0; i < inLen; i += 3)
    {
        int rem = (int)inLen - (int)i;
        uint32_t b = (uint32_t)in[i] << 16;
        if (rem > 1) b |= (uint32_t)in[i+1] << 8;
        if (rem > 2) b |= (uint32_t)in[i+2];

        out[o++] = (byte)B64_TABLE[(b >> 18) & 0x3f];
        out[o++] = (byte)B64_TABLE[(b >> 12) & 0x3f];
        out[o++] = (rem > 1) ? (byte)B64_TABLE[(b >> 6) & 0x3f] : '=';
        out[o++] = (rem > 2) ? (byte)B64_TABLE[ b       & 0x3f] : '=';
        col += 4;
        if (col >= 64) { out[o++] = '\n'; col = 0; }
    }
    out[o++] = '\n';
    out[o]   = '\0';
    *outLen  = o;
    return 0;
}

static int b64_val(uint8_t c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1; /* skip whitespace / padding */
}

int Base64_Decode(const byte* in, word32 inLen, byte* out, word32* outLen)
{
    if (!in || !out || !outLen) return -1;
    word32 o = 0;
    int    buf = 0, bits = 0;

    for (word32 i = 0; i < inLen; i++)
    {
        if (out && o >= *outLen) return -1; /* output buffer too small */
        int v = b64_val(in[i]);
        if (v < 0) continue; /* ignore whitespace and '=' */
        buf  = (buf << 6) | v;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out[o++] = (byte)((buf >> bits) & 0xff);
        }
    }
    *outLen = o;
    return 0;
}
