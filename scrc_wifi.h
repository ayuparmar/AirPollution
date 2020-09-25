#ifndef SCRC_WIFI_H
#define SCRC_WIFI_H
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include "secrets.h"
#include <WiFiUdp.h>
#include <stdlib.h>

#define MAIN_SSID SECRET_SSID
#define MAIN_PASS SECRET_PASS


void nw_setup();

void nw_start();
void nw_stop();

#endif /* SCRC_WIFI_H */
