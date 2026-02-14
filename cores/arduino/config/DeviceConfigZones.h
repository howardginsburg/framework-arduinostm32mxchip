// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

/**
 * @file DeviceConfigZones.h
 * @brief Internal header for STSAFE zone definitions and mapping macros
 * 
 * This file defines the hardware zone sizes for the STSAFE secure element
 * and provides helper macros for defining profile zone mappings.
 * 
 * This is an internal header - do not include directly in application code.
 */

#ifndef __DEVICE_CONFIG_ZONES_H__
#define __DEVICE_CONFIG_ZONES_H__

#include "DeviceConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// STSAFE Zone Sizes (Hardware Constants)
// =============================================================================

#define ZONE_0_SIZE  976   // Large zone for certificates
#define ZONE_2_SIZE  192   // Medium zone
#define ZONE_3_SIZE  120   // WiFi SSID
#define ZONE_5_SIZE  584   // URLs, connection strings
#define ZONE_6_SIZE  680   // Device ID, certificates
#define ZONE_7_SIZE  784   // Certificates, keys
#define ZONE_8_SIZE  880   // Certificates, keys
#define ZONE_10_SIZE 88    // WiFi password

// =============================================================================
// Zone Mapping Helper Macros
// =============================================================================

/**
 * @brief Marker for unused zones in profile mappings (all zones = 0xFF)
 */
#define UNUSED_ZONE {{0xFF, 0xFF, 0xFF}, {0, 0, 0}}

/**
 * @brief Single-zone mapping
 * @param z Zone index
 * @param s Zone size
 */
#define ZONE(z, s) {{z, 0xFF, 0xFF}, {s, 0, 0}}

/**
 * @brief Two-zone mapping (for larger certificates)
 * @param z1 First zone index
 * @param s1 First zone size
 * @param z2 Second zone index
 * @param s2 Second zone size
 */
#define ZONE2(z1, s1, z2, s2) {{z1, z2, 0xFF}, {s1, s2, 0}}

/**
 * @brief Three-zone mapping (for largest certificates/keys)
 * @param z1 First zone index
 * @param s1 First zone size
 * @param z2 Second zone index
 * @param s2 Second zone size
 * @param z3 Third zone index
 * @param s3 Third zone size
 */
#define ZONE3(z1, s1, z2, s2, z3, s3) {{z1, z2, z3}, {s1, s2, s3}}

// =============================================================================
// Combined Buffer Sizes (for runtime buffers)
// =============================================================================

/** Maximum CA certificate size (zones 0+7+8) */
#define MAX_CA_CERT_SIZE (ZONE_0_SIZE + ZONE_7_SIZE + ZONE_8_SIZE)

/** Maximum client certificate size (zones 6+7) */
#define MAX_CLIENT_CERT_SIZE (ZONE_6_SIZE + ZONE_7_SIZE)

/** Maximum device certificate size (zones 0+7+8) */
#define MAX_DEVICE_CERT_SIZE (ZONE_0_SIZE + ZONE_7_SIZE + ZONE_8_SIZE)

/** Maximum client key size */
#define MAX_CLIENT_KEY_SIZE ZONE_8_SIZE

#ifdef __cplusplus
}
#endif

#endif /* __DEVICE_CONFIG_ZONES_H__ */
