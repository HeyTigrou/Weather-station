// INCLUDES BIBLIOTHEQUES :
#include <SoftwareSerial.h>
#include <Wire.h>
#include "DS1307.h"
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ChainableLED.h>


// DEFINE :
#define SEALEVELPRESSURE_HPA (1013.25)
#define NUM_LEDS 1
#define CONFIGURATION 0
#define STANDARD 1
#define ECONOMIQUE 2
#define MAINTENANCE 3

#define MEMORY 0
#define SERIE 1

// COMMANDES INITIALISATION DES COMPOSANTS :
Adafruit_BME280 bme;

int boutonvert = 2;
int boutonrouge = 3;
ChainableLED leds(8, 9, NUM_LEDS);
const int chipSelect = 4; // Model du lecteur de carte SD
const int analogPin = 2; // Capteur de luminosité sur le port A2
SoftwareSerial SoftSerial(11, 10); // Emulation d'un moniteur série sur les Pins 11,10 pour le GPS
DS1307 clock; // Definir un objet de la classe RTC Clock sur le port I2C du grove shield
String gpsData;
bool t;

// DEMARRAGE CLOCK INTERNE PROGRAMME :
unsigned long codeclock;

// PARAMETRES INTERRUPTS :
volatile bool basculeV = false;
int hitBV = 0;
volatile bool basculeR = false;
int hitBR = 0;

// PARAMETRES PROGRAMMES :
int mode = CONFIGURATION;

int LUMIN_LOW = 255;
int LUMIN_HIGH = 768;

int MIN_TEMP_AIR = -10;
int MAX_TEMP_AIR = 60;

int PRESSURE_MIN = 850;
int PRESSURE_MAX = 1080;

int H;
int MIN;
int SEC;

int MOIS;
int JOUR;
int ANNEE;

int DAY;

long LOG_INTERVALL = 600000;
int value;

// VERSION DU PROGRAMME
float version = 1.36;

void setup() {
    //Lancemement du moniteur série :
    Serial.begin(9600);
    while (!Serial) {
        ; // attente de la connexion du moniteur série
    }

    leds.init(); // initialisation des leds 

    SoftSerial.begin(9600); // Ouverture SoftwareSerial pour le GPS

    // Initialisation de la clock 
    clock.begin();
    clock.fillByYMD(2021, 11, 05);
    clock.fillByHMS(12, 00, 00);
    clock.fillDayOfWeek(FRI);
    clock.setTime(); // écrire le temps à la puce RTC

    // Initialisation boutons :
    pinMode(boutonvert, INPUT);
    pinMode(boutonrouge, INPUT);

    // Allumage de base LED rgb :
    leds.setColorHSL(0, 0.50, 0.75, 0.5);
    Serial.println("/////////////////////////////////////////////       DEMARRAGE       /////////////////////////////////////////////");
    Serial.println("Port série, Horloge RTC, Boutons initialisées");

    // Initialisation programme :
    Serial.println("/////////////////////////////////////////////////////////////////////////////////////////////////////////////////");
    init_interruptboutonV();
    init_interruptboutonR();
    Serial.println("/////////////////////////////////////////////////////////////////////////////////////////////////////////////////");
    verifcapteurs();
}

