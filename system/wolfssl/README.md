# system/wolfssl — wolfSSL 5.7.6 integration

This directory contains the build configuration and **real wolfSSL headers**
for the wolfSSL TLS library used by `TLSSocket` and `AzureIoTCrypto`.

## Version

**wolfSSL 5.7.6** (GPLv2+) — vendored from https://github.com/wolfSSL/wolfssl

## Directory layout

```
system/wolfssl/
├── user_settings.h              # Build configuration (WOLFSSL_USER_SETTINGS)
├── README.md                    # This file
└── include/
    └── wolfssl/                 # Real wolfSSL 5.7.6 header tree
        ├── ssl.h
        ├── internal.h
        ├── error-ssl.h
        ├── wolfio.h
        └── wolfcrypt/
            ├── settings.h
            ├── types.h
            ├── hmac.h
            ├── coding.h
            ├── aes.h
            ├── ecc.h
            ├── sha256.h
            └── ... (all wolfcrypt headers)

cores/arduino/wolfssl/
├── src/                         # wolfSSL TLS layer (.c source files)
│   ├── ssl.c
│   ├── tls.c
│   ├── tls13.c
│   ├── internal.c
│   └── ...
└── wolfcrypt/
    └── src/                     # wolfCrypt primitives (.c source files)
        ├── aes.c
        ├── asn.c
        ├── ecc.c
        ├── hmac.c
        ├── sha256.c
        ├── coding.c
        └── ...
```

## How it works

- `user_settings.h` is activated by `-DWOLFSSL_USER_SETTINGS` in `platform.txt`
- Headers live in `system/wolfssl/include/` (on the compiler's `-I` path)
- Source files live in `cores/arduino/wolfssl/` and are auto-compiled by the
  Arduino/PlatformIO build system
- `cores/arduino/wolfssl/` is also on the `-I` path so wolfSSL's internal
  `#include <wolfcrypt/src/misc.c>` resolves correctly

## Build configuration

See `user_settings.h` for the full list of enabled features. Key points:

- **TLS 1.2 + TLS 1.3** client (no server)
- **ECDHE + RSA** key exchange / server authentication
- **AES-GCM**, **ChaCha20-Poly1305** cipher suites
- **HMAC-SHA256** (for Azure IoT SAS tokens)
- **Base64** encode/decode (for key/signature encoding)
- **SP math** with Cortex-M assembly optimisations
- Aggressive flash reduction: small tables, no error strings, no unused algorithms

## Why wolfSSL?

The pre-compiled system binaries (`libdevkit-sdk-core-lib.a`, `libstsafe.a`)
use mbedTLS internally — those symbols are still satisfied by the mbedTLS
headers in `system/mbed-os/features/mbedtls/`. The framework's own open-source
code (TLSSocket, AzureIoTCrypto) uses wolfSSL exclusively, providing:

- TLS 1.3 support
- No closed-source constraints on user code
- Full auditability and modifiability
- GPLv2+ licence (compatible with open-source projects)

## Updating wolfSSL

To update to a newer wolfSSL release:

1. Clone the new tag: `git clone --depth 1 --branch v5.x.y https://github.com/wolfSSL/wolfssl.git`
2. Replace `system/wolfssl/include/wolfssl/` with the new header tree
3. Replace `cores/arduino/wolfssl/src/` with the new `src/*.c` files
4. Replace `cores/arduino/wolfssl/wolfcrypt/src/` with the new `wolfcrypt/src/*.c` files
5. Test all build profiles: `pio ci tests/build_check.cpp --board=mxchip_az3166 ...`
6. Update this README with the new version number
