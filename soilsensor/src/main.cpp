#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <b64.h>
#include <sha256.h>
#include <NTP.h>

// WIFI settings
static char ssid[] = "";
static char pass[] = "";

// Sensor stuff
int sensorPin = A0;
int sensorValue = 0;

// START: Azure Evet Hub settings
static const char* deviceId = "";
static char* KEY = "";
static const char* KEY_NAME = "PrimaryKey";
static const char* HOST = "";
// END: Azure Evet Hub settings

const char* TARGET_URL = "/devices/";
const char* IOT_HUB_END_POINT = "/messages/events?api-version=2015-08-15-preview";

char buffer[256];
int expire = 1511104241;
int sleepTimeS = 3600;

String fullSas;
int limit = 0;
int eventHubMsgCount = 0;

WiFiClientSecure tlsClient;

bool debugOn = false;

void initSerial() {
        Serial.begin(115200);
        Serial.setDebugOutput(true);
}

void initWifi() {
        Serial.print("\r\n\r\nAttempting to connect to SSID: ");
        Serial.println(ssid);
        WiFi.begin(ssid, pass);
        while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(".");
        }
        Serial.println("\r\nConnected to wifi");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
}

// http://hardwarefun.com/tutorials/url-encoding-in-arduino
String URLEncode(const char* msg)
{
        const char *hex = "0123456789abcdef";
        String encodedMsg = "";

        while (*msg!='\0') {
                if( ('a' <= *msg && *msg <= 'z')
                    || ('A' <= *msg && *msg <= 'Z')
                    || ('0' <= *msg && *msg <= '9') ) {
                        encodedMsg += *msg;
                } else {
                        encodedMsg += '%';
                        encodedMsg += hex[*msg >> 4];
                        encodedMsg += hex[*msg & 15];
                }
                msg++;
        }
        return encodedMsg;
}

void connectToAzure() {
        delay(500);
        Serial.print(deviceId);
        Serial.print(" connecting to ");
        Serial.println(HOST);
        if (WiFi.status() != WL_CONNECTED) { return; }
        if (!tlsClient.connect(HOST, 443)) {
                Serial.print("Host connection failed.  WiFi IP Address: ");
                Serial.println(WiFi.localIP());

                delay(2000);
        }
        else {
                Serial.println("Host connected");
                delay(250);
        }
}

String createIotHubSas(char *key, String url){
        String stringToSign = url + "\n" + expire;

        int keyLength = strlen(key);

        int decodedKeyLength = base64_dec_len(key, keyLength);
        char decodedKey[decodedKeyLength]; //allocate char array big enough for the base64 decoded key

        base64_decode(decodedKey, key, keyLength);

        Sha256.initHmac((const uint8_t*)decodedKey, decodedKeyLength);
        Sha256.print(stringToSign);
        char* sign = (char*) Sha256.resultHmac();

        int encodedSignLen = base64_enc_len(HASH_LENGTH);
        char encodedSign[encodedSignLen];
        base64_encode(encodedSign, sign, HASH_LENGTH);

        return "sr=" + url + "&sig="+ URLEncode(encodedSign) + "&se=" + expire;
}

String serializeData(int data){
        StaticJsonBuffer<JSON_OBJECT_SIZE(16)> jsonBuffer; //  allow for a few extra json fields that actually being used at the moment
        JsonObject& root = jsonBuffer.createObject();

        root["DeviceId"] = deviceId;
        root["Utc"] = GetISODateTime();
        root["Moisture"] = data;
        root["Id"] = ++eventHubMsgCount;

        root.printTo(buffer, sizeof(buffer));

        return (String)buffer;
}

String buildHttpRequest(String data){
        return "POST " + (String)TARGET_URL + (String)deviceId + (String)IOT_HUB_END_POINT + " HTTP/1.1\r\n" +
               "Host: " + HOST + "\r\n" +
               "Authorization: SharedAccessSignature " + fullSas + "\r\n" +
               "Content-Type: application/atom+xml;type=entry;charset=utf-8\r\n" +
               "Content-Length: " + data.length() + "\r\n\r\n" + data;
}

void publishToAzure(int data) {
        int bytesWritten = 0;

        // https://msdn.microsoft.com/en-us/library/azure/dn790664.aspx

        if (!tlsClient.connected()) { connectToAzure(); }
        if (!tlsClient.connected()) { return; }

        tlsClient.flush();

        bytesWritten = tlsClient.print(buildHttpRequest(serializeData(data)));

        String response = "";
        String chunk = "";
        int limit = 1;


        do {
                if (tlsClient.connected()) {
                        yield();
                        chunk = tlsClient.readStringUntil('\n');
                        response += chunk;
                }
        } while (chunk.length() > 0 && ++limit < 100);
        if (debugOn) {
        Serial.print("Bytes sent ");
        Serial.print(bytesWritten);
        Serial.print(" Message ");
        Serial.print(eventHubMsgCount);
        Serial.print(", Response chunks ");
        Serial.print(limit);
        Serial.print(", Response code: ");
      }

        if (response.length() > 12) { Serial.println(response.substring(9, 12)); }
        else { Serial.println("unknown"); }

}

void getCurrentTime(){
        int ntpRetryCount = 0;
        while (timeStatus() == timeNotSet && ++ntpRetryCount < 10) { // get NTP time
                Serial.println(WiFi.localIP());
                setSyncProvider(getNtpTime);
                setSyncInterval(60 * 60);
        }
}

void setup() {
        pinMode(5, OUTPUT);
        initSerial();
        initWifi();

        String blah = URLEncode(HOST) + URLEncode(TARGET_URL);
        fullSas =  createIotHubSas(KEY, blah);
        if (debugOn) {
        Serial.println();
        Serial.print(" Full SAS: ");
        Serial.println(fullSas);
      }
}

void loop() {
        digitalWrite(5, HIGH);
        delay(500);
        getCurrentTime();
        sensorValue = analogRead(sensorPin);
        if (debugOn) {
            Serial.print("Moisture: ");
            Serial.println(sensorValue);
        }
        digitalWrite(5, LOW);
        publishToAzure(sensorValue);
        if (debugOn) {
        Serial.println("starting sleep");
        delay(sleepTimeS * 10);
        ESP.getVcc();
        Serial.println("end of sleep");
      } else {
        ESP.deepSleep(sleepTimeS * 1000000);
      }
}
