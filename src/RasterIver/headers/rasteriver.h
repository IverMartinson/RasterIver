#ifndef RASTERIVER_H
#define RASTERIVER_H

#include <stdint.h>

typedef int RI_result;
typedef uint32_t RI_uint;

extern const RI_result RI_SUCCESS;
extern const RI_result RI_ERROR;
extern const RI_result RI_NOT_RUNNING;
extern const RI_result RI_RUNNING;

RI_result RI_Init();
RI_result RI_Stop();
RI_result RI_IsRunning();
RI_result RI_RequestPolygons(int RI_PolygonsToRequest);
RI_result RI_Tick();
RI_result RI_SetBackground(RI_uint RI_BackgroundColor);
RI_result RI_ShowZBuffer(int RI_ShowZBufferFlag);
RI_result RI_SetDebugFlag(int RI_ShowDebugFlag);

#endif // RASTERIVER_H