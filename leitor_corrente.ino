#include <Wire.h>
#include <EmonLib.h>

#include <espnow.h>
#include <ESP8266WiFi.h>

EnergyMonitor emon1;

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 3
#define SAMPLING_TIME 0.0001668649
#define LINE_FREQUENCY 60

#define VOLTAGE_AC 127.00
//#define ACS_MPY 15.41 // Valor inicial para testes
#define ACS_MPY 100.00 // Valor alterado para calibração

double Irms = 0; 

//MAC Address of the receiver 
uint8_t broadcastAddress[] = {0x34, 0x94, 0x54, 0x61, 0xF8, 0x22};

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    int id;
    double cor1;
    double cor2;
    int readingId;
} struct_message;

//Create a struct_message called myData
struct_message myData;

unsigned long previousMillis = 0;   // Stores last time cor1erature was published
const long interval = 10000;        // Interval at which to publish sensor readings

unsigned int readingId = 0;

// Insert your SSID
constexpr char WIFI_SSID[] = "WELITON";

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
    for (uint8_t i=0; i<n; i++) {
      if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
        return WiFi.channel(i);
      }
    }
  }
  return 0;
}


double readcor1erature(double Irms) {
  double t = Irms;
  return t;
}

double readcor2idity(double Irms) {
  double h = Irms;
  return h;
}

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}
 
void setup() {
  //emon1.current(1,ACS_MPY);
  emon1.current(0,ACS_MPY);
  //Init Serial Monitor
  Serial.begin(115200);
  //initBME(); 

  // Set device as a Wi-Fi Station and set channel
  WiFi.mode(WIFI_STA);

  int32_t channel = getWiFiChannel(WIFI_SSID);

  WiFi.printDiag(Serial); // Uncomment to verify channel number before
  wifi_promiscuous_enable(1);
  wifi_set_channel(channel);
  wifi_promiscuous_enable(0);
  WiFi.printDiag(Serial); // Uncomment to verify channel change after

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  esp_now_register_send_cb(OnDataSent);
  
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}
 
void loop() {
  Irms = emon1.calcIrms(1000);
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;
    //Set values to send
    myData.id = BOARD_ID;
    myData.cor1 = readcor1erature(Irms);
    myData.cor2 = readcor2idity(Irms);
    myData.readingId = readingId++;
     
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    Serial.print("loop");
  }
}
