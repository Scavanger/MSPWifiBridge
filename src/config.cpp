#include "config.h"

const char Configuration::protocolValues[][STRING_LEN] = {"tcp", "udp"};
const char Configuration::baudrateValues[][STRING_LEN] = {"9600",  "19200", "38400", "57600", "115200"};

SelectTParameter<STRING_LEN> Configuration::protocolParam =
    Builder<SelectTParameter<STRING_LEN>>("Protocoll").
    label("Protocoll").
    optionValues((const char*)protocolValues).
    optionNames((const char*)protocolValues).
    optionCount(sizeof(protocolValues) / STRING_LEN).
    nameLength(STRING_LEN).
    defaultValue("TCP").
    build();

IntTParameter<uint16_t> Configuration::portParam =
    Builder<IntTParameter<uint16_t>>("Port").
    label("Port").
    defaultValue(5761).
    min(1).
    max(65535).
    step(1).
    placeholder("5761").
    build();

SelectTParameter<STRING_LEN> Configuration::baudrateParam =
    Builder<SelectTParameter<STRING_LEN>>("Baudrate").
    label("Baudarate").
    optionValues((const char*)baudrateValues).
    optionNames((const char*)baudrateValues).
    optionCount(sizeof(baudrateValues) / STRING_LEN).
    nameLength(STRING_LEN).
    defaultValue("115200").
    build();

ParameterGroup Configuration::protocollGroup = ParameterGroup("protocoll", "Protocoll");
ParameterGroup Configuration::serialGroup = ParameterGroup("serial", "Serial");    

Configuration::Configuration() {
    dnsServer = new DNSServer();
    webServer = new WebServer(80);
    iotWebConf = new IotWebConf(NAME, dnsServer, webServer, INITAL_WIFI_PASSWORD, CONFIG_VERSION);
    
    #if defined(ESP8266)
        httpUpdater = new ESP8266HTTPUpdateServer();
    #elif defined(ESP32)
        httpUpdater = new HTTPUpdateServer();
    #endif
}

void Configuration::setupWebConfig(void) {
    protocollGroup.addItem(&protocolParam);
    protocollGroup.addItem(&portParam);
    serialGroup.addItem(&baudrateParam);

    iotWebConf->setStatusPin(STATUS_PIN);
    iotWebConf->setConfigPin(CONFIG_PIN);
    iotWebConf->setupUpdateServer(
        [this](const char* updatePath) { httpUpdater->setup(webServer, updatePath); },
        [this](const char* userName, char* password) { httpUpdater->updateCredentials(userName, password); }
    );
    iotWebConf->addParameterGroup(&protocollGroup);
    iotWebConf->addParameterGroup(&serialGroup);
    iotWebConf->setConfigSavedCallback([this]{saved();});
    iotWebConf->getApTimeoutParameter()->visible = true;
    iotWebConf->init();
    delay(2000);

    webServer->on("/", [this]{handleRoot();});
    webServer->on("/config", [this]{iotWebConf->handleConfig();});
    webServer->onNotFound([this]{iotWebConf->handleNotFound();});
}

void Configuration::doLoop()
{
    iotWebConf->doLoop();
}

IPAddress Configuration::getIP() {
    IPAddress ip;
    if (iotWebConf->getState() == NetworkState::ApMode || iotWebConf->getState() == NetworkState::NotConfigured) {
        ip = WiFi.softAPIP();
    } else {
        ip = WiFi.localIP();
    }
    return ip;
}

void Configuration::handleRoot()
{
    if (iotWebConf->handleCaptivePortal()) {
        return;
    }
    String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
    s += "<title>MSPWifiBridge</title></head><body>";
    s += "<h2>Welcome to MSPWifiBridge</h2>";
    s += "You can reach me under ";
    s += protocolParam.value();
    s += "://";
    s += getIP().toString();
    s += ":";
    s += portParam.value();
    s += "<br>Go to <a href='config'>configure page</a> to change values.";
    s += "</body></html>\n";

    webServer->send(200, "text/html", s);
}

void Configuration::saved()
{
  Serial.println("Configuration was updated.</br>");
}