void loop() {
    codeclock = millis();
    if (mode == CONFIGURATION) {
        leds.setColorHSL(0, 0.125, 0.85, 0.5);
        int rep;
        int reset;
        int chn = 0;
        Serial.println("");
        Serial.println("");
        Serial.println("");
        Serial.println("1 : Modification des paramètres du capteur de luminositée");
        Serial.println("2 : Modification des paramètres du capteur de température de l'air");
        Serial.println("3 : Modification des paramètres du capteur de pression atmosphérique");
        Serial.println("4 : Configuration de l'heure du jour");
        Serial.println("5 : Configuration de la date du jour");
        Serial.println("6 : Configuration du jour de la semaine");
        Serial.println("7 : Configuration du temps entre 2 mesures");
        Serial.println("8 : Remise à zéro de nos paramètres (RESET)");
        Serial.println("9 : Afficher la version du programme");
        Serial.println("10 : Quitter le mode configuration");
        Serial.println("");
        Serial.println("");
        Serial.println("");
        while (Serial.available() == 0) {
            ;
        }
        if (Serial.available() != 0) {
            rep = Serial.parseInt();
        }
        switch (rep) {
        
        case 1:
            Serial.println("Modification des paramètres du capteur de luminositée");
            Serial.println("Quelle est la valeur en dessous de laquelle la luminositée est considérée comme faible ? [0 - 1023]");
            Serial.print("Valeur actuelle : ");
            Serial.println(LUMIN_LOW);
            reset = Serial.parseInt();
            while (Serial.available() == 0) {
                ;
            }
            if (Serial.available() != 0) {
                LUMIN_LOW = Serial.parseInt();
            }
            Serial.print("Valeur modifiée : ");
            Serial.println(LUMIN_LOW);
            Serial.println("Quelle est la valeur au dessus de laquelle la luminositée est considérée comme forte ? [0 - 1023]");
            Serial.print("Valeur actuelle : ");
            Serial.println(LUMIN_HIGH);
            reset = Serial.parseInt();
            while (Serial.available() == 0) {
                ;
            }
            if (Serial.available() != 0) {
                LUMIN_HIGH = Serial.parseInt();
            }
            Serial.print("Valeur modifiée : ");
            Serial.println(LUMIN_HIGH);
            reset = Serial.parseInt();
            break;

        case 2:
            Serial.println("Modification des paramètres du capteur de température de l'air");
            Serial.println("Quelle est la température en dessous de laquelle le capteur est en erreur ? [-40 - 85]");
            Serial.print("Valeur actuelle : ");
            Serial.println(MIN_TEMP_AIR);
            reset = Serial.parseInt();
            while (Serial.available() == 0) {
                ;
            }
            if (Serial.available() != 0) {
                MIN_TEMP_AIR = Serial.parseInt();
            }
            Serial.print("Valeur modifiée : ");
            Serial.println(MIN_TEMP_AIR);
            Serial.println("Quelle est la température au dessus de laquelle le capteur est en erreur ? [-40 - 85]");
            Serial.print("Valeur actuelle : ");
            Serial.println(MAX_TEMP_AIR);
            reset = Serial.parseInt();
            while (Serial.available() == 0) {
                ;
            }
            if (Serial.available() != 0) {
                MAX_TEMP_AIR = Serial.parseInt();
            }
            Serial.print("Valeur modifiée : ");
            Serial.println(MAX_TEMP_AIR);
            reset = Serial.parseInt();
            break;

        case 3:
            Serial.println("Modification des paramètres du capteur de pression atmosphérique");
            Serial.println("Quelle est la pression en dessous de laquelle le capteur est en erreur ? [300 - 1100]");
            Serial.print("Valeur actuelle : ");
            Serial.println(PRESSURE_MIN);
            reset = Serial.parseInt();
            while (Serial.available() == 0) {
                ;
            }
            if (Serial.available() != 0) {
                PRESSURE_MIN = Serial.parseInt();
            }
            Serial.print("Valeur modifiée : ");
            Serial.println(PRESSURE_MIN);
            Serial.println("Quelle est la pression au dessus de laquelle le capteur est en erreur ? [300 - 1100]");
            Serial.print("Valeur actuelle : ");
            Serial.println(PRESSURE_MAX);
            reset = Serial.parseInt();
            while (Serial.available() == 0) {
                ;
            }
            if (Serial.available() != 0) {
                PRESSURE_MAX = Serial.parseInt();
            }
            Serial.print("Valeur modifiée : ");
            Serial.println(PRESSURE_MAX);
            reset = Serial.parseInt();
            break;

        case 4:
            Serial.println("Configuration de l'heure du jour");
            Serial.println("HEURE ? [0-23]");
            reset = Serial.parseInt();
            while (Serial.available() == 0) {
                ;
            }
            if (Serial.available() != 0) {
                H = Serial.parseInt();
            }
            Serial.println("MINUTE ? [0-59]");
            reset = Serial.parseInt();
            while (Serial.available() == 0) {
                ;
            }
            if (Serial.available() != 0) {
                MIN = Serial.parseInt();
            }
            Serial.println("SECONDE ? [0-59]");
            reset = Serial.parseInt();
            while (Serial.available() == 0) {
                ;
            }
            if (Serial.available() != 0) {
                SEC = Serial.parseInt();
            }
            clock.fillByHMS(H, MIN, SEC);
            clock.setTime();
            reset = Serial.parseInt();
            break;

        case 5:
            Serial.println("Configuration de la date du jour");
            Serial.println("MOIS ? [1-12]");
            reset = Serial.parseInt();
            while (Serial.available() == 0) {
                ;
            }
            if (Serial.available() != 0) {
                MOIS = Serial.parseInt();
            }
            Serial.println("JOUR ? [1-31]");
            reset = Serial.parseInt();
            while (Serial.available() == 0) {
                ;
            }
            if (Serial.available() != 0) {
                JOUR = Serial.parseInt();
            }
            Serial.println("ANNEE ? [2000-2099]");
            reset = Serial.parseInt();
            while (Serial.available() == 0) {
                ;
            }
            if (Serial.available() != 0) {
                ANNEE = Serial.parseInt();
            }
            clock.fillByYMD(ANNEE, MOIS, JOUR);
            clock.setTime();
            reset = Serial.parseInt();
            break;

        case 6:
            Serial.println("Configuration du jour de la semaine");
            Serial.println("JOUR ? [1 : MON, 2 : TUE, 3 : WED, 4 : THU, 5 : FRI, 6 : SAT, 7 : SUN]");
            reset = Serial.parseInt();
            while (Serial.available() == 0) {
                ;
            }
            if (Serial.available() != 0) {
                DAY = Serial.parseInt();
            }
            if (DAY == 1) {
                clock.fillDayOfWeek(MON);
            }
            if (DAY == 2) {
                clock.fillDayOfWeek(TUE);
            }
            if (DAY == 3) {
                clock.fillDayOfWeek(WED);
            }
            if (DAY == 4) {
                clock.fillDayOfWeek(THU);
            }
            if (DAY == 5) {
                clock.fillDayOfWeek(FRI);
            }
            if (DAY == 6) {
                clock.fillDayOfWeek(SAT);
            }
            if (DAY == 7) {
                clock.fillDayOfWeek(SUN);
            }
            clock.setTime();
            reset = Serial.parseInt();
            break;

        case 7:
            Serial.println("Configuration du temps entre 2 mesures");
            Serial.print("Valeur actuelle (en min) : ");
            Serial.println(LOG_INTERVALL / 60000);
            Serial.println("Temps en min ?");
            reset = Serial.parseInt();
            while (Serial.available() == 0) {
                ;
            }
            if (Serial.available() != 0) {
                value = Serial.parseInt();
            }
            LOG_INTERVALL = value * 60000;
            Serial.print("Valeur modifiée : ");
            Serial.println(LOG_INTERVALL / 60000);
            reset = Serial.parseInt();
            break;

        case 8:
            Serial.println("/////////////////////////////////////////////    RESET DE NOS PARAMETRES    /////////////////////////////////////////////");
            LUMIN_LOW = 255;
            LUMIN_HIGH = 768;
            MIN_TEMP_AIR = -10;
            MAX_TEMP_AIR = 60;
            PRESSURE_MIN = 850;
            PRESSURE_MAX = 1080;
            clock.fillByYMD(200, 01, 01);
            clock.fillByHMS(12, 00, 00);
            clock.fillDayOfWeek(SAT);
            clock.setTime();
            LOG_INTERVALL = 10;
            reset = Serial.parseInt();
            break;

        case 9:
            Serial.print("Version du programme : ");
            Serial.println(version);
            reset = Serial.parseInt();
            break;

        case 10:
            Serial.println("Sortie du mode configuration");
            Serial.println("");
            Serial.println("          DATE             /    LUMINOSITEE     /TEMPERA/PRESSION/ALTITUDE/HUMIDIT/                   GPS                /");
            mode = STANDARD;
            reset = Serial.parseInt();
            break;
        }
    }

    // BOUCLES JONGLANT ENTRE LES INTERUPTIONS ET MODES 
    if (mode == STANDARD) {
        delay(50);
        standard();
        if (hitBV == 1) {
            mode = ECONOMIQUE;
            if (hitBR == 1) {
                hitBV = 0;
            }
        }
        if (hitBR == 1) {
            mode = MAINTENANCE;
            if (hitBV == 1) {
                hitBR = 0;
            }
        }
    }

    if (mode == ECONOMIQUE) {
        delay(50);
        economique();
        if (hitBV == 1) {
            mode = STANDARD;
            if (hitBR == 1) {
                hitBV = 0;
            }
        }
        if (hitBR == 1) {
            mode = MAINTENANCE;
            if (hitBV == 1) {
                hitBR = 0;
            }
        }
    }

    if (mode == MAINTENANCE) {
        delay(50);
        maintenance();
        if (hitBV == 1) {
        }
        if (hitBR == 1) {
            mode = STANDARD;
            if (hitBV == 1) {
                hitBR = 0;
            }
        }
    }
}

