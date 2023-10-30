#include <WiFi.h>
#include <HTTPClient.h>
#define ARDUINOJSON_DECODE_UNICODE 1
#include <ArduinoJson.h>

//WiFi
const char *ssid = "Resonent_G_1F";
const char *password = "9574960011";

const char *url = "https://raw.githubusercontent.com/uPesy/ESP32_Tutorials/master/JSON/bigJsonExample.json";

struct SpiRamAllocator {
        void* allocate(size_t size) {
                return ps_malloc(size);

        }
        void deallocate(void* pointer) {
                free(pointer);
        }
};

using SpiRamJsonDocument = BasicJsonDocument<SpiRamAllocator>;


void setup(){
        delay(500);
        psramInit();
        Serial.begin(115200);
        Serial.println((String)"Memory available in PSRAM : " +ESP.getFreePsram());

        //Connect to WiFi

        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED){
                delay(100);
                Serial.print(".");
        }

        Serial.print("\nWiFi connected with IP : ");
        Serial.println(WiFi.localIP());

        Serial.println("Downloading JSON");
        HTTPClient http;
        http.useHTTP10(true);
        http.begin(url);
        http.GET();

        SpiRamJsonDocument doc(100000); //Create a JSON document of 100 KB
        DeserializationError error = deserializeJson(doc, http.getStream());
        if(error){
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
        }

        http.end();
        Serial.println((String)"JsonDocument Usage Memory: " + doc.memoryUsage());


        for (int i=0; i!=10;i++){
                Serial.println("\n[+]");
                Serial.println(doc["items"][i]["title"].as<String>());
                Serial.println("------------------------------");
                Serial.println(doc["items"][i]["snippet"].as<String>());
                Serial.println("------------------------------");
                Serial.println((String) "URL : " + doc["items"][i]["link"].as<String>());

        }
}

void loop(){

  
}
