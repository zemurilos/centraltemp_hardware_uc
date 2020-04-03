#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <AES.h>//Biblioteca do AES.

#ifndef STASSID
#define STASSID "GTMF-IFSP"
#define STAPSK  "residencia2019"
#endif

const char *ssid1 = "ESPap";
const char *password1 = "thereisnospoon";
int temp = 37;
int estado = 0;
String log_chuv;

String jsonStrTemp;
bool state = 0;
byte counter = 0;

const char* ssid = STASSID;
const char* password = STAPSK;
//Funções Server

void handleRoot();
void handleInfo();
void handlePisca();
void handleTroca();
void handleTemp();
void handleShower();
void addCORS();

String receberUart();
bool uartTest(String valorESP, String valorSTM);

AES aes;//Cria a classe aes.
byte key[16], out[16], out2[16], inp[32];//Cria arrays (vetores) para a chave, input e output de dados.
const char pass[] = "centraltemp";//Define a chave usada, neste exemplo usamos AES128, então precisa ser <= 16 Bytes.

//ESP8266WebServer server(80);
ESP8266WebServer server(1020);




void setup() {
  Serial.begin(115200, SERIAL_8N1);
  Serial1.begin(115200, SERIAL_8N1);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //Fim OTA

  pinMode(0, OUTPUT);
  pinMode(16, OUTPUT);
  WiFi.softAP(ssid1, password1, 1, false, 4);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);


  String rota = "/banho";

  server.on(rota + "/info", HTTP_OPTIONS, []() {
    addCORS();
  });

  server.on(rota + "/pisca", HTTP_OPTIONS, []() {
    addCORS();
  });

  server.on(rota + "/troca", HTTP_OPTIONS, []() {
    addCORS();
  });

  server.on(rota + "/shower", HTTP_OPTIONS, []() {
    addCORS();
  });

  server.on(rota + "/temp", HTTP_OPTIONS, []() {
    addCORS();
  });

  server.on(rota + "/temp", HTTP_POST, handleTemp);


  server.on(rota + "/shower", handleShower);

  server.on(rota, handleRoot);
  server.on(rota + "/info", handleInfo);
  //server.on(rota + "/temp", handleTemp);
  server.on(rota + "/pisca", handlePisca);
  server.on(rota + "/shower", handleShower);

  server.on("/troca", handleTroca);
  enc128("Cubex", 1);//Faz a função de encriptação e retorna o HEX encriptado.

  server.begin();
}

void loop() {

  ArduinoOTA.handle();
  server.handleClient();

  /* String a ;
    char able = 1;
    float temperatura = 0.0;

    if (estado) {
     Serial1.print('I');
     DynamicJsonDocument doc(1024);

     if (Serial.available()) {

       a = Serial.readString(); // read the incoming data as string

       //Serial.println(a);

     }

     if (a != NULL) {

       //delay(1);

       if (a.substring(0, 3) == "ST#") {
         a = a.substring(3, 4);
         Serial.println(a);

         if (a == "0") {
           if (counter == 2) {
             counter = 0;

           } else {
             counter = 1;
           }
           state = false;

         } else if (a == "1") {

           counter = 2;
           state = true;
         } else if (a == "2") {
           state = false;
           estado = 0;
         }

         // if (counter == 0)
         //   estado = 0;

       }



       a = "";

     }

     jsonStrTemp = "";
     auxiliar = 1;

    }
  */
}

void handleRoot() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", "Serviço de Banho OK");
  estado =  0;

  /*bool error;
    Serial1.print("\t\t\t");

    do {

    Serial1.print("T24");
    String recebida = receberUart();
    error = uartTest("T24", recebida);
    Serial.print(recebida);
    delay(10);

    } while (error);

    Serial.print("Funcionou");*/

}

