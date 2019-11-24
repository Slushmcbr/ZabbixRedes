#include <SPI.h>
#include <Ethernet.h>
#include "DHT.h"

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

float temperatura;
float umidade;
float variavel1, variavel2;

const char* server = "192.168.15.12";
int porta = 10051;
IPAddress ip(192, 168, 15, 20);
IPAddress mydns(192, 168, 0, 1);
EthernetClient client;
long previousMillis = 0;
long interval = 1000;
void zabbix_sender(void);
DHT dht(3, DHT11);

void setup() {
  Serial.begin(9600);
  Serial.print("conectando na rede");
  delay(1000);
  Ethernet.begin(mac, ip, mydns);
  delay(1000);
  Serial.print("IP: \n");
  Serial.print(Ethernet.localIP());
  Serial.print("\n");
  dht.begin();
}

void loop() { 
  temperatura = dht.readTemperature();
  umidade = dht.readHumidity();
    
    if (isnan(umidade) || isnan(temperatura)) {
    Serial.println("Falha na leitura do Sensor DHT!");

  }

  
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;

    variavel1 = temperatura;
    variavel2 = umidade;

    zabbix_sender();
  }
}

void zabbix_sender(void) {
  String host = "ethernet";
  String itens[] = {"variavel1", "variavel2"};
  float valores [] = {variavel1, variavel2};
  for (int i = 0; i < (sizeof(valores) / sizeof(float)); i++)

    if (client.connect(server, porta)) {
      Serial.print("Conectado no Zabbix!\n");
      String zabbix = "";

      zabbix += String("{\"request\":\"sender data\", \"data\":");
      zabbix += String("[{");
      zabbix+= String("\"host\":" ) + String("\"") + String (host) + String ("\"") + String(",");
      zabbix += String ("\"key\":") + String("\"") + String(itens[i]) + String("\"") + String(",");
      zabbix += String("\"value\":") + String("\"") + String(valores[i]) + String("\"");
      zabbix += String("}]}");

      Serial.print("Post via Zabbix Sender:");
      Serial.println(zabbix);

      client.print(zabbix);

      zabbix = "";

      unsigned long timeout = millis();
      while (client.available() == 0){
        if (millis() - timeout > 5000) {
          Serial.println(">>> Timeout!");
          client.stop();
          return;
        }
      }

      while (client.available()) {
        String resposta = client.readStringUntil('\r');
      }
    }
    client.stop();
}
