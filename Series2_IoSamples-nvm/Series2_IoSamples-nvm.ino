/**
 * Copyright (c) 2017 Neviim Jads developer. All rights reserved.
 *
 * Firmware para Arduino Fio com modulo xBee Serial
 * 
 * Led de status porta 13
 * Led de Error porta 12
 * Led de Data porta 10
 * 
 * Manda a leitura do modulo DHT22 para a porta serial xBee
 *   - Unidade
 *   - Temperatura em Celcius e Fahrenheit
 *   - Calcular o índice de calor
 * 
 * Versão: 0.2.3
 */

#include <XBee.h>
#include <SoftwareSerial.h>
#include <DHT.h>

#define DEVICEID "DA"   // this is the LLAP device ID
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define DHTPIN 5        // Pino 5
/*
// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor
*/

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

// Initialize xBee.
XBee xbee = XBee();
XBeeResponse response = XBeeResponse();

// Criar objetos de resposta reutilizáveis para respostas que esperamos lidar 
Rx16Response rx16 = Rx16Response();
Rx64Response rx64 = Rx64Response();

int statusLed = 13;
int errorLed = 12;
int dataLed = 10;

uint8_t option = 0;
uint8_t data = 0;


// ========================= flashLed
//
void flashLed(int pin, int times, int wait) {
    
    for (int i = 0; i < times; i++) {
      digitalWrite(pin, HIGH);
      delay(wait);
      digitalWrite(pin, LOW);
      
      if (i + 1 < times) {
        delay(wait);
      }
    }
}


// ========================= espera_ms
//
void espera_ms(int milessegundos) {
  
  delay(milessegundos); // ms
  
}


// ========================== Setup
//
void setup() { 
  
  pinMode(statusLed, OUTPUT);
  pinMode(errorLed, OUTPUT);
  pinMode(dataLed, OUTPUT);

  // start serial
  Serial.begin(9600);
  xbee.setSerial(Serial);
  dht.begin();

  Serial.println("DHT22 ativado!");
  Serial.println(".");

  flashLed(statusLed, 3, 50);
}


// =========================== Loop
//
void loop() {
  
  // === attempt xbee on    
  xbee.readPacket();

  // se recebeu algum pacote?
  if (xbee.getResponse().isAvailable()) {
    // got something
    
    if (xbee.getResponse().getApiId() == RX_16_RESPONSE || xbee.getResponse().getApiId() == RX_64_RESPONSE) {
      // got a rx packet
      
      if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
              xbee.getResponse().getRx16Response(rx16);
              option = rx16.getOption();
              data = rx16.getData(0);
      } else {
              xbee.getResponse().getRx64Response(rx64);
              option = rx64.getOption();
              data = rx64.getData(0);
      }
      
      // TODO check option, rssi bytes    
      flashLed(statusLed, 1, 10);
      
      // set dataLed PWM to value of the first byte in the data
      analogWrite(dataLed, data);
      Serial.println("Set dataLed PWM: ");
      Serial.print(data);
      
    } else {
      // not something we were expecting
      flashLed(errorLed, 1, 25);    
    }
    
  } else if (xbee.getResponse().isError()) {
    //nss.print("Error reading packet.  Error code: ");  
    //nss.println(xbee.getResponse().getErrorCode());
    // or flash error led
    flashLed(errorLed, 1, 25); 
  }
  
  // retorna Pong, a pergunta recebida
  // Serial.println("Pong");  
  // === attempt xbee fim

  // termometro e umidade
  espera_ms(2000);
  lerDHT22();
  
}


// =================== lerDHT22
//
void lerDHT22() {
  
  // Leitura da temperatura ou umidade leva cerca de 250 milissegundos!
  // Leituras de sensores também podem leva até 2 segundos (caso o sensor for muito lento)
  
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Verifique se alguma leitura falhou sendo verdadeiro 
  // saia desta função (para tentar novamente).
  
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Falha ao ler do sensor DHT!");
    return;
  }

  // Calcular o índice de calor em Fahrenheit (padrão)
  float hif = dht.computeHeatIndex(f, h);
  // Calcular o índice de calor em Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Umidade: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Indice de calor: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
}