void handleInfo() {

  String jsonStr;

  DynamicJsonDocument doc(1024);
  doc["temperatura"] = temp;
  doc["estado"] = estado;
  serializeJson(doc, jsonStr);
  serializeJson(doc, Serial);

  Serial1.print("ON\n");

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/json", jsonStr);

  Serial.print("\n\n\n");
}

void handlePisca() { //Handler

  String strTimes = server.arg("times");
  String strUser = server.arg("user");

  if (strUser != NULL) {

    unsigned int times = strTimes.toInt();   //Recebe a quantidade de vezes para piscar

    Serial1.print("#");
    Serial1.println(times);


    Serial.print("#");
    Serial.print(times);



    for (int i = 0; i < times; i++) {

      digitalWrite(16, HIGH);
      delay(1000);
      digitalWrite(16, LOW);
      delay(1000);
    }

    temp =  times;

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/html", "LED piscou " + String(times) + " vezes.");

  } else {

    server.send(404, "text/html", "Erro User.");

  }

  Serial.print("\n\n\n");
  Serial1.print("\n\n\n");
}

void handleTroca() { //Handler

  String strTimes = server.arg("state");
  String strUser = server.arg("user");
  int state = strTimes.toInt();//Get number of parameters

  if (strUser != NULL) {

    digitalWrite(16, state);

    String statusEstado = (state == 0 ? "desligado" : "ligado");

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/html", "LED foi " + statusEstado);

    estado = state;
    Serial1.printf("E%d", state);
    Serial1.println("");

    Serial.printf("E%d", state);
    server.sendHeader("Access-Control-Allow-Origin", "*");

    server.sendHeader("Access-Control-Allow-Origin", "*");

    server.send(200, "text/html", "LED foi " + statusEstado + ".");

  } else {
    server.sendHeader("Access-Control-Allow-Origin", "*");

    server.send(404, "text/html", "Erro User.");

  }
  Serial.print("\n\n\n");
  Serial1.print("\n\n\n");
}

void handleTemp() {                         // If a POST request is made to URI /login

  Serial.println(server.arg("temperatura"));
  DynamicJsonDocument doc(1024);

  auto error = deserializeJson(doc, server.arg("temperatura"));
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    server.send(401, "text/plain", "401: Erro meu ermão");
    return;
  }
  DynamicJsonDocument json(1024);
  String Json;
  
  json["temperatura"] = server.arg("temperatura");
  log_chuv = "Temperatura:" + String(server.arg("temperatura"));
  serializeJson(json, Serial);
  String jsonStr;
  serializeJson(json, jsonStr);
  //serializeJson(doc, Serial);
  server.send(200, "text/json", jsonStr);



  /*  if ( ! server.hasArg("temperatura") || server.arg("temperatura") == NULL ) { // If the POST request doesn't have username and password data
      server.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
      return;
    }

    if (server.arg("temperatura") != "0") { // If both the username and the password are correct
      server.send(200, "text/plain", "Valor = " + server.arg("temperatura"));
    } else {                                                                              // Username and password don't match
      server.send(401, "text/plain", "401: Erro meu ermão");
    }
  */
}

/*
  void handleTemp() {

  char auxiliar = 1;
  String strTempRx = server.arg("t");

  if (auxiliar) {

    Serial1.print('T');
    Serial1.print(strTempRx);

    Serial.print('T');
    Serial.print(strTempRx);
  }
  auxiliar = 0;



  String a = "", waterSerial, tempSerial, timSerial, temp1, temp2;
  char able = 1;
  float temperatura = 0.0;

  DynamicJsonDocument doc(1024);


  while (able) {

    while (Serial.available()) {

      a = Serial.readString(); // read the incoming data as string

      //Serial.println(a);

    }

    if (a != NULL) {

      //delay(1);

      if (a.substring(0, 4) == "WAT#") {
        waterSerial = a.substring(4, 9);
      } else if (a.substring(0, 4) == "TEMP") {
        tempSerial = a.substring(4, 9);
      } else if (a.substring(0, 4) == "TIM#") {
        timSerial = a.substring(4, 12);

        temp1 = tempSerial.substring(0, 2);
        temp = temp1.toInt();
        temp2 = tempSerial.substring(2, 4);

        temperatura = temp1.toInt();
        temperatura += (temp2.toInt() / 100);

        //tempSerial = tempSerial.substring(0, 2) + "," + tempSerial.substring(2, 4);
        doc["agua"] = waterSerial;
        doc["temperatura"] = temperatura;
        doc["tempo"] = timSerial;
        estado = 1;
        able = 0;
        break;
      }

      a = "";
    }
  }

  serializeJson(doc, jsonStrTemp);
  serializeJson(doc, Serial);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/json", jsonStrTemp);
  jsonStrTemp = "";

  Serial1.print("\n\n\n");
  }
*/

