//#include <TimeProfiler.h>
#include <WiFi.h>
#include <AsyncUDP.h>
#include <analogWrite.h>

const char * ssid = "esp32_ssid";
const char * password = "password";

char intensity[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
int intensity_values[8] = {255, 255, 175, 175, 155, 155, 0, 0};
int pins[3] = {14, 12, 13};
int port = 1234;
char * sensorMappings;

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
}

void loop() {
  if(udp.listen(port)) {
    udp.onPacket([](AsyncUDPPacket packet) {
      Serial.print("Received data: ");
      Serial.write(packet.data(), packet.length());
      Serial.println();
      sensorMappings = (char * )packet.data();
      Serial.println(sensorMappings);
      int vals[3] = {0,0,0};
      for(int i = 0; i<3; i++){
          char c = sensorMappings[i];
          int j = 0;
          for(j = 0; j < 8; j++){
          if(c == intensity[j])
              break; 
          }
          vals[i] = intensity_values[j];
      }
      for(int i = 0; i<3; i++){
        analogWrite(pins[i], vals[i]);
      } 
    });
  }   
}
