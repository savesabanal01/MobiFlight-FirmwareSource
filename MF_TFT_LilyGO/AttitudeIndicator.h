#pragma once


namespace AttitudeIndicator
{
    enum {
        ROUND_SHAPE = 1,
        RECT_SHAPE
    };
    
    void init(uint8_t type);
    void loop();
    void stop();
}