void standard() {
    leds.setColorHSL(0, 0.25, 0.75, 0.25);
    recupdata(LOG_INTERVALL, MEMORY);
}

void economique() {
    leds.setColorHSL(0, 0.625, 0.75, 0.25);
    recupdata((2 * LOG_INTERVALL), MEMORY);
}

void maintenance() {
    leds.setColorHSL(0, 0.08, 0.85, 0.25);
    recupdata(LOG_INTERVALL, SERIE);
}

void verifcapteurs() {
    Serial.print("Initialisation de la carte SD : ");
    if (!SD.begin(chipSelect)) { // Vérifie si la carte SD est présente et si elle peut être initialisée
        Serial.println("      Carte indisponible");
        while (!SD.begin(chipSelect)) {
            leds.setColorHSL(0, 0, 0.75, 0.5);
            delay(1000);
            leds.setColorHSL(0, 0.5, 0, 0.5);
            delay(2000);
        } // attendre la présence de la carte
        Serial.println("      Carte disponible");
    }
    Serial.println("      La carte SD est fonctionnelle");

    Serial.print("Vérification des capteurs : ");
    if (!bme.begin(0x76)) {
        Serial.println("      Capteur BME280 (Température / Pression / Altitude / Humiditée) hors service, vérification branchement nécéssaire");
        while (!bme.begin(0x76)) {
            leds.setColorHSL(0, 0, 0.75, 0.5);
            delay(1000);
            leds.setColorHSL(0, 0.25, 0.75, 0.5);
            delay(1000);
        }
    }
    Serial.println("      Capteurs opérationnels ");
    delay(500);
    Serial.print("Vérification GPS : ");
    if (!SoftSerial.available()) {
        Serial.println("      Erreur GPS, vérification branchement nécéssaire");
        while (!SoftSerial.available()) {
            leds.setColorHSL(0, 0, 0.75, 0.5);
            delay(1000);
            leds.setColorHSL(0, 0.18, 0.75, 0.5);
            delay(1000);
        }
    }
    Serial.println("      GPS fonctionnel");
    Serial.println("/////////////////////////////////////////////////////////////////////////////////////////////////////////////////");
}

