# system/wolfssl — wolfSSL integration directory

This directory contains the build configuration and stub headers for the
wolfSSL TLS library used by `TLSSocket` and `AzureIoTCrypto`.

## Current state: stubs

The `include/wolfssl/` tree contains **minimal stub headers** that define
only the types and function signatures needed to compile and link the
framework.  The actual implementations live in
`cores/arduino/wolfssl/wolfssl_tls_stub.c` and
`cores/arduino/wolfssl/wolfcrypt_stub.c`.

These stubs are sufficient for the CI build verification and for
`AzureIoTCrypto` (HMAC-SHA256 / Base64) which is implemented in pure C
inside the stub.  The TLS handshake stub in `wolfssl_tls_stub.c` does **not**
perform real TLS — it passes data through unencrypted.

## Upgrading to real wolfSSL

To replace the stubs with the genuine wolfSSL library:

### 1. Download wolfSSL 5.x

```bash
git clone https://github.com/wolfSSL/wolfssl.git --depth 1 --branch v5.7.0-stable
```

Or download a release archive from https://www.wolfssl.com/download/

### 2. Configure wolfSSL for a minimal embedded client build

```bash
cd wolfssl
./autogen.sh
./configure \
  --enable-tls13 \
  --enable-ecc \
  --enable-hkdf \
  --enable-sni \
  --enable-base64encode \
  --disable-examples \
  --disable-filesystem \
  --disable-oldtls
```

For bare-metal builds, use `WOLFSSL_USER_SETTINGS` and
`system/wolfssl/user_settings.h` instead of `./configure`.

### 3. Copy sources into the framework

```
system/wolfssl/wolfssl-5.x/
├── wolfssl/          (header tree)
│   ├── ssl.h
│   ├── wolfcrypt/
│   │   ├── hmac.h
│   │   ├── coding.h
│   │   └── ...
│   └── ...
└── src/              (wolfSSL .c sources compiled by platform.txt)
    ├── ssl.c
    ├── tls.c
    ├── tls13.c
    ├── wolfcrypt/src/hmac.c
    ├── wolfcrypt/src/sha256.c
    ├── wolfcrypt/src/coding.c
    └── ...
```

Update `platform.txt` to compile sources from `{build.system.path}/wolfssl/wolfssl-5.x/src/`
and add `{build.system.path}/wolfssl/wolfssl-5.x/` to the include path.

### 4. Remove the stubs

Delete the following stub files once real wolfSSL is in place:

```
cores/arduino/wolfssl/wolfssl_tls_stub.c
cores/arduino/wolfssl/wolfcrypt_stub.c
system/wolfssl/include/wolfssl/ssl.h
system/wolfssl/include/wolfssl/wolfcrypt/hmac.h
system/wolfssl/include/wolfssl/wolfcrypt/coding.h
system/wolfssl/include/wolfssl/wolfcrypt/types.h
```

`system/wolfssl/user_settings.h` should be **kept** — wolfSSL reads it when
`WOLFSSL_USER_SETTINGS` is defined (set in `platform.txt`).

## Why wolfSSL?

The pre-compiled system binaries (`libdevkit-sdk-core-lib.a`, `libstsafe.a`)
use mbedTLS internally — those symbols are still satisfied by the mbedTLS
headers in `system/mbed-os/features/mbedtls/`.  The framework's own
open-source code (TLSSocket, AzureIoTCrypto) now uses wolfSSL exclusively,
which:

- provides TLS 1.3 support
- carries no closed-source constraints on the user's code
- can be audited / modified freely
