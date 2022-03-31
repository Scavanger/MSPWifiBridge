#pragma once

#include <IotWebConf.h>
#include <IotWebConfTParameter.h>

#if defined(ESP8266)
    #include <ESP8266HTTPUpdateServer.h>
#elif defined(ESP32)
    #include <IotWebConfESP32HTTPUpdateServer.h>
#endif


// --- These values may be changed
#define CONFIG_VERSION "0.3"
#define TCP_PORT 5761
#define NAME "MSPWifiBridge"
#define INITAL_WIFI_PASSWORD "123456789"
// ---

#define STRING_LEN 128
#define NUMBER_LEN 32
#define CONFIG_PIN 0
#define STATUS_PIN LED_BUILTIN

using namespace iotwebconf;

class Configuration {
    private:
        static const char protocolValues[][STRING_LEN];
        static const char baudrateValues[][STRING_LEN];
        static ParameterGroup protocollGroup;
        static ParameterGroup serialGroup;
        
        #if defined(ESP8266)
            ESP8266HTTPUpdateServer *httpUpdater;
        #elif defined(ESP32)
            HTTPUpdateServer *httpUpdater;
        #endif
            
        DNSServer *dnsServer;
        WebServer *webServer;
        IotWebConf *iotWebConf;

        void handleRoot();
        void saved();  
    public:
        static SelectTParameter<STRING_LEN> protocolParam;
        static IntTParameter<uint16_t> portParam;
        static SelectTParameter<STRING_LEN> baudrateParam;
        
        Configuration();
        void setupWebConfig(void);
        void doLoop(void);
        IPAddress getIP();
};
