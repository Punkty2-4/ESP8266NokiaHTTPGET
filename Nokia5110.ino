#define RST 5    //D1   GPIO5
#define CE  4    //D2   GPIO4
#define DC  0    //D3   GPIO0
#define DIN 2    //D4   GPIO2
#define CLK 14   //D5   GPIO14

#include <SPI.h>
#include <ESP8266WiFi.h>
#include "font.h"

char server[] = "api.thingspeak.com";    // name address for Google (using DNS)
char ssid[]   = "tsWIFI";                // your network SSID (name)
char pass[]   = "ginger4ever";           // your network password
String APIKey = "KF9BI90WJIEAGU44";      // API Key
bool recordFlag = false;
String msgStr;
WiFiClient client;

void LcdWriteCharacter(char character);
void LcdWriteData(byte dat);
void LcdWriteCmd(byte cmd);
void retrieveMsg();

void LcdWriteString(char *characters) {
  while(*characters) LcdWriteCharacter(*characters++);
}

void LcdWriteCharacter(char character) {
  for(int i=0; i<5; i++) LcdWriteData(ASCII[character - 0x20][i]);
  LcdWriteData(0x00);
}

void LcdWriteData(byte dat) {
  digitalWrite(DC, HIGH); //DC pin is low for commands
  digitalWrite(CE, LOW);
  shiftOut(DIN, CLK, MSBFIRST, dat); //transmit serial data
  digitalWrite(CE, HIGH);
}

void LcdXY(int x, int y) {
  LcdWriteCmd(0x80 | x);  // Column.
  LcdWriteCmd(0x40 | y);  // Row.
}

void LcdWriteCmd(byte cmd) {
  digitalWrite(DC, LOW); //DC pin is low for commands
  digitalWrite(CE, LOW);
  shiftOut(DIN, CLK, MSBFIRST, cmd); //transmit serial data
  digitalWrite(CE, HIGH);
}

void retrieveMsg() {
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    //Serial.println("connected");
    recordFlag = true;
    // Make a HTTP request:
    client.println("GET /apps/thinghttp/send_request HTTP/1.0");
    client.println("Host: api.thingspeak.com");
    client.println("Connection: close");
    client.println("X-THINGSPEAKAPIKEY: "+APIKey);
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }  
}

void setup() {
  
  pinMode(RST, OUTPUT);
  pinMode(CE, OUTPUT);
  pinMode(DC, OUTPUT);
  pinMode(DIN, OUTPUT);
  pinMode(CLK, OUTPUT);
  digitalWrite(RST, LOW);
  digitalWrite(RST, HIGH);
  
  LcdWriteCmd(0x21);  // LCD extended commands
  LcdWriteCmd(0xBF);  // set LCD Vop (contrast)
  LcdWriteCmd(0x04);  // set temp coefficent
  LcdWriteCmd(0x14);  // LCD bias mode 1:40
  LcdWriteCmd(0x20);  // LCD basic commands
  LcdWriteCmd(0x0C);  // LCD normal video
  
  for(int i=0; i<504; i++) LcdWriteData(0x00); // clear LCD

  LcdXY(0,0);
  LcdWriteString("Current Temp:"); 

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection:
  /*if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }*/
  WiFi.begin(ssid, pass);

  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");
  retrieveMsg();  
  
}

void loop() {

  //retrieveMsg();
  if (client.available() && (recordFlag == true) ) {
    char c = client.read();
    msgStr += c;
    //Serial.print(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    //Serial.println();
    //Serial.println("disconnecting.");
    recordFlag = false;
    //Serial.print(msgStr);
    int index = msgStr.lastIndexOf("Connection: close");
    String subStr = msgStr.substring(index + 21, index + 26);
    char buf[256];
    subStr.toCharArray(buf, subStr.length());
    LcdXY(0,2);
    LcdWriteString("                ");
    LcdXY(0,2);
    LcdWriteString(buf);
    LcdXY(42,2);
    LcdWriteString("C deg");
    Serial.print(subStr);
    msgStr = "";
    client.stop();
    delay(1000);
    retrieveMsg();  

  }
    
}
