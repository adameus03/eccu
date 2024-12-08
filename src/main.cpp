#include <Arduino.h>
#include <WiFi.h>
#include "soc/rtc_cntl_reg.h"  //disable brownout problems

#include "confidential/WiFiCredentials.hpp"
#include "confidential/serverinfo.hpp"
#include "confidential/clientinfo.hpp"
#include "ota.hpp"
#include "camera.hpp"
#include "webserver.hpp"

// IPAddress local_IP(192, 168, 1, 254);
// IPAddress gateway(192, 168, 1, 1);
// IPAddress subnet(255, 255, 255, 0);

WiFiClient client;

void initWiFi() {
    // Configures static IP address
    /*if (!WiFi.config(local_IP, gateway, subnet)) {
        Serial.println("STA Failed to configure");
    }*/
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    Serial.println("Attempting WiFi connection");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    Serial.println("");
    Serial.println("WiFi connected");
    
    Serial.print("Local IP: ");
    Serial.print(WiFi.localIP());
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
 
  Serial.begin(115200); 
  Serial.setDebugOutput(false);

  initWiFi();
  Serial.print("WiFi RSSI: ");
  Serial.println(WiFi.RSSI());
  startWebServer();

  startHandlingPublicOTA();
  haltPublicOTA(); // will be resumed when requested via web interface or in emergency OTA handling
  
  // resumePublicOTA(); //emergency OTA handling
  // Serial.println("Delaying 60 seconds for emergency OTA handling");
  // delay(60000);
  // Serial.println("Resuming normal operation");
  // haltPublicOTA();
  

  esp_err_t res = initialize_camera();
  if (res != ESP_OK) {
    Serial.println("Camera init failed with error = " + String(res));
    Serial.println("Restarting in 60 seconds");
    for (int i = 0; i < 60; i++) {
      if(checkIfTimeForPublicOTAHandle()){
        publicOTATickImplied();
      }
      delay(1000);
      Serial.print(".");
    }
    Serial.println("Restarting now");
    ESP.restart();
  }
}

String sendPhoto() {
  String getAll;
  String getBody;

  freeze_load_fb();
  camera_fb_t * fb = get_fb_no_reserve();
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }
  
  String serverName = SERVER_NAME;
  String serverPath = SERVER_PATH;
  Serial.println("Connecting to server: " + serverName);

  if (client.connect(serverName.c_str(), SERVER_PORT)) {
    Serial.println("Connection successful!");    
    String head = "--Tesseract\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--Tesseract--\r\n";

    uint16_t imageLen = fb->len;
    uint16_t extraLen = head.length() + tail.length();
    uint16_t totalLen = imageLen + extraLen;
  
    client.println("POST " + serverPath + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Length: " + String(totalLen));
    // identify client
    client.println("X-Client-Id: " + String(CAM_ID));
    client.println("Content-Type: multipart/form-data; boundary=Tesseract");
    client.println();
    client.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0; n<fbLen; n=n+1024) {
      if (n+1024 < fbLen) {
        client.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        client.write(fbBuf, remainder);
      }
    }   
    client.print(tail);
    
    release_fb();
    
    int timoutTimer = 10000;
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + timoutTimer) > millis()) {
      Serial.print("*");
      delay(100);      
      while (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (getAll.length()==0) { state=true; }
          getAll = "";
        }
        else if (c != '\r') { getAll += String(c); }
        if (state==true) { getBody += String(c); }
        startTimer = millis();
      }
      if (getBody.length()>0) { break; }
    }
    Serial.println();
    client.stop();
    Serial.println(getBody);
  }
  else {
    getBody = "Connection to " + serverName +  " failed.";
    Serial.println(getBody);
    Serial.println("Restarting WiFi connection...");
    if (!WiFi.disconnect()) {
      Serial.println("Disconnecting WiFi failed");
      Serial.println("Restarting system now");
      ESP.restart();
    }
    if (!WiFi.reconnect()) {
      Serial.println("Reconnecting WiFi failed");
      Serial.println("Restarting system now");
      ESP.restart();
    }
    Serial.println("Restarted WiFi connection");
  }
  return getBody;
}

void loop() {
  if(checkIfTimeForPublicOTAHandle()){
    publicOTATickImplied();
  } else {
    String response = sendPhoto();
    Serial.println(response);
    delay(100);
  }
}