void recupdata(long timer, int datamode) {
    if ((codeclock % timer) > (timer - 500)) {
        // make a string for assembling the data to log:
        String dataString = "";
        // Read time from RTC CLock
        dataString += getTime() + " ; ";
        int nb_valeur_moy = 10;
        float tabCapteurluminositee[nb_valeur_moy];
        float tabCapteurtemperature[nb_valeur_moy];
        float tabCapteurpression[nb_valeur_moy];
        float tabCapteuraltitude[nb_valeur_moy];
        float tabCapteurhumiditee[nb_valeur_moy];

        //Récuperation des données
        for (int i = 0; i < nb_valeur_moy; i++) { 
            tabCapteurluminositee[i] = analogRead(analogPin);
            tabCapteurtemperature[i] = bme.readTemperature();
            tabCapteurpression[i] = bme.readPressure() / 100.0F;
            tabCapteuraltitude[i] = bme.readAltitude(SEALEVELPRESSURE_HPA);
            tabCapteurhumiditee[i] = bme.readHumidity();
        } 
        float sommeluminositee = 0;
        float sommetemperature = 0;
        float sommepression = 0;
        float sommealtitude = 0;
        float sommehumiditee = 0;

        // Sommes pour les moyennes 
        for (int j = 0; j < nb_valeur_moy; j++) {
            sommeluminositee += tabCapteurluminositee[j];
            sommetemperature += tabCapteurtemperature[j];
            sommepression += tabCapteurpression[j];
            sommealtitude += tabCapteuraltitude[j];
            sommehumiditee += tabCapteurhumiditee[j];
        }
        // Calcul des différentes moyennes
        float moyenneluminositee = (float)sommeluminositee / (float)nb_valeur_moy;
        float moyennetemperature = (float)sommetemperature / (float)nb_valeur_moy;
        float moyennepression = (float)sommepression / (float)nb_valeur_moy;
        float moyennealtitude = (float)sommealtitude / (float)nb_valeur_moy;
        float moyennehumiditee = (float)sommehumiditee / (float)nb_valeur_moy;

        if (moyenneluminositee < LUMIN_LOW) {
            dataString += moyenneluminositee;
            dataString += "     faible";
        }
        if (moyenneluminositee > LUMIN_HIGH) {
            dataString += moyenneluminositee;
            dataString += "     forte";
        }
        else if ((moyenneluminositee > LUMIN_LOW) && (moyenneluminositee < LUMIN_HIGH)) {
            dataString += moyenneluminositee;
            dataString += "     moyenne";
        }
        dataString += " ; ";

        if (moyennetemperature < MIN_TEMP_AIR) {
            dataString += "ERREUR";
        }
        if (moyennetemperature > MAX_TEMP_AIR) {
            dataString += "ERREUR";
        }
        else if ((moyennetemperature > MIN_TEMP_AIR) && (moyennetemperature < MAX_TEMP_AIR)) {
            dataString += moyennetemperature;
        }
        dataString += " ; ";

        if (moyennepression < PRESSURE_MIN) {
            dataString += "ERREUR";
        }
        if (moyennepression > PRESSURE_MAX) {
            dataString += "ERREUR";
        }
        else if ((moyennepression > PRESSURE_MIN) && (moyennepression < PRESSURE_MAX)) {
            dataString += moyennepression;
        }
        dataString += " ; ";
        dataString += moyennealtitude;
        dataString += " ; ";
        dataString += moyennehumiditee;
        dataString += " ; ";

        // GPS Reading
        gpsData = "";
        // si des données proviennent du moniteur émulé ==> des données proviennent du gps
        if (SoftSerial.available())
        {
            t = true;
            while (t) {
                gpsData = SoftSerial.readStringUntil('\n');
                if (gpsData.startsWith("$GPGGA", 0)) {
                    t = false;
                }
            }
        }
        dataString += gpsData;
        // ouvrir le fichier (1 seul fichier en même temps) 
        if (datamode == MEMORY) {
            File dataFile = SD.open("data.txt", FILE_WRITE);
            // si le fichier est disponnible
            if (dataFile) {
                dataFile.println(dataString);
                dataFile.close();
            }

            // si le fichier ne s'ouvre pas, afficher une erreur:
            else {
                Serial.println("Erreur d'accès au fichier Donnees.txt");
                leds.setColorHSL(0, 0, 0.75, 0.5);
                delay(1000);
                leds.setColorHSL(0, 0.5, 0, 1);
                delay(2000);
            }
        }
        if (datamode == SERIE) {
            // print to the serial port too:
            Serial.println(dataString);
        }
    }
    else return;
}

