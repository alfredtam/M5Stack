#include <M5StickC.h>
//Four headers for Web Server//
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>


//Declarations for Web Server
const char *ssid = "";
const char *password = "";
WebServer server(80);
const int led = 13;

//Declarations for Accelerometer
float accX = 0;
float accY = 0;
float accZ = 0;
int ss = 0;
int mn = 0;
int hr = 0;
int UseCount = 0;
int RecordedTime = 0;
int RollToHalt = 0; //Account for upward movement/return to position//
boolean RunStatus = false;

//Declarations for Button
boolean ButtonToggle = 0;

// DisplayIP //
void DisplayIP(){
  if (M5.BtnA.wasPressed()) {
      ButtonToggle = 1;
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setRotation(1);
      M5.Lcd.setTextSize(1);
    do {
      M5.Lcd.setCursor(12, 35);
      M5.Lcd.println(WiFi.localIP());
      M5.update();
      delay(1000);
      if (M5.BtnB.wasPressed()){
        ButtonToggle = 0;
        StaticPrint();
       }
    } while (ButtonToggle==1);
  }
}

// End of DisplayIP

//Render webpage

void RenderWebpage(){
  char temp[400];
  snprintf(temp, 400,

  "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>Smart Needle</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; font-size:40px; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hi Sewist!</h1>\
    <p>Usage Counter: %0d </p>\
    <p>Runtime: %02d:%02d:%02d</p>\
  </body>\
  </html>", UseCount, hr, mn % 60, ss % 60
  );
  server.send(200, "text/html", temp);
}

//*End of Render webpage

//Exception handling of webpage

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

// End of Exception handling of webpage

void StaticPrint(){
  M5.Lcd.setRotation(4); //Display aligned with the M5 button//
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(5, 5); //From left of screen, From top of screen//
  M5.Lcd.println("N-TRKR");

  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(5,60);
  M5.Lcd.println("U-Cnt:");

  M5.Lcd.setCursor(5,105);
  M5.Lcd.println("RTime:");
}


void setup() {
  // Run Once Code //
  // Needle Tracker //
  M5.begin();

  StaticPrint();

  M5.MPU6886.Init();

  // Web Server //
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) { //Wait for connection
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("IOT-CTRL")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", RenderWebpage); //Renders Webpage
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound); //Exception Handling
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Repeated Code //
  /* Extremely important for buttons to work */
  M5.update();

  // DisplayIP //
  DisplayIP();

  // Needle Tracker //

  M5.MPU6886.getAccelData(&accX,&accY,&accZ); //Get Accelerometer Data//
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(5, 25);
  M5.Lcd.printf("X(mg): %.0f",accX * 1000);
  M5.Lcd.setCursor(5, 35);
  M5.Lcd.printf("Y(mg): %.0f",accY * 1000);
  M5.Lcd.setCursor(5, 45);
  M5.Lcd.printf("Z(mg): %.0f",accZ * 1000);

  if(int(accY) > 0.9){
    UseCount = UseCount + 1;
    RunStatus = true;
    RecordedTime = RecordedTime + 1000;
  }
  else if(RunStatus == true && RollToHalt < 5){
        RecordedTime = RecordedTime + 1000;
        RollToHalt = RollToHalt + 1;
  }
  else {
        RunStatus = false;
        RollToHalt = 0;
  }


  M5.Lcd.setCursor(5,80);
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf("[%0d]",UseCount);


  ss = RecordedTime / 1000;
  mn = ss / 60;
  hr = mn / 60;

  M5.Lcd.setCursor(5,125);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("%02dh:%02dm:%02ds\n", hr, mn % 60, ss % 60);

  delay(500);


  // Web Server //
//  M5.Lcd.setTextSize(0);
//  M5.Lcd.setCursor(0,140);
//  M5.Lcd.println(WiFi.localIP());
  server.handleClient();
}
