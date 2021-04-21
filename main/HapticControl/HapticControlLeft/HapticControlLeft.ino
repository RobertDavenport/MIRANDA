#include <TimeProfiler.h>
#include <WiFi.h>
#include <AsyncUDP.h>
#include <analogWrite.h>

const char * ssid = "esp32_ssid";
const char * password = "password";

char intensity[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
int intensity_values[8] = {255, 220, 205, 190, 175, 155, 130, 0};
int pins[3] = {12, 13, 14};
int port = 1234;
String sensorMappings;

AsyncUDP udp;


void setup() {
  for(int i = 0; i<3; i++){
    pinMode(pins[i], OUTPUT);
  }
  Serial.begin(115200);

  WiFi.mode(WIFI_STA); // Make this the client (the server is WIFI_AP)
  WiFi.begin(ssid, password);
  delay(100);
  Serial.print("Connecting...");
  // Display a period every 0.5 s to show the user something is happening.
  while (WiFi.waitForConnectResult() != WL_CONNECTED) { 
    Serial.print(".");
    delay(500);
  }
  Serial.println("");

  if(udp.listen(port)) {
    udp.onPacket([](AsyncUDPPacket packet) {
      //Serial.print("Received data: ");
      //Serial.write(packet.data(), packet.length());
      //Serial.println();
      String getData((const __FlashStringHelper*) packet.data());
      sensorMappings = sensorMappings.substring(0,2);
      //Serial.println(sensorMappings);
    });
  }
}

void loop() {


    delay(100);
    udp.broadcastTo("A message for the server from the client.", port);
    //TIMEPROFILE_BEGIN(one); // profiler
  
    
    //SCOPED_TIMEPROFILE(all); // begin scoped profiler
    Serial.println(sensorMappings);
    int vals[3] = {0,0,0};
    for(int i = 0; i<3; i++){
        char c = sensorMappings.charAt(i);
        //Serial.println(c);
        //TIMEPROFILE_END(one); // end "one"
        int j = 0;
        for(j = 0; j < 8; j++){
        if(c == intensity[j])
            break; 
        }
        vals[i] = intensity_values[j];
    }
    
    // Print profiler
    //Serial.print("all : ");
    //Serial.println(TimeProfiler.getProfile("all"));
    //Serial.print("one : ");
    //Serial.println(TimeProfiler.getProfile("one"));
    
    for(int i = 0; i<3; i++){
      analogWrite(pins[i], vals[i]);
    }    
}
