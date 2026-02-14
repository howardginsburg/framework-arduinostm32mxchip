// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#ifndef __DEVICE_CONFIG_CLI_H__
#define __DEVICE_CONFIG_CLI_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Print help for all available configuration commands in the active profile
 */
void config_print_help(void);

/**
 * @brief Dispatch a configuration command
 * 
 * @param cmdName Command name (e.g., "set_wifissid", "set_broker")
 * @param argc Argument count
 * @param argv Argument array
 * @return true if command was handled, false if not recognized
 */
bool config_dispatch_command(const char* cmdName, int argc, char** argv);

/**
 * @brief Show configuration status for all settings in the active profile
 */
void config_show_status(void);

#ifdef __cplusplus
}
#endif

#endif /* __DEVICE_CONFIG_CLI_H__ */
