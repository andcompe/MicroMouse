#ifndef MAIN_H
#define MAIN_H  

#define ALLOWED_TIME_SECONDS 120
#define DNS_PORT 53
#define WEBSERVER_PORT 80

#ifdef ALESP32C3
  #define ANALOGPIN2 2
  #define SRV_TURN 3
  #define SRV_SPEED 4 
  #define BTN_RESTART 2

#endif

#define SERVO     0
#define ACCEL_FACTOR  0.2
#define DEACCEL_FACTOR  0.6
#define TURN_FACTOR  0.5

#define MAX_MOTOR_SPEED   255
#define MAX_MOTOR_TURN    255

#define SPIFFS_SAVE_FILE "/playtime.dat"

void renderQRCode(uint8_t pos_x, uint8_t pos_y, const char *text);
void renderQRCode2x(uint8_t pos_x, uint8_t pos_y, const char *text, int version);
void generateWifiString(char *buf, char *ssid, char *password, const char *authType = "WPA2");
void renderWifiText(uint8_t pos_x, uint8_t pos_y, const char *ssid, const char *password);
void setupWebServer();
void handleHomePage();
void handleNotFound();
void captiveRedirect(AsyncWebServerRequest *request);
void captiveOK(AsyncWebServerRequest *request);
void setupMotors();
void updateMotors();
void updateDisplay();

#endif