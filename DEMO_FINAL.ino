
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////// Code for Airpollution deployed node in IIIT Campus ///////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        
#include <JSONVar.h>
#include <JSON.h>
#include <Arduino_JSON.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <time.h>
#include "secrets.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include "DHT.h"
#include <ThingSpeak.h>
#include "MutichannelGasSensor.h"
#include <SDS011.h>
#include <stdlib.h>
#define version "T6700-R15 I2C v1.0"



#define SECRET_SSID12 SECRET_SSID
#define SECRET_PASS12 SECRET_PASS

//Constants
#define DHTPIN D3    // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define node_RX D7
#define node_TX D8
#define ADDR_I2C 0x04
#define readDelay 5  //delay between I2C write & read requests in mS (10 recommended)
#define measureDelay 1000  //delay between measure and read PPm requests in mS (2250 min recommended)
#define ADDR_6700  0x15 // default I2C slave address is 0x15
#define sampleTime 30000 //time between measures
#define SENSOR_ADDR     0X19
#define PRE_HEAT_TIME   1

SDS011 my_sds;
DHT dht(DHTPIN, DHTTYPE);

//Variables
int chk, error;
float hum, temp, p10, p25;
float Co;
float no2;
float nh3;
String data;
int data1 [6];
int CO2ppmValue;
const int sampleWindow = 50;                              // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;



//////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   MAIN WIFI DETAILS OF DEPLOYMENT SITE and Thinkspeak Deatil /////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#define MAIN_SSID SECRET_SSID12
#define MAIN_PASS SECRET_PASS12
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

char CURRENT_SSID[] = MAIN_SSID;//SECRET_SSID7;
char CURRENT_PASS[] = MAIN_PASS;
//----------------  Fill in your credentails   ---------------------
char ssid[] = MAIN_SSID;                      // your network SSID (name)
char pass[] = MAIN_PASS;                      // your network password
char WIFI_SSID[] = MAIN_SSID;                 // WiFi for ThingSpeak part
char WIFI_PSWD[] = MAIN_PASS;                 // WiFi for OneM2M part

// ##################### Update of IP adress of the server#################
String CSE_IP = "onem2m.iiit.ac.in";
// #######################################################

int WIFI_DELAY = 100; //ms

// oneM2M : CSE params
int CSE_HTTP_PORT = 443;
String CSE_NAME = "in-name";
String CSE_M2M_ORIGIN = "admin:admin";

//############### oneM2M : resources' params#####################
String DESC_CNT_NAME = "DESCRIPTOR";
String DATA_CNT_NAME = "DATA";
String CMND_CNT_NAME = "COMMAND";
int TY_AE = 2;
int TY_CNT = 3;
int TY_CI = 4;
int TY_SUB = 23;
//################################################################

//################## HTTP constants###############################
int LOCAL_PORT = 9999;
char *HTTP_CREATED = "HTTP/1.1 201 Created";
char *HTTP_OK = "HTTP/1.1 200 OK\r\n";
int REQUEST_TIME_OUT = 5000; //ms
//################################################################


WiFiServer server(LOCAL_PORT); // HTTP Server (over WiFi). Binded to listen on LOCAL_PORT contant
WiFiClient client;
String context = "";
String command = ""; // The received command

//############MISC##########

int SERIAL_SPEED = 9600;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// USING HTTP CLIENT /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

String cse_ip = "onem2m.iiit.ac.in";
String cse_port = "443";
String ser = "https://" + cse_ip + ":" + cse_port + "/~/in-cse/in-name/";

const char* host = "onem2m.iiit.ac.in";
const int httpsPort = 443;

//SHA1 finger print of certificate use web browser to view and copy
const char fingerprint[] PROGMEM = "10 3D D5 4E B1 47 DB 4B 5C B0 89 08 41 A7 A4 14 87 10 7F E8";
//=======================================================================
//                    Power on setup for Airpollution
//=======================================================================