String getTime() {
    String time = "";
    clock.getTime();
    time += String(clock.hour, DEC);
    time += String(":");
    time += String(clock.minute, DEC);
    time += String(":");
    time += String(clock.second, DEC);
    time += String("  ");
    time += String(clock.month, DEC);
    time += String("/");
    time += String(clock.dayOfMonth, DEC);
    time += String("/");
    time += String(clock.year + 2000, DEC);
    time += String(" ");
    time += String(clock.dayOfMonth);
    time += String("*");
    switch (clock.dayOfWeek)// Friendly printout the weekday
    {
    case MON:
        time += String("MON");
        break;
    case TUE:
        time += String("TUE");
        break;
    case WED:
        time += String("WED");
        break;
    case THU:
        time += String("THU");
        break;
    case FRI:
        time += String("FRI");
        break;
    case SAT:
        time += String("SAT");
        break;
    case SUN:
        time += String("SUN");
        break;
    }
    time += String(" ");
    return time;
}

void basculerV() {
    if (hitBV == 1) {
        hitBV = 0;
        return;
    }

    if (hitBV == 0) {
        hitBV = 1;
    }
}

void basculerR() {
    if (hitBR == 1) {
        hitBR = 0;
        return;
    }

    if (hitBR == 0) {
        hitBR = 1;
    }
}

void init_interruptboutonV(void) {
    attachInterrupt(digitalPinToInterrupt(boutonvert), basculerV, FALLING);
    Serial.println("Interrupt Bouton Vert : ON");
}

void init_interruptboutonR(void) {
    attachInterrupt(digitalPinToInterrupt(boutonrouge), basculerR, FALLING);
    Serial.println("Interrupt Bouton Rouge : ON");
}