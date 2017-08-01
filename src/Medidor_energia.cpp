#include <Ticker.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

extern "C"{
#include "user_interface.h"
}

#define wifi_ssid "PAPAYA"
#define wifi_password "papaya2014"

#define mqtt_server "192.168.0.60"
#define mqtt_user "general"
#define mqtt_password "smolder79"


#define topico_input_lum "input_lum"
#define topico_pulso "pulso_max"

WiFiClient espClient;
void callback(char* topic, byte* payload, unsigned int length);
PubSubClient client(mqtt_server, 1883, callback, espClient);

os_timer_t mTimer;

int contador, leituras, cont_pulso;
bool led_medidor, led_medidor_ant=1, pisca;
int pulso, pulso_max; //valor da medicao do led na piscada
String envio;
bool flag_pulso;  //verifica q houve pulso

//Nunca execute nada na interrupcao, apenas setar flags!
void tCallback(void *tCall){
    pulso = analogRead(A0);

    if(pulso<pulso_max-20) led_medidor = 0; //nao houve pulso do led do medidor, valor de leitura base do opto
    else led_medidor = 1;

    if(!led_medidor_ant & led_medidor) {
      cont_pulso++; //se mudou de 0 pra 1, incrementa
      flag_pulso=1;
    }
    led_medidor_ant = led_medidor;

    if(pulso>pulso_max)  pulso_max = pulso; //pega o valor maximo do pulso do led

    contador++;   //pra calcular quantas vezes essa funcao eh executada
}

//FUNCAO DE EXECUCAO COM ESTOURO DE TIMER
void usrInit(void){
    os_timer_setfn(&mTimer, tCallback, NULL);  //Chama a funcao tCallback a cada 10ms
    os_timer_arm(&mTimer, 10, true);  //10ms
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

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  Serial.print("Mensagem recebida [");
  String topico = String(topic);
  Serial.print(topico);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    msg += (char)payload[i];
  }
  Serial.println();
  if(topico=="input") {
    if(msg=="[]") cont_pulso=0; //se receber a msg "[]" zera o medidor KWh
    //if(msg.startsWith("#")) {
      //msg.remove(0);
  }
  Serial.println(msg);
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
      client.subscribe("input");   //faz uma leitura no topico para receber
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

long lastMsg = 0;
int cont = 0, valor_lum;

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
  //valor_lum = analogRead(A0);
  Serial.print("opto_min: ");
  Serial.println(pulso);
  Serial.print("pulso_max: ");
  Serial.println(pulso_max);
  pulso_max = 0; //reinicia o pulso_max para se recalibrar
  contador = 0;  //zera a contagem de interrupcoes por segundo q esta ocorrendo
  yield();

  digitalWrite(LED_AZUL, LOW);
  delay(300);
  digitalWrite(LED_AZUL, HIGH);
  delay(700);
  yield();
  if(flag_pulso) {  //so envia dados se houver pulso no led do medidor
    envio = String(cont_pulso) + "n" + String(pulso) + "n" + String(pulso_max);
    client.publish("set_lum", String(envio).c_str(), true); //publica a contagem de pulsos KWh
    Serial.println(envio);
    flag_pulso=0; //so executa de novo se houver nova piscada no led do medidor
    yield();
  }
  // long now = millis();
  // if (now - lastMsg > 10000) {   //executa a cada 10s
  //   lastMsg = now;
  //   Serial.println("pegando dados ################################ ");
  //   pulso_max=0; //reinicia o pulso_max
  // }
}
