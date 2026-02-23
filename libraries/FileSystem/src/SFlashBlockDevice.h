// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

// SFlashBlockDevice is now part of the framework core.
// This file re-exports it for backward compatibility with user sketches.
//
// The canonical implementation lives in:
//   cores/arduino/SFlashBlockDevice.[h/cpp]
//
// Angle-bracket include resolves to cores/arduino/ via -I{build.core.path},
// bypassing this file to avoid a circular self-include.

#ifndef SFLASH_BLOCK_DEVICE_LIB_WRAPPER_H
#define SFLASH_BLOCK_DEVICE_LIB_WRAPPER_H

#include <SFlashBlockDevice.h>

#endif /* SFLASH_BLOCK_DEVICE_LIB_WRAPPER_H */
