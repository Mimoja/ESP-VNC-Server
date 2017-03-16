
#include <ESP8266WiFi.h>

class VNCServer{
  private:
    String RFB_MAGIC = "RFB 003.003\n";
    enum{
        AUTH_FAILED = 0,
        AUTH_NONE = 1,
        AUTH_VNC = 2,
    };
    static const int MAX_NUM_CLIENTS = 8;
  
    WiFiServer server;
    WiFiClient clients[MAX_NUM_CLIENTS];
    int println(char* string){
      Serial.print("[VNC] ");
      return Serial.println(string);
    }

    bool writeVersion(WiFiClient client){
        println("Sending version to client");
        client.print(RFB_MAGIC);
    }
    
    bool readVersion(WiFiClient client){
      while(!client.available());
      String version = client.readStringUntil('\n');
      println("Client version:");
      println((char*)version.c_str());

      return version.equals(RFB_MAGIC);
    }
    bool writeAuthentificaton(WiFiClient client){
      client.print(0);
      client.print(0);
      client.print(0);
      client.print(0);
      client.print(AUTH_NONE);


    }
    bool handleNewClient(WiFiClient client){
      println("New client");
      writeVersion(client);
      readVersion(client);
      writeAuthentificaton(client);
      while(client.connected()){
        if(client.available()){
          char a = client.read();
          Serial.print(a, HEX);
        }
        }
      }
    
  public:
    VNCServer(int port):server(port){
      server.begin();
    }
    
    void handleClient(){
      WiFiClient client = server.available();
      if (client)handleNewClient(client);

    }
};

VNCServer vnc(5900);

const char* EX_SSID = "Northern Lights";
const char* PASSWD = "technikeristinformiert";

const char* events[] = {
    "WIFI_EVENT_STAMODE_CONNECTED ",
    "WIFI_EVENT_STAMODE_DISCONNECTED",
    "WIFI_EVENT_STAMODE_AUTHMODE_CHANGE",
    "WIFI_EVENT_STAMODE_GOT_IP",
    "WIFI_EVENT_STAMODE_DHCP_TIMEOUT",
    "WIFI_EVENT_SOFTAPMODE_STACONNECTED",
    "WIFI_EVENT_SOFTAPMODE_STADISCONNECTED",
    "WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED",
    "WIFI_EVENT_MAX",

};

void WiFiEvent(WiFiEvent_t event) {
    switch(event) {
        case WIFI_EVENT_STAMODE_GOT_IP:
            Serial.println("WiFi connected");
            Serial.println("IP address: ");
            Serial.println(WiFi.localIP());
            break;
        case WIFI_EVENT_STAMODE_DISCONNECTED:
            Serial.println("Wifi disconnected");
            break;
        case WIFI_EVENT_STAMODE_CONNECTED:
             Serial.println("Wifi connected");
        case WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED:
            break;
        default:
            Serial.printf("[WiFi-event] event: %d %s\n", event, events[event]);
            break;
    }
}

void setup() {
  Serial.begin(74880);
  delay(500);
  Serial.println();

  Serial.println("Connecting to Wifi");
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(EX_SSID, PASSWD);
}
  
void loop() {
  vnc.handleClient();
}


