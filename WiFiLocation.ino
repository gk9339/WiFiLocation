#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <wpa2_enterprise.h>
#include "serv_credentials.h" //defines HOMESSID, HOMEPASS, ENTSSID, ENTUSER, ENTPASS
#include "data_structures.h"

#define MAX_NETWORKS 50
#define MAX_SAMPLES 10

String message[MAX_SAMPLES];
int messageIndex = 0;

double RSSIarr[MAX_NETWORKS];
Array<String, MAX_NETWORKS> knownNetworks("");

ESP8266WebServer server(80);

char* ssid = NULL;
char* username = NULL;
char* password = NULL;

void setup()
{
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);

    while( ssid == NULL )
    {
        int networkCount = WiFi.scanNetworks();
        for( int i = 0; i < networkCount; i++ )
        {
            if( WiFi.SSID(i).equals(HOMESSID) )
            {
                ssid = HOMESSID;
                password = HOMEPASS;
                break;
            }
            if( WiFi.SSID(i).equals(ENTSSID) )
            {
                ssid = ENTSSID;
                username = ENTUSER;
                password = ENTPASS;
          
                //WPA2-enterprise PEAP setup
                struct station_config wifi_config;
                memset(&wifi_config, 0, sizeof(wifi_config));

                strcpy((char*)wifi_config.ssid, ssid);

                wifi_set_opmode(STATION_MODE);
                wifi_station_set_config(&wifi_config);
                wifi_station_clear_cert_key();
                wifi_station_clear_enterprise_ca_cert();
                wifi_station_set_wpa2_enterprise_auth(1);
                wifi_station_set_enterprise_identity((uint8*)username, strlen(username));
                wifi_station_set_enterprise_username((uint8*)username, strlen(username));
                wifi_station_set_enterprise_password((uint8*)password, strlen(password));
                wifi_station_connect();
                break;
            }
        }

        delay(5000); //Neither home or enterprise network found
    }
    
    WiFi.mode(WIFI_STA);
    Serial.println("");
    WiFi.begin(ssid,password);
    
    digitalWrite(LED_BUILTIN, LOW);
    while( WiFi.status() != WL_CONNECTED )
    {
        delay(500);
        Serial.print(".");
    }    
    digitalWrite(LED_BUILTIN, HIGH);
    
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if( MDNS.begin("esp8266") )
    {
        Serial.println("mDNS responder started");
    }

    server.on("/", handleIndex);
    server.on("/scanNetworks", scanNetworks);
    server.on("/measure", scanLoop);

    server.begin();
    Serial.println("HTTP server started");
}

void loop()
{
    server.handleClient();
    MDNS.update();
}

void handleIndex()
{
    digitalWrite(LED_BUILTIN, LOW);
    String temp = "<html>\n\    <head>\n\
        <title>ESP8266 WiFiLocation test</title>\n\
    <\head>\n    <body>\n\
        <a href=\"/scanNetworks\" style=\"font-size:100px\">scan networks</a><br>\n\
        <a href=\"/measure\" style=\"font-size:100px\">take measurement</a>\n\
    </body>\n</html>";
    server.send(200, "text/html", temp);
    digitalWrite(LED_BUILTIN, HIGH);
}

void scanLoop()
{
    scanRSSIs();
    printRSSIs();
}

void scanRSSIs()
{
    digitalWrite(LED_BUILTIN, LOW);
    int networkCount = WiFi.scanNetworks();

    resetRSSIs();

    for( int i = 0; i < networkCount; i++ )
    {
        String bssid = WiFi.BSSIDstr(i);
        uint16_t networkIndex = knownNetworks.indexOf(bssid);

        if( networkIndex != 65535 )
        {
            RSSIarr[networkIndex] = WiFi.RSSI(i);
        }
    }
    digitalWrite(LED_BUILTIN, HIGH);
}

void printRSSIs()
{
    digitalWrite(LED_BUILTIN, LOW);
    int knownNetCount = knownNetworks.length();
    String temp;

    if( messageIndex < MAX_SAMPLES )
    {
        message[messageIndex] = "";
        for( int i = 0; i < knownNetCount; i++ )
        {
            message[messageIndex] += RSSIarr[i];
            if( i != knownNetCount - 1 )
            {
                message[messageIndex] += ',';
            }
        }
        messageIndex++;
    }
    
    temp = "<html>\n    <head>\n";
    if( messageIndex < MAX_SAMPLES )
    {
        temp += "        <meta http-equiv='refresh' content='2'/>\n";
    }
    temp += "        <title>ESP8266 WiFiLocation test</title>\n\
    </head>\n    <body>\n\
        <a href=\"/\" style=\"font-size:100px\">\<-back</a><br>\n        <p>";
    for( int i = 0; i < knownNetCount; i++ )
    {
        temp += knownNetworks[i];
        temp += i == knownNetCount - 1 ? "</p>\n" : ",";
    }
    for( int i = 0; i < messageIndex; i++ )
    {
        temp += "        <p>"+message[i]+"<br></p>\n";
    }
    temp += "    </body>\n</html>";

    server.send(400, "text/html", temp);
    digitalWrite(LED_BUILTIN, HIGH);
}

void resetRSSIs()
{
    const uint16_t RSSIcount = sizeof(RSSIarr) / sizeof(double);

    for( int i = 0; i < RSSIcount; i++ )
    {
        RSSIarr[i] = 0;
    }
}

void scanNetworks()
{
    digitalWrite(LED_BUILTIN, LOW);
    int networkCount = WiFi.scanNetworks();
    String temp, bssid;

    messageIndex = 0;

    for( int i = 0; i < networkCount; i++ )
    {
        bssid = WiFi.BSSIDstr(i);
        message[i] = bssid;
        message[i] += " (";
        message[i] += WiFi.RSSI(i); 
        message[i] += ") - ";
        message[i] += WiFi.SSID(i);

        if( WiFi.SSID(i).equals("student-curtin") || 
            WiFi.SSID(i).equals("eduroam") ||
            WiFi.SSID(i).equals("staff-curtin") ||
            WiFi.SSID(i).equals("CurtinGuest") ||
            WiFi.SSID(i).equals("Kottler") ||
            WiFi.SSID(i).equals("RPi") ||
            WiFi.SSID(i).equals("WAVLINK-N") )
        {
            uint16_t networkIndex = knownNetworks.indexOf(bssid);
            if( networkIndex == 65535 )
            {
                knownNetworks.push(bssid);
            }
        }
    }
    int knownNetCount = knownNetworks.length();
    
    temp = "<html>\n    <head>\n\
        <title>ESP8266 WiFiLocation test</title>\n\
    </head>\n    <body>\n\
        <a href=\"/\" style=\"font-size:100px\">\<-back</a><br>\n";
    for( int i = 0; i < networkCount; i++ )
    {
        temp += "        <p>"+message[i]+"<br></p>\n";
        message[i] = "";
    }
    temp += "        <br>\n        <p>";
    for( int i = 0; i < knownNetCount; i++ )
    {
        temp += knownNetworks[i];
        temp += i == knownNetCount - 1 ? "</p>\n" : ",";
    }
    temp += "    </body>\n</html>";

    server.send(400, "text/html", temp);
    digitalWrite(LED_BUILTIN, HIGH);
}
