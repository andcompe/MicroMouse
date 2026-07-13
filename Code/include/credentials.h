#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#define MOUSEID 0

struct WiFiCredential {
    const char* ssid;
    const char* password;
};

constexpr WiFiCredential wifiCredentials[] = {
    {"Mouse", "ABCD1234"}, 
};

#endif
