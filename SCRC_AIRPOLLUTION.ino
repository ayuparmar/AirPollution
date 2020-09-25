#include <Arduino.h>
#include<time.h>
#include "scrc_wifi.h"
#include "scrc_dht.h"
#include "secrets.h"
#include "scrc_pub_thingspeak.h"
#include "scrc_sds.h"
#include "scrc_noise.h"
#include "scrc_co2.h"
#include "scrc_gas.h"
#include  "scrc_ntpclient.h"
#include "scrc_pub_onem2m.h"

#define MAIN_LOOP_DELAY  10000

WiFiClientSecure httpsClient;
WiFiClient client;


// declare all the parameters
//Buffer to store pm2.5 & pm10 values
float* buf_pm = new float[2];

//Buffer to store Temperature & Humudity values
short* buf_temp_rh = new short[2];

//Buffer to store Noise values
short* buf_noise = new short[1];

//Buffer to store AQI values
short* buf_aqi = new short[1];

//Buffer to store CO2 values
short* buf_co2 = new short[1];

//Buffer to store Gas values
float* buf_gas = new float[3];

//Buffer to store config values
String* buf_config = new String[3];



String d;
float so2;     //Dummy still not having this sensor
String AQL, AQI_MP;

//################## HTTP constants###############################
int LOCAL_PORT = 9999;
char *HTTP_CREATED = "HTTP/1.1 201 Created";
char *HTTP_OK = "HTTP/1.1 200 OK\r\n";
int REQUEST_TIME_OUT = 5000; //ms
//################################################################

WiFiServer server(LOCAL_PORT); // HTTP Server (over WiFi). Binded to listen on LOCAL_PORT contant
const char* host = "onem2m.iiit.ac.in";
const int httpsPort = 443;
const char fingerprint[] PROGMEM = "10 3D D5 4E B1 47 DB 4B 5C B0 89 08 41 A7 A4 14 87 10 7F E8";
String AE = "test_Air_Pollution_Monitoring";
String Container = "NLGR/NLGR_FF";
String Link = "/~/in-cse/in-name/" + AE + "/" + Container;


String time_stamp;
void setup()
{
  nw_setup();
  ntp_setup();
  hw_setup_dht();
  hw_setup_sds();
  hw_setup_noise();
  hw_setup_co2();
 // hw_setup_gas();
  pub_setup_thingspeak();

}

void loop() {
  nw_start();
  connect_http();
  time_stamp = ntp_loop();
  hw_read_dht(buf_temp_rh);
  hw_read_sds(buf_pm);
  buf_aqi[0] = read_aqi(buf_pm[0], buf_pm[1]);
  hw_read_noise(buf_noise);
  //hw_read_gas(buf_gas);
  hw_read_co2(buf_co2);
  // Serial.println(time_stamp);
  AQL = aql(buf_aqi[0]);
  AQI_MP = aqi_mp(buf_pm[0], buf_pm[1]);
  // publish data to thingspeak
  pub_thingspeak(time_stamp, buf_pm[0], buf_pm[1],  buf_temp_rh[1], buf_temp_rh[0], buf_co2[0], buf_aqi[0], buf_noise[0], buf_gas[0], buf_gas[1], buf_gas[2]);

  //publish data to onem2m
 publish_onem2m(buf_pm[0], buf_pm[1],  buf_temp_rh[1], buf_temp_rh[0], buf_gas[0], buf_co2[0], buf_gas[1], buf_gas[2], buf_aqi[0], buf_noise[0], so2 , AQL, AQI_MP, Link, host);

  //Serial.println(buf_noise[0]);
  //Serial.println(buf_gas[1]);
  free (buf_noise);
  buf_noise = NULL;

  delay(MAIN_LOOP_DELAY);

}
