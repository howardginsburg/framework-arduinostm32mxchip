/* system/wolfssl/user_settings.h
 *
 * wolfSSL 5.7.6 build configuration for MXChip AZ3166 (STM32F4 Cortex-M4).
 *
 * This file is picked up automatically when WOLFSSL_USER_SETTINGS is defined
 * at compile time (see platform.txt).  It replaces wolfssl/options.h and
 * selects the features needed for:
 *   - TLS 1.2 + TLS 1.3 client (ECDHE + RSA server auth)
 *   - HMAC-SHA256 (Azure IoT SAS-token generation)
 *   - Base64 encode / decode (key / signature encoding)
 *
 * Target: STM32F412RG — 1 MB flash, 256 KB RAM, ARM Cortex-M4F
 */

#ifndef WOLFSSL_USER_SETTINGS_H
#define WOLFSSL_USER_SETTINGS_H

/* ─────────────────────────────────────────────────────────────────────────── */
/*  Platform / environment                                                    */
/* ─────────────────────────────────────────────────────────────────────────── */
#define WOLFSSL_USER_IO          /* We supply custom I/O callbacks            */
#define NO_FILESYSTEM            /* No POSIX file I/O on bare-metal           */
#define NO_WRITEV                /* No writev() / struct iovec                */
#define NO_WOLFSSL_DIR           /* No opendir / readdir                      */
#define NO_MAIN_DRIVER           /* No main() in test/bench sources           */
#define WOLFSSL_NO_SOCK          /* No BSD socket layer                       */
#define NO_DEV_RANDOM            /* No /dev/urandom                           */
#define SINGLE_THREADED          /* No pthreads; mbed RTOS mutex TBD          */
#define NO_SIG_WRAPPER           /* Smaller sig code                          */
#define WC_NO_ASYNC_THREADING    /* No async threading support                */

/* ─────────────────────────────────────────────────────────────────────────── */
/*  Protocol versions                                                         */
/* ─────────────────────────────────────────────────────────────────────────── */
#define WOLFSSL_TLS13            /* TLS 1.3 support                           */
#define NO_OLD_TLS               /* Disable TLS 1.0 / 1.1                    */
#define WOLFSSL_ALT_CERT_CHAINS  /* Flexible cert chain verify (Azure uses    */
                                 /*   intermediates not in strict order)      */

/* ─────────────────────────────────────────────────────────────────────────── */
/*  Asymmetric crypto                                                         */
/* ─────────────────────────────────────────────────────────────────────────── */
#define HAVE_ECC                 /* ECDHE key exchange + ECDSA verify         */
#define ECC_SHAMIR               /* Faster ECC point multiply                 */
#define HAVE_HKDF                /* Required by TLS 1.3 key schedule          */
#define HAVE_SUPPORTED_CURVES    /* Supported-curves TLS extension            */
#define HAVE_FFDHE_2048          /* Finite-field DHE group (fallback)         */
#define FP_MAX_BITS 4096         /* Max RSA key size we can verify            */
#define WC_RSA_PSS              /* RSA-PSS signatures — required by TLS 1.3  */

/* ─────────────────────────────────────────────────────────────────────────── */
/*  Symmetric / hash / MAC                                                    */
/* ─────────────────────────────────────────────────────────────────────────── */
#define WOLFSSL_SHA256           /* SHA-256                                   */
#define WOLFSSL_SHA384           /* SHA-384 — needed by TLS 1.3              */
#define WOLFSSL_SHA512           /* SHA-512 — needed by some certs           */
#define HAVE_CHACHA              /* ChaCha20 stream cipher                    */
#define HAVE_POLY1305            /* Poly1305 MAC                              */
#define HAVE_AESGCM              /* AES-128/256-GCM cipher suites             */
#define HAVE_AES_CBC             /* AES-CBC (fallback)                        */
#define WOLFSSL_AES_COUNTER      /* AES-CTR mode                              */

/* ─────────────────────────────────────────────────────────────────────────── */
/*  TLS extensions                                                            */
/* ─────────────────────────────────────────────────────────────────────────── */
#define HAVE_TLS_EXTENSIONS      /* Required for SNI, ALPN, etc.             */
#define HAVE_SNI                 /* Server Name Indication                    */

