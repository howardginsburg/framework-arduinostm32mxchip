// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#include "SettingUI.h"

// Active UI array — defaults to the built-in SETTING_UI table.
// Overridden at runtime when a custom profile provides CUSTOM_PROFILE_UI.
static const SettingUIMetadata* s_activeUI = SETTING_UI;
static int s_activeUICount = (int)SETTING_UI_COUNT;

const SettingUIMetadata* SettingUI_GetActiveArray(void)
{
    return s_activeUI;
}

int SettingUI_GetActiveCount(void)
{
    return s_activeUICount;
}

void SettingUI_SetCustomUI(const SettingUIMetadata* ui, int count)
{
    if (ui != NULL && count > 0)
    {
        s_activeUI = ui;
        s_activeUICount = count;
    }
}
