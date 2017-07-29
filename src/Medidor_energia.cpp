#include <Ticker.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

extern "C"{
#include "user_interface.h"
}

#define wifi_ssid "PAPAYA"
#define wifi_password "papaya2014"

#define mqtt_server "192.168.0.47"
#define mqtt_user "general"
#define mqtt_password "smolder79"

#define your_topic "medidor"

WiFiClient espClient;
PubSubClient client(espClient);

#define opt_off 10  //inserir a leitura do opto sensor quando o led desligado
//#define opt_on 50

os_timer_t mTimer;

int contador, leituras, cont_pulso;
bool led_pulso, led_pulso_ant=0, pisca;

//Nunca execute nada na interrupcao, apenas setar flags!
void tCallback(void *tCall){
    if(analogRead(A0)>opt_off) led_pulso = 1;  //testa o estado do led atraves do leitor otico
    else led_pulso = 0;

    if(!led_pulso_ant & led_pulso) cont_pulso++; //se mudou de 0 pra 1, incrementa
    led_pulso_ant = led_pulso;

    contador++;
}

//FUNCAO DE EXECUCAO COM ESTOURO DE TIMER
void usrInit(void){
    os_timer_setfn(&mTimer, tCallback, NULL);  //Chama a funcao tCallback a cada 10ms
    os_timer_arm(&mTimer, 10, true);  //10
}

#define LED_AZUL 2 // led azul na placa lolin nodemcu v3

int opt_101; //Tensao do leitor de luminosidade opt101

// Ticker ticker;
// void tick()
// {
//   toggle state
//   int state = digitalRead(LED_AZUL);  // get the current state of GPIO1 pin
//   digitalWrite(LED_AZUL, !state);     // set pin to the opposite state
//   pisca++;
// }
void setup_wifi();

void setup()
{
    Serial.begin(115200);
    pinMode(LED_AZUL, OUTPUT);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    // ticker.attach(0.1, tick);
    usrInit();
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

long lastMsg = 0;
int cont = 0;

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  Serial.print("pulsos: ");
  Serial.println(cont_pulso);
  Serial.print("leituras por segundo: ");
  Serial.println(contador);
  Serial.println(analogRead(A0));
  //Serial.println(leituras);
  //leituras++;
  contador=0;  //zera a contagem de interrupcoes por segundo q esta ocorrendo
  yield();

  digitalWrite(LED_AZUL, LOW);
  delay(10);
  digitalWrite(LED_AZUL, HIGH);
  delay(990);
  yield();

  long now = millis();

  if (now - lastMsg > 10000) {
    lastMsg = now;

    //cont += 1;

    client.publish(your_topic, String(cont_pulso).c_str(), true);

  }


}
