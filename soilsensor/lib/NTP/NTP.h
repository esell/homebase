#ifndef _NTP_H
#define _NTP_H

#include <Arduino.h>
#include <Time.h>
#include <TimeLib.h>
#include <String.h>
#include <WiFiUdp.h>


// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address);

char* GetISODateTime();

time_t getNtpTime();

#endif