void handleShower() {

  String jsonStr;

  DynamicJsonDocument doc(1024);
  doc["modo"] = estado;
  doc["banho"] = state;
  serializeJson(doc, jsonStr);
  serializeJson(doc, Serial);

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/json", jsonStr);
  Serial.print("\n\n\n");
  Serial1.print("\n\n\n");
}

void addCORS() {
  server.sendHeader("Access-Control-Allow-Credentials", "false"); //mudar para 'true' para testes.
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,PUT,DELETE,OPTIONS");
  //server.send(204);
}

bool uartTest(String valorESP, String valorSTM) {
  return (valorESP != valorSTM);
}


String receberUart() {

  String STM;
  byte uartTesting = 1;
  while (uartTesting) {

    if (Serial.available()) {
      STM = Serial.readString(); // read the incoming data as string
    }
    if (STM != NULL) {
      Serial.println(STM);
      uartTesting = 0;
    }
  }
  log_chuv += ",\n" + "Tempo:" + "add var para tempo";
  return STM;
}

void enc128(const char txt[], bool db)//Argumentos: (texto e debug)
{
  if (strlen(pass) > 16)//Verifica se a chave tem o tamanho limite de 16 caracteres.
  {
    if (db == true)
    {
      Serial.println("Chave para AES128 <= 16 Bytes");
    }
    return;//Se a chave for maior, irá sair da função.
  }

  if (strlen(txt) > 16)//Verifica se o texto tem o tamanho limite de 16 caracteres.
  {
    if (db == true)
    {
      Serial.println("Frase/numero para AES <= 16 Bytes / bloco");
    }
    return;//Se o texto for maior, irá sair da função.
  }

  for (byte i = 0; i < strlen(pass); i++)//Adiciona a chave(pass) na array key.
  {
    key[i] = pass[i];
  }

  for (byte i = 0; i < strlen(txt); i++)//Adiciona o texto na array input.
  {
    inp[i] = txt[i];
  }

  //Adiciona a chave ao algoritimo.
  if (aes.set_key(key, 16) != 0)//Verifica se a chave esta correta, caso nao, sairá da função.
  {
    if (db == true)
    {
      Serial.println("Erro ao configurar chave");
    }
    return;//Sai da função
  }

  //Faz a encriptação da array INPUT e retorna o HEXA na array OUTPUT.
  if (aes.encrypt(inp, out) != 0)//Verifica se a encriptação esta correta, se não, sairá da função.
  {
    if (db == true)
    {
      Serial.println("Erro ao encriptar");
    }
    return;//Sai da função
  }

  if (db == true)//Se o debug estiver on (1), irá mostrar o HEXA no serial monitor.
  {
    for (byte i = 0; i < 16; i++)
    {
      Serial.print(out[i], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
    for (byte i = 0; i < 16; i++)
    {
      Serial.print((char)out[i]);
      Serial.print(" ");
    }
    Serial.println(" ");
    aes.decrypt(out, out2);
    for (byte i = 0; i < 16; i++)
    {
      Serial.print((char)out2[i]);
      Serial.print(" ");
    }

    Serial.println();
  }

  aes.clean();//Limpa a chave e residuos sensiveis da encriptação.
}
