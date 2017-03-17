
#include <ESP8266WiFi.h>

class VNCServer{
  private:
    String RFB_MAGIC = "RFB 003.003\n";
    enum{
        AUTH_FAILED = 0x00,
        AUTH_NONE = 0x01,
        AUTH_VNC = 0x02,
    };
    static const int MAX_NUM_CLIENTS = 8;

    /** Key events: 
     *            +-----------------+--------------------+
                 | Key name        | Keysym value (hex) |
                 +-----------------+--------------------+
                 | BackSpace       | 0xff08             |
                 | Tab             | 0xff09             |
                 | Return or Enter | 0xff0d             |
                 | Escape          | 0xff1b             |
                 | Insert          | 0xff63             |
                 | Delete          | 0xffff             |
                 | Home            | 0xff50             |
                 | End             | 0xff57             |
                 | Page Up         | 0xff55             |
                 | Page Down       | 0xff56             |
                 | Left            | 0xff51             |
                 | Up              | 0xff52             |
                 | Right           | 0xff53             |
                 | Down            | 0xff54             |
                 | F1              | 0xffbe             |
                 | F2              | 0xffbf             |
                 | F3              | 0xffc0             |
                 | F4              | 0xffc1             |
                 | ...             | ...                |
                 | F12             | 0xffc9             |
                 | Shift (left)    | 0xffe1             |
                 | Shift (right)   | 0xffe2             |
                 | Control (left)  | 0xffe3             |
                 | Control (right) | 0xffe4             |
                 | Meta (left)     | 0xffe7             |
                 | Meta (right)    | 0xffe8             |
                 | Alt (left)      | 0xffe9             |
                 | Alt (right)     | 0xffea             |
                 +-----------------+--------------------+
                 The interpretation of keysyms is a complex area.  In order to be as
   widely interoperable as possible, the following guidelines should be
   followed:

   o  The "shift state" (i.e., whether either of the Shift keysyms is
      down) should only be used as a hint when interpreting a keysym.
      For example, on a US keyboard the '#' character is shifted, but on
      a UK keyboard it is not.  A server with a US keyboard receiving a
      '#' character from a client with a UK keyboard will not have been
      sent any shift presses.  In this case, it is likely that the
      server will internally need to simulate a shift press on its local
      system in order to get a '#' character and not a '3'.

   o  The difference between upper and lower case keysyms is
      significant.  This is unlike some of the keyboard processing in
      the X Window System that treats them as the same.  For example, a
      server receiving an upper case 'A' keysym without any shift
      presses should interpret it as an upper case 'A'.  Again this may
      involve an internal simulated shift press.

   o  Servers should ignore "lock" keysyms such as CapsLock and NumLock
      where possible.  Instead, they should interpret each character-
      based keysym according to its case.

   o  Unlike Shift, the state of modifier keys such as Control and Alt
      should be taken as modifying the interpretation of other keysyms.
      Note that there are no keysyms for ASCII control characters such
      as Ctrl-A -- these should be generated by clients sending a
      Control press followed by an 'a' press.

   o  On a client where modifiers like Control and Alt can also be used
      to generate character-based keysyms, the client may need to send
      extra "release" events in order that the keysym is interpreted
      correctly.  For example, on a German PC keyboard, Ctrl-Alt-Q
      generates the '@' character.  In this case, the client needs to
      send simulated release events for Control and Alt in order that
      the '@' character is interpreted correctly, since Ctrl-Alt-@ may
      mean something completely different to the server.

   o  There is no universal standard for "backward tab" in the X Window
      System.  On some systems shift+tab gives the keysym
      "ISO_Left_Tab", on others it gives a private "BackTab" keysym, and
      on others it gives "Tab" and applications tell from the shift
      state that it means backward-tab rather than forward-tab.  In the
      RFB protocol, the latter approach is preferred.  Clients should
      generate a shifted Tab rather than ISO_Left_Tab.  However, to be
      backwards-compatible with existing clients, servers should also
      recognize ISO_Left_Tab as meaning a shifted Tab.

   o  Modern versions of the X Window System handle keysyms for Unicode
      characters, consisting of the Unicode character with the hex
      1000000 bit set.  For maximum compatibility, if a key has both a
      Unicode and a legacy encoding, clients should send the legacy
      encoding.

   o  Some systems give a special interpretation to key combinations
      such as Ctrl-Alt-Delete.  RFB clients typically provide a menu or
      toolbar function to send such key combinations.  The RFB protocol
      does not treat them specially; to send Ctrl-Alt-Delete, the client
      sends the key presses for left or right Control, left or right

     */
  
