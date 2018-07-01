/*
 *  Inspired by Ivan Grokhotkov, 2015.
 *  This example is in public domain.
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.3


/***********MODIFY THIS**********/
const char* ssid = "Nokia 3";
const char* password = "fd6bbc8d51bd";
#define DEVICE_TYPE "NodeMCU"
#define DEVICE_ID "NodeMCU-1"
/********************************/
#define ORG "d425ow"
#define TOKEN "dQNbMQIw+wBc9fQgey"
#define ANGLE 19 //angulo de giro
#define ANGLE2 22 //angulo de giro
//-------- Customise the above values --------

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char topic[] = "iot-2/cmd/test/fmt/String";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

WiFiClient wifiClient;
void callback(char* topic, byte* payload, unsigned int payloadLength);
PubSubClient client(server, 1883, callback, wifiClient);


enum PERIODE_JOURNEE{MATIN, MIDI, SOIR}; //Etapa del dia: Dia Tarde Noche
enum ETAT{ATT_PRISE, PRISE_MEDOC, MEDOC_PRIS, MEDOC_NON_PRIS};  //estado
bool medocPris = false; //medicamento tomado
int nbRappels = 0; //numero de recordatorios
long lastMsg = 0;
long lastMsg2 = 0;
bool PassageUnefois = false;

const int buttonPin = D1;     // the number of the pushbutton pin
const int ledPin =  13;      // the number of the LED pin

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

PERIODE_JOURNEE JourCourant = MATIN;3
ETAT EtatCourant = ATT_PRISE;

unsigned long previousMillis = 0;
const long intervalLed = 1000;
int ledState = LOW;  
Servo myservo1;
Servo myservo2;
int position1 = 0;
int position2 = 0;
int roue1[3]; //rueda 1
int roue2[3]; // rueda 2
char msg[20]; //mensaje
void setup() {
  Serial.begin(9600);
  setup_wifi();
  
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(buttonPin, INPUT);
    pinMode(D4, OUTPUT);              // LED VERTE

  client.setServer(mqtt_server, MQTTPort);
  client.setCallback(callback);
  mqttConnect();

  myservo1.attach(D2); 
  myservo2.attach(D3);
  myservo1.write(position1); 
  myservo2.write(position2);  

}

