/* system/wolfssl/user_settings.h
 *
 * Minimal wolfSSL build configuration for MXChip AZ3166 (STM32F4 Cortex-M4).
 *
 * This file is picked up automatically when WOLFSSL_USER_SETTINGS is defined
 * at compile time (see platform.txt).  It replaces wolfssl/options.h for
 * on-device builds and selects only the primitives actually used:
 *   - TLS 1.2 + TLS 1.3 client
 *   - HMAC-SHA256 (SAS-token generation)
 *   - Base64 encode / decode (key / signature encoding)
 *
 * When vendoring the real wolfSSL sources (see README.md), drop them into
 * system/wolfssl/ and remove the stub sources from cores/arduino/wolfssl/.
 */

#ifndef WOLFSSL_USER_SETTINGS_H
#define WOLFSSL_USER_SETTINGS_H

/* ── Custom I/O ───────────────────────────────────────────────────────────── */
/* We supply wolfssl_recv / wolfssl_send callbacks; disable BSD-socket layer. */
#define WOLFSSL_USER_IO

/* ── Protocol versions ────────────────────────────────────────────────────── */
#define WOLFSSL_TLS13
/* TLS 1.0 / 1.1 support is intentionally omitted. */
#define NO_OLD_TLS

/* ── Asymmetric crypto ────────────────────────────────────────────────────── */
#define HAVE_ECC
#define HAVE_HKDF           /* Required by TLS 1.3 */

/* ── Symmetric / hash ─────────────────────────────────────────────────────── */
#define WOLFSSL_SHA256
#define HAVE_CHACHA
#define HAVE_POLY1305

/* ── TLS extensions ───────────────────────────────────────────────────────── */
#define HAVE_TLS_EXTENSIONS /* Required for SNI, ALPN, etc. */
#define HAVE_SNI

/* ── Miscellaneous ────────────────────────────────────────────────────────── */
#define WOLFSSL_MAX_ERROR_SZ 80
#define NO_FILESYSTEM        /* No POSIX filesystem on bare-metal target */
#define NO_WOLFSSL_SERVER    /* Client-only build */
#define SINGLE_THREADED      /* mbed RTOS mutex support can be added later */

/* Memory reduction for embedded targets */
#define SMALL_SESSION_CACHE
#define NO_SESSION_CACHE_REF

#endif /* WOLFSSL_USER_SETTINGS_H */
