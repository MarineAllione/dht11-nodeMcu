/* 
 * *************************
 * Capteur : DHT11
 * 
 * Les broches sont numérotées de la gauche vers la droite lorsque l'on regarde le capteur de face
 * 
 * Broche n°1 connectée au +3.3V
 * Broche n°2 (data) connectée à la broche 'D4' du NodeMcu (Pin 2 pour l'arduino) avec une résistance de 10 K reliée au +3.3v
 * Broche n°3 non connectée
 * Broche n°4 connectée à la masse (GND)
 * 
 * Installer la bibliothèque 'DHT sensor library by Adarftuit'
 * Cette librairie necessite une librairie supplémentaire qui est disponible à l'adresse : https://github.com/adafruit/Adafruit_Sensor
 * et qui doit être installée selon ce tutoriel : https://learn.adafruit.com/adafruit-all-about-arduino-libraries-install-use/installing-a-library-on-linux
 *  
 */

// Déclaration des librairies
#include <DHT.h>                      // pour communiquer avec le capteur DHT
#include <ESP8266WiFi.h>              // pour le support du wifi
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>        // pour utliser le wifi
#include <LiquidCrystal_I2C.h>        //pour utiliser la librairie du lcd

// Préparation du capteur DHT
#define DHTPIN 2                      // broche sur laquelle est raccordée la data du capteur (la broche 'D4' du NodeMcu correspond à la broche 2 de l'arduino)
#define DHTTYPE DHT11                 // précise la référence du capteur DHT (DHT11 ou DHT21 ou DHT22)
DHT dht(DHTPIN, DHTTYPE);             // Initialisation du capteur DHT

LiquidCrystal_I2C lcd(0x27,20,4);

// Initialisation du wifi
ESP8266WiFiMulti WiFiMulti;           

// fonction de démarrage
void setup() {
  
  // Démarrage du bus série
  Serial.begin(115200);               // vitesse
  Serial.println("Bonjour");          // écriture d'un petit message...
  Serial.println("DHT11 + VPS");
  Serial.println("");

//Allumer le lecteur lcd
  lcd.init();
  lcd.backlight();
  dht.begin();                        // Démarrage du capteur DHT11

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("David", "daviddavid");     // Connexion au réseau wifi

  
}

// boucle infinie
void loop() {
  
  delay(10000);                        // attendre 10 secondes

  float t = dht.readTemperature();    // mesurer la température (en ° Celsius)
  float h = dht.readHumidity();       // mesurer l'humidité (en %)
  
  if (isnan(h) || isnan(t)) {         // test si des valeurs ont été récupérées
    
    // s'il y a un problème...
    
    Serial.println("Failed to read from DHT sensor!");    // affiche un message d'erreur
    return;                           // quitte pour retenter une lecture
    
  } else {

    // si tout va bien...

    // conversion des valeurs en entier
    int temp = int(t);
    int humi = int(h);
  
    // affichage des valeurs dans le bus série
    Serial.print("Temperature : ");
    Serial.print(temp);
    Serial.print(" *C\t");

    //Lecteur lcd
    lcd.setCursor(0,0);
    lcd.print("Temp : ");
    lcd.setCursor(5,0);
    lcd.print(t);
    
    Serial.print("Humidité : ");
    Serial.print(humi);
    Serial.println(" %");

 //Lecteur lcd
    lcd.setCursor(0,1);
    lcd.print("Humidity : ");
    lcd.setCursor(9,1);
    lcd.print(h);
    

    // envoi des données vers internet
    if((WiFiMulti.run() == WL_CONNECTED)) {   // Si le wifi est connecté
      
          Serial.println("Connexion wifi ok");
          
          HTTPClient http;                    // préparation d'une requète HTTP au serveur

          Serial.print("HTTP begin... ");
          http.begin("http://51.75.126.50/dht11/store_temp.php");   // connexion au serveur
          
          http.addHeader("Content-Type", "application/json");    // envoi d'un entête pour préciser notre format de données : json
          
          String data = "{\"temperature\":\"" + String(temp) + "\",\"humidite\":\"" + String(humi) + "\"}";    // préparation du json {"temperature":"22","humidite":"55"}
          Serial.println(data);
          
          int httpCode = http.POST(data);    // envoi les données et récupère un code de retour
          
          if(httpCode == 200 ) {              // si le code de retour est bon (200)
             Serial.println("POST : ok");
          } else {                            // si le code de retour n'est pas bon (différent de 200)
             Serial.print("POST : failed, error : ");
             Serial.println(http.errorToString(httpCode).c_str());
          }
          http.end();                    // on ferme la connexion au serveur
    } else  {
        Serial.println("Echec connexion wifi");
    }
  }
}