void setup() {
  WiFiClientSecure httpsClient;                   //Declare object of class WiFiClient
  delay(1000);
  Serial.begin(115200);

  WiFi.mode(WIFI_OFF);                           //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);                           //Only Station No AP, This line hides the viewing of ESP as wifi hotspot

  WiFi.begin(ssid, pass);                        //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());                 //IP address assigned to your ESP


  // randomSeed(analogRead(0));
  dht.begin();
  my_sds.begin(node_RX, node_TX);
  gas.begin(SENSOR_ADDR);   //
  /* for (int i = 60 * PRE_HEAT_TIME; i >= 0; i--)
    {
     Serial.print(i / 60);
     Serial.print(":");
     Serial.println(i % 60);
     delay(1000);
    }*/
}

String d;


//########## Function of Read Co2 ####################

int readC02()
{
  // start I2C
  Wire.beginTransmission(ADDR_6700);
  Wire.write(0x04); Wire.write(0x13); Wire.write(0x8B); Wire.write(0x00); Wire.write(0x01);
  // end transmission
  Wire.endTransmission();
  // read report of current gas measurement in ppm
  delay(2000);
  Wire.requestFrom(ADDR_6700, 5);    // request 4 bytes from slave device
  data1[0] = Wire.read();
  data1[1] = Wire.read();
  data1[2] = Wire.read();
  data1[3] = Wire.read();
  /*Serial.print("Func code: "); Serial.print(data[0],HEX);
    Serial.print(" byte count: "); Serial.println(data[1],HEX);
    Serial.print("MSB: 0x");  Serial.print(data[2],HEX); Serial.print("  ");
    Serial.print("LSB: 0x");  Serial.print(data[3],HEX); Serial.print("  ");*/
  CO2ppmValue = ((data1[2] * 0xFF ) + data1[3]);
}

//#################################################################





