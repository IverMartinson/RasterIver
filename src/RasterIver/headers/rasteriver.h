#ifndef RASTERIVER_H
#define RASTERIVER_H

#include <stdint.h>

typedef int RI_result;
typedef int RI_flag;
typedef uint32_t RI_uint;

// RI_result
typedef enum {
    RI_ERROR = -1,
    RI_SUCCESS = 0,
    RI_NOT_RUNNING = -2,
    RI_RUNNING = 1,
} RI_result_enum;

// RI_flag
typedef enum {
    RI_FLAG_DEBUG = 0,
    RI_FLAG_DEBUG_VERBOSE = 1,
    RI_FLAG_SHOW_Z_BUFFER = 2,
} RI_flag_enum;

RI_result RI_Init();
RI_result RI_Stop();
RI_result RI_IsRunning();
RI_result RI_RequestPolygons(int RI_PolygonsToRequest);
RI_result RI_Tick();
RI_result RI_SetBackground(RI_uint RI_BackgroundColor);
RI_result RI_ShowZBuffer(int RI_ShowZBufferFlag);
RI_result RI_SetFlag(RI_flag RI_FlagToSet, int RI_Value);

#endif // RASTERIVER_H