#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <WiFiAP.h>
#include <Wire.h>
#include <SPIFFS.h>
#include <Servo.h>
#include "main.h"

#if __has_include("credentials_hidden.h")
#include "credentials_hidden.h"
#else 
#include "credentials.h"
#endif

String urlMouse = "/mouse";
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);
uint16_t seed=123;
volatile int16_t speed=0,turn=0;
float _real_speed=0,_real_turn=0;
long last_speed_update = 0;
long last_speed_message = 0;
long last_display_update = 0;
long start_time=0;
bool stop=false;
bool redraw=true;

DNSServer dnsserver;
AsyncWebServer webserver(80);
void btnPressed();
void startChangeTime();

void setup() 
{
  Serial.begin(115200);
  //delay(3000);

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    while(1);
  }

  // START WIFI
  WiFi.softAPConfig(apIP, apIP, netMsk);
  if (!WiFi.softAP(wifiCredentials[MOUSEID].ssid, wifiCredentials[MOUSEID].password)) {
    log_e("Soft AP creation failed.");
    while (1);
  }
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  urlMouse = String("http://") + myIP.toString() + "/mouse";

  // Start Web server
  setupWebServer();

  // DISPLAY QR CODE
  setupMotors();  
}

void loop() 
{
  dnsserver.processNextRequest();
  updateMotors();
}



//***************************************************************/
//********************* MOTOR AND SERVO     *********************/
//***************************************************************/

Servo servo_turn;
Servo servo_speed;  


void setupMotors()
{
  servo_turn.attach(SRV_TURN);
  servo_speed.attach(SRV_SPEED);
  updateMotors();
}

void updateMotors()
{
  if (millis()-last_speed_update<100)
    return;
  last_speed_update = millis();

  if (stop || (millis()-last_speed_message>3000))
  {
    speed=0;
    turn=0;
  }
  /*
  Serial.print("Speed: ");
  Serial.print(speed);
  Serial.print(" Turn: ");
  Serial.println(turn);
  */
  // NEW SPEED CALC!
  servo_turn.writeMicroseconds(map(turn,-255,255,1200,1800));
  servo_speed.writeMicroseconds(map(speed,-255,255,2400,600));
}

//***************************************************************/
//********************* WebServer/DnsServer *********************/
//***************************************************************/
void setupWebServer() 
{
  Serial.println("Starting DNS Server");
  dnsserver.start(53, "*", WiFi.softAPIP());

 
  webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    captiveRedirect(request);   
  });

  webserver.onNotFound([](AsyncWebServerRequest *request){
    captiveRedirect(request);
  });

  webserver.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)  
  { 
    speed = 0;
    turn = 0;
    if (request->hasArg("x"))
      turn = map(request->arg("x").toInt(),-100,100,-255,255);
    if (request->hasArg("y"))
      speed = map(request->arg("y").toInt(),-10,10,-255,255);
    if (!stop)
      request->send(200, "text/plain", "OK\n");
    else
      request->send(418, "text/plain", "Game Over - I'm a teapot\n");
    //Serial.printf("Got speed: %d turn: %d\n", speed, turn);
    last_speed_message = millis();
    if (start_time==0 && (speed!=0 || turn!=0))
      start_time=millis();
  });

  
  webserver.on("/getname", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(200, "text/plain", "Mouse " + String(MOUSEID) + "\n");
  });

  webserver.on("/mouse", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/index.html", String(), false);
    //request->send(200, "text/html", index_html);
    //Serial.println("Client Connected");
  });

 // --- Captive portal probe endpoints (serve HTML 200 + JS redirect) ---
  // Google/Android
  webserver.on("/generate_204", HTTP_ANY, captiveOK);
  webserver.on("/gen_204", HTTP_ANY, captiveOK);
  // Microsoft/Windows
  webserver.on("/connecttest.txt", HTTP_ANY, captiveOK);
  webserver.on("/ncsi.txt", HTTP_ANY, captiveOK);
  // Apple
  webserver.on("/hotspot-detect.html", HTTP_ANY, captiveOK);
  webserver.on("/captive.apple.com", HTTP_ANY, captiveOK);
  // Samsung variants (HTTP)
  webserver.on("/connectivitycheck.txt", HTTP_ANY, captiveOK);
  webserver.on("/mobile/success.html", HTTP_ANY, captiveOK);
  // Some carriers poke simple roots like "/"
  webserver.on("/success.txt", HTTP_ANY, captiveOK);

  webserver.begin();
}

void captiveOK(AsyncWebServerRequest *request) 
{
    Serial.println("Captive portal probe detected");
    request->send(200, "text/html",
                  "<!doctype html><meta name='viewport' content='width=device-width, initial-scale=1'>"
                  "<title>Sign-in</title><p>Redirecting...</p>"
                  "<script>location.replace('/');</script>");
}

void captiveRedirect(AsyncWebServerRequest *request) 
{
//    Serial.println("Request redirected to captive portal");
    auto response = request->beginResponse(302, "text/plain", "");
    response->addHeader("Location", String("http://") + request->client()->localIP().toString()+"/mouse", true);
    request->send(response);

}
