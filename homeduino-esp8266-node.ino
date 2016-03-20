 #include <ESP8266WiFi.h>
#include <RFControl.h>

#include "settings.h"

void setup()
{
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {  
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Setup interrupt");  
  Serial.flush();
  pinMode(RECEIVER_PIN, INPUT);
  RFControl::startReceiving(RECEIVER_PIN);
}
 
void loop()
{
  if(RFControl::hasData()) {
      unsigned int *timings;
      unsigned int timings_size;
      RFControl::getRaw(&timings, &timings_size);
      unsigned int buckets[8];
      unsigned int pulse_length_divider = RFControl::getPulseLengthDivider();
      RFControl::compressTimings(buckets, timings, timings_size);
      
      String bucketsStr = "[";
      for(unsigned int i=0; i < 8; i++) {
          unsigned long bucket = buckets[i] * pulse_length_divider;
          if(bucket == 0) {
           break; 
          }
          if(i != 0) {
            bucketsStr += ",";
          }
          bucketsStr += bucket;
          
      }
      bucketsStr += "]";
      String pulsesStr;
      for(unsigned int i=0; i < timings_size; i++) {
          pulsesStr += timings[i];
      }
      RFControl::continueReceiving();
      Serial.print(bucketsStr);
      Serial.print(" ");
      Serial.print(pulsesStr);
      Serial.print("\r\n");
      Serial.flush();
      String url = "/homeduino/received?apikey=" + String(HOMEDUINO_APIKEY) + "&buckets=" + bucketsStr + "&pulses=" + pulsesStr;
      WiFiClient client;
      if (!client.connect(PIMATIC_IP, PIMATIC_PORT)) {
        Serial.println("connection failed");
        return;
      }

      Serial.print("\r\n");
      // This will send the request to the server
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + PIMATIC_IP + "\r\n" + 
                   "Connection: close\r\n\r\n");
      delay(10);
      // Read all the lines of the reply from server and print them to Serial
      while(client.available()){
        String line = client.readStringUntil('\r');
        Serial.print(line);
      }
  
      Serial.flush();
      
    }
  delay(10);
}
