#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#if defined(ESP8266)
    #include <ESP8266WiFi.h>
#elif defined(ESP32)
    #include <WiFi.h>
#endif

#include "config.h"
#include "msp.h"

#define UART_TIME_OUT 5
#define STANDARD_BAUD 115200

typedef enum { 
    TCP, 
    UDP 
} Protocoll_e;

WiFiServer *wifiServer;
WiFiClient wifiClient;
WiFiUDP *wifiUdp;
IPAddress ipAddress;
Configuration config;
Protocoll_e protocoll;
MSP mspUart;
MSP mspWifi;

void setup() {
    config.setupWebConfig();

    unsigned long baudRate = atoi(config.baudrateParam.value());
    if (baudRate != 0) {
        Serial.begin(baudRate);
    } else {
        Serial.begin(STANDARD_BAUD);
    }

    Serial.println("Protocoll: ");
    if (strcmp(config.protocolParam.value(), "udp") == 0) {
        wifiUdp = new WiFiUDP();
        wifiUdp->begin(config.portParam.value());
        protocoll = UDP;
        Serial.println("UDP");
    } else {
        wifiServer = new WiFiServer(config.portParam.value());
        wifiServer->begin();
        protocoll = TCP;
        Serial.println("TCP");
    } 
}

void loop() {
    config.doLoop();

    if (protocoll == TCP) {
       
        if (!wifiClient.connected()) {
            wifiClient = wifiServer->available();
            return;
        }
        
        while (!mspWifi.isframeValid() && wifiClient.available()) {
            mspWifi.readByte((uint8_t)wifiClient.read());
        }
        

    } else if (protocoll == UDP) {
        int packetSize = wifiUdp->parsePacket();
        if (packetSize > 0) {
            ipAddress = wifiUdp->remoteIP();
            while (!mspWifi.isframeValid() && wifiUdp->available()) {
                mspWifi.readByte((uint8_t)wifiUdp->read());
            }
        }
    }

    if (mspWifi.isframeValid()) {
        Serial.write(mspWifi.frame, mspWifi.getLength());
    }
    mspUart.setCliMode(mspWifi.isCliMode());
    mspWifi.reset();

    while (!mspUart.isframeValid()) {
        if (Serial.available()) {
            mspUart.readByte((uint8_t)Serial.read());
        } else {
            delay(UART_TIME_OUT);
            if (!Serial.available()) {
                break;
            }
        }
    }

    if (mspUart.isframeValid()) {
        if (protocoll == TCP) {
            wifiClient.write(mspUart.frame, mspUart.getLength());
        } else if (protocoll == UDP) {
            wifiUdp->beginPacket(ipAddress, config.portParam.value());
            wifiUdp->write(mspUart.frame, mspUart.getLength());
            wifiUdp->endPacket();
        }

    }
    mspWifi.setCliMode(mspUart.isCliMode());
    mspUart.reset();
}
