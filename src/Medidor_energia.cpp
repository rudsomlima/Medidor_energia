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

#define topico_medidor "medidor"
#define topico_set_lum "set_lum"
#define topico_input_lum "input_lum"
#define topico_pulso_alto "pulso_alto_max"

WiFiClient espClient;
void callback(char* topic, byte* payload, unsigned int length);
//PubSubClient client(espClient);
PubSubClient client(mqtt_server, 1883, callback, espClient);


//#define opt_on 50

os_timer_t mTimer;

int contador, leituras, cont_pulso;
bool led_pulso, led_pulso_ant=0, pisca;
int opt_off = 10;  //inserir a leitura do opto sensor quando o led desligado
int pulso_alto, pulso_alto_max; //valor da medicao do led na piscada
String envio;

//Nunca execute nada na interrupcao, apenas setar flags!
void tCallback(void *tCall){
    pulso_alto = analogRead(A0);
    if(pulso_alto>opt_off) led_pulso = 1;  //testa o estado do led atraves do leitor optico
    else led_pulso = 0;

    if(pulso_alto>pulso_alto_max)  pulso_alto_max = pulso_alto; //pega o valor maximo do pulso do led

    if(!led_pulso_ant & led_pulso) cont_pulso++; //se mudou de 0 pra 1, incrementa
    led_pulso_ant = led_pulso;

    contador++;
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
  if(topico==topico_input_lum) {
    if(msg=="[]") cont_pulso=0; //se receber a msg "[]" zera o medidor KWh
    else {
      opt_off = msg.toInt();
      Serial.print("opt_off setado: ");
      Serial.println(opt_off);
    }
  }
  // Serial.println(msg);
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
  Serial.print("valor_lum: ");
  Serial.println(pulso_alto);
  Serial.print("pulso_alto_max: ");
  Serial.println(pulso_alto_max);

  //Serial.println(leituras);
  //leituras++;
  contador=0;  //zera a contagem de interrupcoes por segundo q esta ocorrendo
  yield();

  digitalWrite(LED_AZUL, LOW);
  delay(10);
  digitalWrite(LED_AZUL, HIGH);
  delay(990);
  yield();
  envio = String(cont_pulso) + "n" + String(pulso_alto) + "n" + String(pulso_alto_max);
  // client.publish(topico_medidor, String(cont_pulso).c_str(), true); //publica a contagem de pulsos KWh
  // yield();
  client.publish(topico_set_lum, String(envio).c_str(), true); //publica a contagem de pulsos KWh
  Serial.println(envio);
  yield();
  // client.publish(topico_input_lum, String(pulso_alto).c_str(), true); //publica a contagem de pulsos KWh
  // yield();
  // client.publish(topico_pulso_alto, String(pulso_alto_max).c_str(), true); //publica a contagem de pulsos KWh
  // yield();



  long now = millis();

  if (now - lastMsg > 10000) {   //executa a cada 10s
    lastMsg = now;
    client.subscribe("input_lum");   //faz uma leitura no topico lum para receber msgclient.publish(topico_lum, String(valor_lum).c_str(), true); //publica a luminosidade do sensor
    yield();

    pulso_alto_max=0; //reinicia o pulso_alto_max

  }
}