void loop() {
  // put your main code here, to run repeatedly:

  WiFiClientSecure httpsClient;                                           //Declare object of class WiFiClient

  Serial.println(host);

  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000);                                          // 15 Seconds
  delay(1000);

  Serial.print("HTTPS Connecting");
  int r = 0;                                                              //retry counter
  while ((!httpsClient.connect(host, httpsPort)) && (r < 30)) {
    delay(100);
    Serial.print(".");
    r++;
  }
  if (r == 30) {
    Serial.println(":Connection failed");
  }
  else {
    Serial.println(":Connected to web");
  }

  String getData, Link;

  //POST Data
  Link = "/~/in-cse/in-name/Team39_Outdoor_Air_Pollution/node_1";

  Serial.print("requesting URL: ");
  Serial.println(host);
  /*
    POST /post HTTP/1.1
    Host: postman-echo.com
    Content-Type: application/x-www-form-urlencoded
    Content-Length: 13

    say=Hi&to=Mom

  */

  Serial.println("");

  /*c = gas.measure_CO();
    Serial.print("The concentration of CO is ");
    if (c >= 0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
    String d = String(c) + ",";


    c = gas.measure_NO2();
    Serial.print("The concentration of NO2 is ");
    if (c >= 0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
    d = d + String(c) + ",";

    c = gas.measure_NH3();
    Serial.print("The concentration of NH3 is ");
    if (c >= 0) Serial.print(c);
    else Serial.print("invalid");
    d = d + String(c) + ',';*/

  error = my_sds.read(&p25, &p10);
  if (!error) {
    Serial.println("P2.5: " + String(p25) + "\t" + "P10:  " + String(p10));
    //ThingSpeak.setField(6, p25);
    //ThingSpeak.setField(7, p10);
    d = "PM2.5- " + String(p25) + ", " + "PM10- " + String(p10) + ",";

  }
  else {
    d = d + "-1,-1,";
  }


  hum = dht.readHumidity();
  temp = dht.readTemperature();

  //    Print temp and humidity values to serial monitor
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celsius");

  d = d + "Temp- " + String(temp) + ", " + "Hum- " + String(hum) + ", ";

  Co = gas.measure_CO();
  Serial.print("The concentration of CO is ");
  Serial.print(Co);
  d = d + "Co- " + String(Co) + ", ";


  no2 = gas.measure_NO2();
  Serial.print("The concentration of NO2 is ");
  Serial.print(no2);
  d = d + "No2- " + String(no2) + ", ";

  nh3 = gas.measure_NH3();
  Serial.print("The concentration of NH3 is ");
  Serial.print(nh3);
  d = d + "NH3-" + String(nh3) + ", ";


  // Print CO2 Value on serial monitor

  int co2Value = readC02();
  {
    Serial.print("CO2 Value: ");
    Serial.println(CO2ppmValue);
    //    ThingSpeak.setField(8, c);
    d = d + "Co2- " + String(CO2ppmValue) + ", ";
  }

  //


  // Noise

  unsigned long startMillis = millis();                  // Start of sample window
  float peakToPeak = 0;                                  // peak-to-peak level

  unsigned int signalMax = 0;                            //minimum value
  unsigned int signalMin = 1024;                         //maximum value

  // collect data for 50 mS
  while (millis() - startMillis < sampleWindow)
  {
    sample = analogRead(A0);                             //get reading from microphone
    if (sample < 1024)                                   // toss out spurious readings
    {
      if (sample > signalMax)
      {
        signalMax = sample;                              // save just the max levels
      }
      else if (sample < signalMin)
      {
        signalMin = sample;                              // save just the min levels
      }
    }
  }
  peakToPeak = signalMax - signalMin;                    // max - min = peak-peak amplitude
  float db = map(peakToPeak, 20, 900, 49.5, 90);         //calibrate for deciBels
  Serial.print("Noise Value: ");
  Serial.println(db);
  // ThingSpeak.setField(3, c);
  d = d + "Noise- " + String(db) + ", ";
  //

  //Print AQI value on Serial monitor.

  Serial.print("AQI: ");
  Serial.println(aqi(p25, p10));
  int a = aqi(p25, p10);
  //ThingSpeak.setField(5, aqi(p25,p10));
  d = d + "AQI- " + String(a) ;





  String data = String() + "{\"m2m:cin\":{\"con\":\"" + d + "\"}}";
  Serial.println(data);

  const char* origin   = "admin:admin";
  //const char *content = "application/json;ty=4";
  // const char *host = "onem2m.iiit.ac.in";

  String request = String() + "POST " + Link + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "X-M2M-Origin:" + origin + "\r\n" +
                   "Content-Type:application/json;ty=" + 4 + "\r\n" +
                   "Content-Length: " + data.length() + "\r\n" +
                   "Connection: close\r\n\n" +
                   data;
  //POST request

  httpsClient.print(request);

  Serial.println("request sent");

  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }

  Serial.println("reply was:");
  Serial.println("==========");
  String line;
  while (httpsClient.available()) {
    line = httpsClient.readStringUntil('\n');  //Read Line by Line
    Serial.println(line); //Print response
  }
  Serial.println("==========");
  Serial.println("closing connection");

  //############### Thinkspeak #############################################

  ThingSpeak.setField(1, (float)temp);
  ThingSpeak.setField(2, (float)hum);
  ThingSpeak.setField(3, (float)p25);
  ThingSpeak.setField(4, (float)p10);
  ThingSpeak.setField(5, (float)Co);
 // ThingSpeak.setField(6, (float)no2);
  ThingSpeak.setField(6, (float)nh3);

  ThingSpeak.setField(7, (int)CO2ppmValue);
  ThingSpeak.setField(8, (float)db);
  delay(1000);
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Channel update successful.");
  }
  \
  else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  //#######################################################################


  delay(20000);  //POST Data at every 20 seconds


}