    WiFiServer server;
    WiFiClient clients[MAX_NUM_CLIENTS];
    int println(char* string){
      Serial.print("[VNC] ");
      return Serial.println(string);
    }

    void write16(WiFiClient client, int val){
      client.write((uint8_t)(val >> 8)& 0xFF);
      client.write((uint8_t)(val & 0xFF) ); 
    }
    char read8(WiFiClient client){
      while(!client.available());
      return client.read();
      }

    int read16(WiFiClient client){
     return (((read8(client) & 0xFF) << 8) | (read8(client) & 0xFF)); 
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
      println("Sending Auth");

      client.write((uint8_t)0);
      client.write((uint8_t)0);
      client.write((uint8_t)0);
      client.write((uint8_t)AUTH_NONE);

    }
    bool readAuthentificaton(WiFiClient client){
      while(!client.available());
      char auth = read8(client);
      println("Client auth:");
      Serial.print("[VNC] ");
      Serial.println(auth, HEX);
      
      return auth == AUTH_NONE;
    }
    bool writeInit(WiFiClient client){
      println("Sending Server init");
      write16(client, 800);
      write16(client, 600);
      
      client.write((uint8_t)32); // bits-per-pixel
      client.write((uint8_t)32); // depth
      client.write((uint8_t)0); // big-endian
      client.write((uint8_t)1); // true-color
      
      write16(client, 0xFF);
      write16(client, 0xFF);
      write16(client, 0xFF);
      
      client.write((uint8_t)16);
      client.write((uint8_t)8);
      client.write((uint8_t)0);
      
      client.write((uint8_t)0);
      client.write((uint8_t)0);
      client.write((uint8_t)0);

      client.write((uint8_t)0);
      client.write((uint8_t)0);
      client.write((uint8_t)0);
      client.write((uint8_t)strlen("name"));
      client.print("name");
    }


    bool readShared(WiFiClient client){
      while(!client.available());
      char s = read8(client);
      println("Client auth:");
      Serial.print("[VNC] ");
      Serial.println(s, HEX);

      if(!s)println("Client does not want to share - LOL");
      
      return true;
    }
    
    bool handleNewClient(WiFiClient client){
      println("New client");
      writeVersion(client);
      readVersion(client);
      writeAuthentificaton(client);
      readShared(client);
      writeInit(client);

      while(client.connected()){
        if(client.available()){
          char type = read8(client);
          Serial.print("0x");
          Serial.println(type, HEX);

            if (type == 0x0) {
                 println("Got SetPixelFormat");
                 read8(client);
                 read8(client);
                 read8(client);

                int bits_per_pixel = read8(client);
                int depth = read8(client);
                int big_endian = read8(client);
                int true_color = read8(client);
                int red_maximum = read16(client);
                int green_maximum = read16(client);
                int blue_maximum = read16(client);
                int red_shift = read8(client);
                int green_shift = read8(client);
                int blue_shift = read8(client);

                int padding[3];
                for(int i = 0; i < 3; i++){
                  padding[i] = read8(client);
                }
              }
              else  if (type == 2) {
                println("Got SetEncoding");
                read8(client);
                int num_encodings = read16(client);
                println("Got num encodings:");
                Serial.println(num_encodings);
                println("Encoding:");
                for(int i = 0; i < num_encodings; i++){
                  uint32_t encoding_type = read16(client)<< 16 | read16(client);  
                  Serial.print(" 0x");
                  Serial.print(encoding_type, HEX);
                }
                Serial.println();
              }
              else if (type == 3) {
                 println("Got FrameBufferUpdateRequest");
                 int incremental = read8(client);
                 int x_position = read16(client);
                 int y_position = read16(client);
                 int width = read16(client);
                 int height = read16(client);
              }
              else if (type == 4) {
                 println("Got KeyEvent");
                 int down_flag = read8(client);
                 read16(client);
                 int key = read16(client)<< 16 | read16(client);  
                 println("Got Key");
                 if(key & 0xFF00 )Serial.print(key);
                 else{
                  Serial.print("0x");
                  Serial.println(key, HEX);
                 }
              }
              else if (type == 5) {
                println("Got PointerEvent");
                int button_mask = read8(client);
                int x_position = read16(client);
                int y_position = read16(client);
              }
              else if (type == 6) {
                 println("Got ClientCutText");
                 read8(client);
                 read8(client);
                 read8(client);
                 int length = read16(client)<< 16 | read16(client);
                 for(int i = 0; i < length; i++) Serial.print(read8(client));
                 Serial.println();
              }
              else {
                Serial.print("[VNC] ");
                Serial.print("Unknown message type. Received message type 0x" );
                Serial.println(type, HEX);
              }
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


