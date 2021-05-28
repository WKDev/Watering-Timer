//Watering IoT Unit
//Coded May 28, 2021
// Supports :
//OTA Upload
//HTTP Server(once it triggered, it turns on for 30 mins )
// 192.168.0.5 ACUNIT // it should be updated

//!!!IMPORTANT
// Low-Level Relay is used for this project. which means, if code below written 'LOW', Relay turns on. and vise versa.
// if you are trying to use common High-Level Relay, you should change LOW to HIGH, HIGH to LOW.

//Arduino OTA
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>

// HTTPServer
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID
#define STASSID "IoT_System"
#define STAPSK "nodemcu01"
#endif

#define RELAY 12 // D6 12
#define BUTTON 14

#define UNIT_TYPE "Watering Timer"

const char *ssid = STASSID;
const char *password = STAPSK;

ESP8266WebServer server(80);

int triggerState = 0;
unsigned long startTime = 0;
unsigned long endTime = 0;

void handleRoot()
{
  digitalWrite(LED_BUILTIN, HIGH);
  String message = "<h1>Welcome to Cranberry-IoT END UNIT.</h1>";
  message += "<h2>This is " + String(UNIT_TYPE) + " Unit</h2>";
  message += "\n</p><p>Arguments: ";
  message += server.args();
  message += "</p>";
  message += "<p>local IP : " + WiFi.localIP().toString() + "</p>";
  if (triggerState)
  {
    message += "<p>Watering...";
    message += (startTime + 1000 * 60 * 30 - millis()) / (1000 * 60);
    message += " min(s) ";
    message += (startTime + 1000 * 60 * 30 - millis()) / (1000) - floor(startTime + 1000 * 60 * 30 - millis()) / (1000 * 60);
    message += " sec left.</p>";
  }
  else
  {
    float sinceDay = endTime / (1000 * 60 * 60 * 24);
    float sinceHour = endTime / (1000 * 60 * 60);
    if (sinceHour >= 24)
    {
      sinceHour = sinceHour - (endTime / (1000 * 60 * 60 * 24));
    }

    if (sinceDay < 1)
    {
      message += "<p>It's been";
      message += sinceHour;
      message += " hours";
      message += " since last watered"
                 "</p>";
    }
    else
    {
      message += "<p>It's been ";
      message += sinceDay;
      message += "Days ";
      message += sinceHour;
      message += " since last watered"
                 "</p>";
    }
  }
  message += "<br><a href = '/on'> on</ a><br><a href = '/off'>[off]</ a>";

  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(200, "text/html", message);
}

// Define routing
void restServerRouting()
{
  server.on("/", HTTP_GET, handleRoot);
  server.on("/on", []()
            {
              triggerState = 1;
              startTime = millis();
              server.send(200, "text/html", "<a href='/on'>[on]</a><br><a href='/off'>off</a>");
            });

  server.on("/off", []()
            {
              triggerState = 0;
              server.send(200, "text/html", "<a href='/on'>on</a><br><a href='/off'>[off]</a>");
            });
}

// Manage not found URL
void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup()
{
  //WiFi CONNECTION
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(1000);

    ESP.restart();
  }
  ArduinoOTA.setPassword("admin");

  ArduinoOTA.onStart([]()
                     {
                       String type;
                       if (ArduinoOTA.getCommand() == U_FLASH)
                       {
                         type = "sketch";
                       }
                       else
                       { // U_FS
                         type = "filesystem";
                       }

                       // NOTE: if updating FS this would be the place to unmount FS using FS.end()
                       Serial.println("Start updating " + type);
                     });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
                       Serial.printf("Error[%u]: ", error);
                       if (error == OTA_AUTH_ERROR)
                       {
                         Serial.println("Auth Failed");
                       }
                       else if (error == OTA_BEGIN_ERROR)
                       {
                         Serial.println("Begin Failed");
                       }
                       else if (error == OTA_CONNECT_ERROR)
                       {
                         Serial.println("Connect Failed");
                       }
                       else if (error == OTA_RECEIVE_ERROR)
                       {
                         Serial.println("Receive Failed");
                       }
                       else if (error == OTA_END_ERROR)
                       {
                         Serial.println("End Failed");
                       }
                     });
  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println("Ready to get OTAUpdate.");

  Serial.println(WiFi.localIP());

  // Activate mDNS this is used to be able to connect to the server
  // with local DNS hostmane esp8266.local
  if (MDNS.begin("esp8266"))
  {
    Serial.println("MDNS responder started");
  }

  // Set server routing
  restServerRouting();
  // Set not found response
  server.onNotFound(handleNotFound);
  // Start server
  server.begin();
  Serial.println("HTTP server started");

  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);
}

void loop()
{

  ArduinoOTA.handle();
  server.handleClient();

  if (triggerState)
  {
    if (millis() < (startTime + 1000 * 60 * 30)) // this is important parameter.
    //as modifiy this syntax, watering duration adjusted.
    {
      digitalWrite(RELAY, LOW);
    }
    else
    {
      digitalWrite(RELAY, HIGH);
      triggerState = 0;
      endTime = millis();
    }
    Serial.print("Watering... ");
    Serial.print((startTime + 1000 * 60 * 30 - millis()));
    Serial.println(" left.");
  }
}