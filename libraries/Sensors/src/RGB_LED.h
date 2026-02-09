#ifndef __RGB_LED_H__
#define __RGB_LED_H__

#include "mbed.h"

// Predefined colors
#define RGB_LED_RED       255, 0, 0
#define RGB_LED_GREEN     0, 255, 0
#define RGB_LED_BLUE      0, 0, 255
#define RGB_LED_YELLOW    255, 255, 0
#define RGB_LED_CYAN      0, 255, 255
#define RGB_LED_MAGENTA   255, 0, 255
#define RGB_LED_WHITE     255, 255, 255
#define RGB_LED_ORANGE    255, 165, 0

class RGB_LED
{
    public:
        RGB_LED(PinName red = PB_4, PinName green = PB_3, PinName blue = PC_7);

        void setColor(int red = 255, int green = 255, int blue = 255);
        void turnOff();
        
        // Helper methods for common colors
        void setRed()       { setColor(RGB_LED_RED); }
        void setGreen()     { setColor(RGB_LED_GREEN); }
        void setBlue()      { setColor(RGB_LED_BLUE); }
        void setYellow()    { setColor(RGB_LED_YELLOW); }
        void setCyan()      { setColor(RGB_LED_CYAN); }
        void setMagenta()   { setColor(RGB_LED_MAGENTA); }
        void setWhite()     { setColor(RGB_LED_WHITE); }
        void setOrange()    { setColor(RGB_LED_ORANGE); }

    private: 
        PwmOut _red;
        PwmOut _green;
        PwmOut _blue;
};

#endif