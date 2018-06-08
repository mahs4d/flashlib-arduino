#include <Esp8266.h>

#define SOFT_RX 8
#define SOFT_TX 9

#define PORT 8890


SoftwareSerial wifiSerial(SOFT_RX, SOFT_TX);
Esp8266 esp8266 = Esp8266();

uint8_t data[32];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  wifiSerial.begin(9600);
  
  esp8266.setEspSerial(&wifiSerial);  
  // use uart for debug
  esp8266.setDebugSerial(&Serial);
  
  esp8266.setDataByteArray(data, 32);

  esp8266.configureServer(PORT);
}

void loop() {
  if (esp8266.readSerial()) {
    if (esp8266.isData()) {  
      /*
      Serial.print("Got data, len ");
      Serial.print(esp8266.getDataLength());
      Serial.print(" on channel ");
      Serial.println(esp8266.getChannel());
      */
      
      // see what we got
      for (int i = 0; i < esp8266.getDataLength(); i++) {
        Serial.print(data[i]);  
      }
      
      // output to debug
      //esp8266.debug(data, 32);
      
      if (esp8266.send(esp8266.getChannel(), "ok") == SUCCESS) {
       // success 
      } else {
       // failed to send 
      }
      
      // or send byte array
//      uint8_t buf[1];
//      buf[0] = 0;
//      esp8266.send(esp8266.getChannel(), buf, 1);
    } else if (esp8266.isConnect()) {
      Serial.print("Connected on channel ");
      Serial.println(esp8266.getChannel());     
    } else if (esp8266.isDisconnect()) {
      Serial.print("Disconnected on channel ");
      Serial.println(esp8266.getChannel());     
    } 
  } else if (esp8266.isError()) {
    Serial.print("Failed on command ");
    Serial.print(esp8266.getLastCommand());
    Serial.print(" with error ");
    Serial.println(esp8266.getLastResult());    
    
    if (esp8266.getLastCommand() == UNKNOWN_COMMAND) {
      Serial.print("Resetting");
      
      // assume the worst and reset
      esp8266.restartEsp8266();
      // need to apply server config which is lost on restart
      esp8266.configureServer(PORT);
    }
  } else {
   // no data 
  }
}
