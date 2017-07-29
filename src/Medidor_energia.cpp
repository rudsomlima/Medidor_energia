#include <Ticker.h>
#include <Arduino.h>

extern "C"{
#include "user_interface.h"
}

os_timer_t mTimer;

int contador, leituras, cont_pulso;
bool led_pulso, led_pulso_ant=0, pisca;

//Nunca execute nada na interrupcao, apenas setar flags!
void tCallback(void *tCall){
    if(analogRead(A0)>10) led_pulso = 1;  //testa o estado do led atraves do leitor otico
    else led_pulso = 0;

    if(!led_pulso_ant & led_pulso) cont_pulso++; //se mudou de 0 pra 1, incrementa
    led_pulso_ant = led_pulso;

    contador++;
}

void usrInit(void){
    os_timer_setfn(&mTimer, tCallback, NULL);
    os_timer_arm(&mTimer, 10, true);
}

#define LED_AZUL 2 // led azul na placa lolin nodemcu v3

int opt_101; //Tensao do leitor de luminosidade opt101

Ticker ticker;
void tick()
{
  //toggle state
  // int state = digitalRead(LED_AZUL);  // get the current state of GPIO1 pin
  // digitalWrite(LED_AZUL, !state);     // set pin to the opposite state
  pisca++;
}

void setup()
{
    Serial.begin(9600);
    pinMode(LED_AZUL, OUTPUT);
    ticker.attach(0.1, tick);
    usrInit();
}

void loop()
{
  Serial.print("pisca: ");
  Serial.println(pisca/2);
  Serial.print("pulsos: ");
  Serial.println(cont_pulso);
  Serial.print("leituras por segundo: ");
  Serial.println(contador);
  //Serial.println(leituras);
  //leituras++;
  contador=0;
  yield();

  digitalWrite(LED_AZUL, LOW);
  delay(10);
  digitalWrite(LED_AZUL, HIGH);
  delay(990);
}
