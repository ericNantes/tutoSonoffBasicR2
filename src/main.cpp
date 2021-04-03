#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "InfoConnexion.h"
#include "ESP8266WebServer.h"
#include "ArduinoOTA.h"

// Définition des entrées/sorties
#define PIN_LED 13
#define PIN_RELAI 12
#define PIN_BOUTON 0

// Adresse IP partielle du module
#define IP_MODULE 201

// Page html
const char index_html[] PROGMEM = R"=====(
<!doctype html>
<html lang="fr">
    <head>
        <meta charset="utf-8">
        <title>Commande module SONOFF BASIC R2</title>
        <style>
            h1, h2, h3, p { font: arial;}
            .bouton {
                font: Arial;
                background-color: blueviolet;
                border: none;
                color: white;
                padding: 15px 32px;
                text-align: center;
                text-decoration: none;
                display: inline-block;
                font-size: 16px;
                margin: 4px 2px;
                cursor: pointer;                
            }
        </style>
    </head>
    <body>
        <h1 id="etatSonoffBasic">Etat du module : %ETAT_SONOFF%</h1>
        
        <h1>Commande du module</h1>
        <button class="bouton" onclick="appelServeur('/switchOn', traiteReponse)">Allumer</button>
        <button class="bouton" onclick="appelServeur('/switchOff', traiteReponse)">Eteindre</button>

        <script>
            function appelServeur(url, cFonction) {
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function() {
                    if (this.readyState == 4 && this.status == 200) {
                        cFonction(this);
                    }
                };
                xhttp.open("GET", url, true);
                xhttp.send();
            }
            function traiteReponse(xhttp) {
                document.getElementById("etatSonoffBasic").innerHTML = "Etat du module : " + xhttp.responseText;
            }
        </script>
    </body>
</html>
)=====";

// Informations de connexion : cachées dans fichier InfoConnexion.h
// Vous pouvez décommenter ici ou créer comme moi un fichier InfoConnexion.h
//const char * SSID = "Votre SSID";
//const char * PASSWORD = "Votre mot de passe";

// Gestion des événements du WiFi
// Lorsque la connexion vient d'aboutir
void onConnected(const WiFiEventStationModeConnected& event);
// Lorsque l'adresse IP est attribuée
void onGotIP(const WiFiEventStationModeGotIP& event);

// Objet WebServer
ESP8266WebServer serverWeb(80);

// Fonctions du serveur Web
void handleRoot() {
  String temp(reinterpret_cast<const __FlashStringHelper *> (index_html));
  if (digitalRead(PIN_RELAI) == HIGH) temp.replace("%ETAT_SONOFF%", "ON"); else temp.replace("%ETAT_SONOFF%", "OFF"); 
  serverWeb.send(200, "text/html", temp);
}

void switchOn() {
  digitalWrite(PIN_RELAI, HIGH);
  serverWeb.send(200, "text/plain", "ON");
}

void switchOff() {
  digitalWrite(PIN_RELAI, LOW);
  serverWeb.send(200, "text/plain", "OFF");
}

void APOff() {
  WiFi.enableAP(false);
  serverWeb.send(200, "text/plain", "Access Point Off.");
}

void setup() {
  // Liste des IP pour une configuration en IP Fixe
  IPAddress ip(192, 168, 0, IP_MODULE);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(192, 168, 0, 1);

  // Mise en place d'une liaison série
  Serial.begin(9600L);
  delay(100);

  // Configuration des entrées/sorties
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_RELAI, OUTPUT);
  pinMode(PIN_BOUTON, INPUT_PULLUP);

  // Module OFF par défaut
  digitalWrite(PIN_RELAI, LOW);

  // Mode de connexion
  WiFi.softAP("Module Sonoff Basic R2");
  WiFi.mode(WIFI_AP_STA);
 
  // Démarrage de la connexion
  WiFi.config(ip, gateway, subnet, dns);
  WiFi.begin(SSID, PASSWORD);

  static WiFiEventHandler onConnectedHandler = WiFi.onStationModeConnected(onConnected);
  static WiFiEventHandler onGotIPHandler = WiFi.onStationModeGotIP(onGotIP);

  // Mise en place du serveur WebServer
  serverWeb.on("/switchOn", switchOn);
  serverWeb.on("/switchOff", switchOff);
  serverWeb.on("/APOff", APOff);
  serverWeb.on("/", handleRoot);
  serverWeb.on("/index.html", handleRoot);
  serverWeb.begin();
}

void loop() {
  // Si l'objet est connecté au réseau, on effectue les tâches qui doivent l'être dans ce cas
  if (WiFi.isConnected()) {
    digitalWrite(PIN_LED, LOW); // LED active à l'état bas
    ArduinoOTA.begin();
  }
  else {
    digitalWrite(PIN_LED, HIGH);
  }
  ArduinoOTA.handle();
  serverWeb.handleClient();
}

void onConnected(const WiFiEventStationModeConnected& event) {
  Serial.println("WiFi connecté");
  Serial.println("Adresse IP : " + WiFi.localIP().toString());
}

void onGotIP(const WiFiEventStationModeGotIP& event) {
  Serial.println("Adresse IP : " + WiFi.localIP().toString());
  Serial.println("Passerelle IP : " + WiFi.gatewayIP().toString());
  Serial.println("DNS IP : " + WiFi.dnsIP().toString());
  Serial.print("Puissance de réception : ");
  Serial.println(WiFi.RSSI());
}