void setup_wifi(){ //muestra información en el monitor de serie sobre la conexión wifi

  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) { //funcion enviada por el servidor
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    msg[i] = (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.println(msg);
  if(!strcmp(topic, prendreMedocTopic)){
    if((length == 8) || (length == 7)){
      EtatCourant = PRISE_MEDOC;
    }
    else if(length == 12){
      sscanf( msg, "%d;%d;%d;%d;%d;%d;", 
          &roue1[0], &roue1[1], &roue1[2], &roue2[0], &roue2[1], &roue2[2]);
      for(int i = 0; i < 3; i++){
        Serial.println(roue1[i]);
        Serial.println(roue2[i]);
      }
    }
  }
  /*// Parse JSON object
  DynamicJsonBuffer jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject((char*)payload);
  if (!root.success()) {
    Serial.println("Parsing failed!");
    return;
  }
  Serial.print("reponse: ");
  */
  

}

void initManagedDevice() {
   if (client.subscribe(prendreMedocTopic)) {
     Serial.println("subscribe to PrendreMedocTopic OK");
   } else {
     Serial.println("subscribe to PrendreMedocTopic FAILED");
   }
 }

void mqttConnect() {
  if (!client.connected()) {
    Serial.print("Reconnecting MQTT client to "); Serial.println(mqtt_server);
    while (!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }
    initManagedDevice();
    Serial.println();
  }
}
  

void prendreMedoc(){
  unsigned long delaiMili = DELAISEC*1000;

  long now = millis();

  String payload= "{}";
  if ((now - lastMsg2) > (delaiMili/2)) {
    lastMsg2 = now;
    if (client.publish(askRev, (char*) payload.c_str())) {
      Serial.println("Publish ok");
    } else {
      Serial.println("Publish failed");
    }
  }

}

void tournerRoues(){ //girar las ruedas
  switch(JourCourant){
    case MATIN:
      for(int i = 0; i < roue1[0]*2; i++){
        myservo1.write(position1 + ANGLE);
        position1 = position1 + ANGLE+i;
        delay(1500);
      }
      for(int i = 0; i < roue2[0]*2; i++){
        myservo2.write(position2 + ANGLE2);
        position2 = position2 + ANGLE2+i;
        delay(1500);
      }
    break;

    case MIDI:
      for(int i = 0; i < roue1[1]*2; i++){
        myservo1.write(position1 + ANGLE);
        position1 = position1 + ANGLE+i;
        delay(1500);
      }
      for(int i = 0; i < roue2[1]*2; i++){
        myservo2.write(position2 + ANGLE2);
        position2 = position2 + ANGLE2+i;
        delay(1500);
      }
    break;

    case SOIR:
      for(int i = 0; i < roue1[2]*2; i++){
        myservo1.write(position1 + ANGLE);
        position1 = position1 + ANGLE+i;
        delay(1500);
      }
      for(int i = 0; i < roue2[2]*2; i++){
        myservo2.write(position2 + ANGLE2);
        position2 = position2 + ANGLE2+i;
        delay(1500);
      }
    break;
  }
}

void checkPrise(){ 
  //Serial.println("Dans Check Prise");
  unsigned long delaiMili = DELAISEC*1000;

  long now = millis();
  if(!medocPris){
    if ((now - lastMsg) > delaiMili) {
      // comenzará un temporizador de 30 minutos para un nuevo recordatorio
      lastMsg = now;
      String payload= "nonPris";
      if (client.publish(medocNonPris, (char*) payload.c_str())) {
        Serial.println("Publish non pris ok");
      } else {
        Serial.println("Publish failed");
      }
      EtatCourant = ATT_PRISE;   //dispensando
    }  
  }
  else{
    EtatCourant = MEDOC_PRIS; //dispensando
  }
  
}
void checkPriseApresRappel(){
  if(medocPris){
    EtatCourant = MEDOC_PRIS;
    String payload= "stop";
      if (client.publish(stopRappel, (char*) payload.c_str())) {
        Serial.println("Publish ok");
      } else {
        Serial.println("Publish failed");
      }  
  }

}
void actionsDeRappel(){
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= intervalLed) {
    previousMillis = currentMillis;   
   if (ledState == LOW){
            digitalWrite(D4, HIGH);
      ledState = HIGH;  // Note that this switches the LED *off*

    }
    else{
            digitalWrite(D4, LOW);
      ledState = LOW;   // Note that this switches the LED *on*
    }
    digitalWrite(LED_BUILTIN, ledState);
  }
}

void contacterFamille(){ //contactar familiar, aun no funciona falla 
      String payload= "Contact";
      if (client.publish(MsgFamille, (char*) payload.c_str())) {
        Serial.println("Publish ok");
      } else {
        Serial.println("Publish failed");
      }

      EtatCourant = ATT_PRISE;
}

void machineEtat(){
  switch(EtatCourant)
  {
    case ATT_PRISE:
      if(nbRappels){
        checkPriseApresRappel();
      }
    break;

    case PRISE_MEDOC:
        if(!nbRappels)
          tournerRoues();
        lastMsg = millis();
        nbRappels++;
        EtatCourant = MEDOC_NON_PRIS; //medicina no tomada
        Serial.print("Primer pase en Prise_medoc, nbRappels = ");
        Serial.println(nbRappels);
      
    break;

    case MEDOC_PRIS:
      EtatCourant = ATT_PRISE;
      nbRappels = 0;
      medocPris = false;
      PassageUnefois = false;
      // Si se toma medicamento, pasa al siguiente período
      switch (JourCourant){
        case MATIN:
          JourCourant = MIDI;
        break;
        case MIDI:
          JourCourant = SOIR;
        break;
        case SOIR:
          JourCourant = MATIN;
        break;
        
    }
    break;

    case MEDOC_NON_PRIS: //Medicina no tomada 
        if(nbRappels > 3){
          contacterFamille(); //contacta a los familiares
          EtatCourant = MEDOC_PRIS;
        }
        actionsDeRappel();// leds parpadeantes, buzser?
        checkPrise();
            
    break;
    
  }
  
}
void verifConteneur(){
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == LOW) {
      medocPris = true;
      Serial.println("Medoc Pris");
  } else {
    // turn LED off:
    medocPris = false;
  }
}


void loop() {
  machineEtat();
  if(nbRappels)
    verifConteneur();

  if (!client.loop()) {
    mqttConnect();
  }
  
}
