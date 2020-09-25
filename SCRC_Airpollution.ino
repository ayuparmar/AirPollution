//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////// Code for Airpollution deployed node in IIIT Campus ///////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include<time.h>
#include "scrc_wifi.h"
#include "scrc_sds.h"
#include "scrc_co2.h"
#include "scrc_dht.h"
#include "scrc_noise.h"
#include "scrc_gas.h"
#include "scrc_http.h"
#include "scrc_pub_thingspeak.h"
#include "scrc_pub_onem2m.h"
#include "secrets.h"


// define all the necessary config params
#define MAIN_LOOP_DELAY  10000
WiFiClientSecure httpsClient;
WiFiClient client;


// declare all the parameters
//Buffer to store pm2.5 & pm10 values
float* buf_pm = new float[2];

//Buffer to store Temperature & Humudity values
float* buf_temp_rh = new float[2];

//Buffer to store CO2 values
float* buf_co2 = new float[1];

//Buffer to store AQI values
float* buf_aqi = new float[1];

//Buffer to store Noise values
float* buf_noise = new float[1];

//Buffer to store Gas values
float* buf_gas = new float[3];

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
String AE = "Air_Quality_Monitoring";
String Container = "AQ_BN00_00/AP_data";
String Link = "/~/in-cse/in-name/" + AE + "/" + Container;

String randNumber;
// call respective hw & sw onetime, setup methods
void setup()
{
  nw_setup();

  hw_setup_sds();
  hw_setup_dht();
  hw_setup_co2();
  hw_setup_noise();
  //hw_setup_gas();
  pub_setup_thingspeak();

}


void loop() {

  // start the wifi n/w
  nw_start();

  // read the data from h/w and store in the corresponding buffers
  connect_http();

  hw_read_dht(buf_temp_rh);
  hw_read_co2(buf_co2);
  hw_read_sds(buf_pm);
  buf_aqi[0] = read_aqi(buf_pm[0], buf_pm[1]);
  hw_read_noise(buf_noise);
  hw_read_gas(buf_gas);
  AQL = aql(buf_aqi[0]);
  AQI_MP = aqi_mp(buf_pm[0], buf_pm[1]);
  // publish data to thinkspeek
  pub_thingspeak(buf_pm[0], buf_pm[1],  buf_temp_rh[1], buf_temp_rh[0], buf_co2[0], buf_aqi[0], buf_noise[0], buf_gas[0], buf_gas[1], buf_gas[2], so2, AQL, AQI_MP);

  //publish data to onem2m
  publish_onem2m(buf_pm[0], buf_pm[1],  buf_temp_rh[1], buf_temp_rh[0], buf_gas[0], buf_co2[0], buf_gas[1], buf_gas[2], buf_aqi[0], buf_noise[0], so2 , AQL, AQI_MP, Link, host);


  // stop the wifi n/w
  nw_stop();

  delay(MAIN_LOOP_DELAY);
}
