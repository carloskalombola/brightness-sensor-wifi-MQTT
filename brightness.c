/// Photoresistor Project (brightness) +wifi+MQTT

//### Import libs
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>


///### configuration Wi-Fi - wifi configuration
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
// Setting the WiFi channel speeds up the connection:
// Définir le canal WiFi accélère la connexion:
#define WIFI_CHANNEL 6
int status = WL_IDLE_STATUS;   // l'état de la radio Wifi - the Wifi radio's status


//////###Configuration LDR
#define LDR_PIN 34 //définit la broche de lecture de la photorésistance
// Caractéristiques du LDR - LDR Characteristics
// Ces constantes doivent correspondre aux attributs "gamma" 
// et "rl10" de la photorésistance
const float GAMMA = 0.7;
const float RL10 = 33;
int valorAnterior = 0; // Variable pour stocker la valeur précédente - Variable to store the previous value
float lux=0;


///########## Configuration du MQTT
const char* mqtt_server = "test.mosquitto.org";// MQTT broker
char* my_topic_SUB = "MCK/SUB";// le sujet choisi - the chosen topic
char* my_topic_PUB = "MCK/PUB";// le sujet choisi


/////##### configuration wifi et MQTT
WiFiClient espClient;
PubSubClient client(espClient);


/////### Réglage du temps de lecture
int contador = 1;
char mensagem[30];
unsigned long lastMillis = 0;


void setup() {
  // Code de configuration, à exécuter une seule fois - put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
conectawifi();


  client.setServer(mqtt_server, 1883); // Définir le serveur et le port du courtier MQTT - Set the MQTT broker server and port
  client.setCallback(callback); // Définir la fonction de rappel pour les messages MQTT entrants - Set the callback function for incoming MQTT messages


}
void loop() {
  // mettez votre code principal ici, pour l'exécuter à plusieurs reprises:
  // put your main code here, to run repeatedly:


  float luz =  fotoresistor();


  // Vérifier la connexion MQTT et traiter les messages entrants
  // Check MQTT connection and process incoming messages
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //Ajoutez un délai pour éviter d'inonder le courtier MQTT de messages
  // Add a delay to avoid flooding the MQTT broker with messages
 delay(100); 


  //Attendez 15 secondes pour envoyer un nouveau message
  //Wait 15 seconds to send a new message
   if (millis() - lastMillis > 15000) {
    lastMillis = millis();
    sprintf(mensagem, "%d", luz);
    //sprintf(mensagem, "MQTT ESP32 - Mensagem no. %d", contador);  
  Serial.print("Mensagem enviada: ");
  Serial.println(mensagem);
 //Envoyer le message au courtier - Send the message to the broker
  client.publish(my_topic_PUB, mensagem);
  Serial.print(contador);
  Serial.println("-Mensagem enviada com sucesso...");
 
  //Incrémenter le compteur - Increment the counter
  contador++;
 
    }
   
  delay(10); //cela accélère la simulationthis-speeds up the simulation
}


void conectawifi()
  {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Conectando ao WiFi ");
  Serial.print(WIFI_SSID);
  // Attendre la connexion-Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
       //WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  }
  Serial.println(" Conectado!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  }


float  fotoresistor()
{
  int analogValue = analogRead(LDR_PIN);
  //Serial.println(analogValue);
  ////ne lit que s'il y a un changement dans la valeur
  //only reads if there is a change in the value
   if (analogValue != valorAnterior) {
   
  float voltage = analogValue / 4096. * 3.3;
  float resistance = 2000 * voltage / (1 - voltage / 3.3);
   lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));


  Serial.print("Ambiente: ");


  ///Valeur d'éclairage de bureau 400-Office lighting value 400
  if (lux > 400) {
    Serial.println("Claro!");
  } else {
    Serial.println("Escuro  ");
  }


    Serial.print("Luminosidade: ");
  Serial.println(lux);




  // Mettre à jour la valeur précédente avec la nouvelle valeur
  //Update the previous value with the new value
    valorAnterior = analogValue;
  }
  return lux;
}




  //Traiter le message MQTT entrant et contrôler le servomoteur:
  // Process incoming MQTT message and control the servo motor
void callback(char* topic, byte* payload, unsigned int length) {
  String string;
  Serial.print("Chegou a mensagem [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
     string+=((char)payload[i]);  
  }
  Serial.print(string);
 
}
//Essayez de vous reconnecter au courtier MQTT si la connexion est perdue:
// Attempt to reconnect to the MQTT broker if the connection is lost
void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT ...");
    if (client.connect("ESPClient")) {
      Serial.println("Conectado");
      client.subscribe(my_topic_SUB);
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" Tente novamente em 5 segundos");
     
     
  }
  }
}
