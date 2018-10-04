//
// Created by scottw on 10/4/18.
//

#ifndef Arduino_
#include <Arduino.h>
#endif


/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial


#include <FS.h> //  Settings saved to SPIFFS
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h> // required for settings file to make it readable

#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"

#include <ESPmanager.h>

ESP8266WebServer HTTP(80);

ESPmanager settings(HTTP, SPIFFS, "ESPManager");


// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "YourNetworkName";
char pass[] = "YourPassword";

// To run, set your ESP8266 build to 160MHz, and include a SPIFFS of 512KB or greater.
// Use the "Tools->ESP8266/ESP32 Sketch Data Upload" menu to write the MP3 to SPIFFS
// Then upload the sketch normally.

// pno_cs from https://ccrma.stanford.edu/~jos/pasp/Sound_Examples.html

AudioGeneratorMP3 *mp3;
AudioFileSourceSPIFFS *file;
AudioOutputI2SNoDAC *out;
AudioFileSourceID3 *id3;

// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
    (void)cbData;
    Serial.printf("ID3 callback for: %s = '", type);

    if (isUnicode) {
        string += 2;
    }

    while (*string) {
        char a = *(string++);
        if (isUnicode) {
            string++;
        }
        Serial.printf("%c", a);
    }
    Serial.printf("'\n");
    Serial.flush();
}

void mp3Play() {
    Serial.printf("Sample MP3 playback begins...\n");
    file = new AudioFileSourceSPIFFS("/pno-cs.mp3");
    id3 = new AudioFileSourceID3(file);
    id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
    out = new AudioOutputI2SNoDAC();
    mp3 = new AudioGeneratorMP3();
    mp3->begin(id3, out);
}

void setup()
{
    Serial.begin(115200);
    SPIFFS.begin();

    Serial.println("");
    Serial.println(F("Example ESPconfig"));

    Serial.printf("Sketch size: %u\n", ESP.getSketchSize());
    Serial.printf("Free size: %u\n", ESP.getFreeSketchSpace());

    settings.begin();

    HTTP.begin();

    Serial.print(F("Free Heap: "));
    Serial.println(ESP.getFreeHeap());

    Blynk.begin(auth, ssid, pass);
}

void loop()
{
    Blynk.run();
    HTTP.handleClient();
    settings.handle();
    if (mp3->isRunning()) {
        if (!mp3->loop()) mp3->stop();
    }
}
