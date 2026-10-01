//----------------------------------------------------------------------------
//
//  Workfile: ota.hpp
//
//  Copyright: Jim Wright 2026
//
//  Notes:
//     OTA Wi-Fi and firmware update interface definitions.
//
//----------------------------------------------------------------------------
#ifndef OTA_H
#define OTA_H

void wifiInit(void);
void otaRun(void);

bool isWifiGood();

#endif