/* ─────────────────────────────────────────────────────────────────────────── */
/*  Build-mode: client-only                                                   */
/* ─────────────────────────────────────────────────────────────────────────── */
#define NO_WOLFSSL_SERVER        /* Strip all server-side code                */
#define KEEP_PEER_CERT           /* Retain peer cert for post-handshake info  */

/* ─────────────────────────────────────────────────────────────────────────── */
/*  Disable algorithms we do not use                                          */
/* ─────────────────────────────────────────────────────────────────────────── */
#define NO_DSA                   /* DSA signatures                            */
#define NO_RC4                   /* RC4 stream cipher                         */
#define NO_HC128                 /* HC-128 stream cipher                      */
#define NO_RABBIT                /* Rabbit stream cipher                      */
#define NO_MD4                   /* MD4 hash                                  */
#define NO_DES3                  /* Triple-DES                                */
#define NO_PSK                   /* Pre-shared key suites                     */
#define NO_PWDBASED              /* PBKDF1/2 — not used                      */

/* ─────────────────────────────────────────────────────────────────────────── */
/*  Math library — single-precision for Cortex-M                              */
/* ─────────────────────────────────────────────────────────────────────────── */
#define WOLFSSL_SP_MATH_ALL      /* Use SP math for all operations            */
#define WOLFSSL_SP_SMALL         /* Smaller (slower) SP tables                */
#define SP_WORD_SIZE 32          /* 32-bit target                             */
#define WOLFSSL_HAVE_SP_ECC      /* SP implementation of ECC                  */
#define WOLFSSL_HAVE_SP_RSA      /* SP implementation of RSA verify           */
#define WOLFSSL_SP_ARM_CORTEX_M_ASM /* Cortex-M assembly optimisations       */
#define WOLFSSL_SP_ASM           /* Enable SP assembly                        */

/* ─────────────────────────────────────────────────────────────────────────── */
/*  Flash/RAM reduction                                                       */
/* ─────────────────────────────────────────────────────────────────────────── */
#define WOLFSSL_SMALL_STACK      /* Allocate large buffers on heap            */
#define SMALL_SESSION_CACHE      /* Smaller session-ticket cache              */
#define NO_SESSION_CACHE_REF     /* No session-cache ref counting             */
#define GCM_SMALL                /* Smaller AES-GCM lookup tables             */
#define RSA_LOW_MEM              /* Smaller RSA verify tables                 */
#define WOLFSSL_AES_SMALL_TABLES /* Smaller AES S-box tables                  */
#define USE_SLOW_SHA             /* Smaller SHA-1 (speed for size)            */
#define USE_SLOW_SHA256          /* Smaller SHA-256                           */
#define USE_SLOW_SHA512          /* Smaller SHA-384/512                       */
#define ALT_ECC_SIZE             /* Reduce ECC struct sizes                   */
#define ECC_USER_CURVES          /* Only include curves we need               */
#define HAVE_ECC256              /* P-256 (required)                          */
#define HAVE_ECC384              /* P-384                                     */
#define WOLFSSL_MAX_ERROR_SZ 80  /* Short error-string buffer                 */
#define NO_ERROR_STRINGS         /* Remove error string table (saves ~4 KB)   */

/* ─────────────────────────────────────────────────────────────────────────── */
/*  RNG — no hardware RNG on this board; use RTOS time + ISR entropy          */
/*  For production, implement wc_GenerateSeed() in wc_port.c callbacks.       */
/*  WOLFSSL_GENSEED_FORTEST allows builds to succeed; replace with a real     */
/*  entropy source before deployment.                                         */
/* ─────────────────────────────────────────────────────────────────────────── */
#define WOLFSSL_GENSEED_FORTEST  /* TODO: replace with HW RNG / TRNG hook    */

/* ─────────────────────────────────────────────────────────────────────────── */
/*  Base64 — required by AzureIoTCrypto.cpp                                   */
/* ─────────────────────────────────────────────────────────────────────────── */
#define WOLFSSL_BASE64_ENCODE    /* Enable Base64_Encode / Base64_Encode_NoNl */

#endif /* WOLFSSL_USER_SETTINGS_H */
