// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#if !defined(ARDUINO_ESP32_DEV) && !defined(ARDUINO_INKPLATE6V2)
#error "Wrong board selection for this example, please select e-radionica Inkplate6 or Soldered Inkplate6 in the boards menu."
#endif

#include "WiFi.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "Inkplate.h"            //Include Inkplate library to the sketch
Inkplate display(INKPLATE_1BIT); // Create object on Inkplate library and set library to work in monochorme mode\

#include "Button.h" 
#include "OdooAPI.h"

#define DELAY_MS 5000
// Delay in milliseconds between screen refresh. Refreshing e-paper screens more often than 5s is not
// recommended
std::vector<Bouton> boutons; // Votre vecteur de boutons
AuthResult auth;
// Paramètres NTP
const long utcOffsetInSeconds = 3600;  // UTC offset en secondes (ex. 3600 pour UTC+1)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

String padTitle="Test title";
std::function<void()> previousAction; 

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin("YourSSID", "Your Password"); //Set your SSID and password for wifi
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

float convertSecondsToDecimalHours(int totalSeconds) {
    float hours = totalSeconds / 3600.0; // Convertir les secondes en heures
    int intHours = int(hours); // Partie entière des heures
    float decimalMinutes = (hours - intHours) * 60; // Convertir la partie décimale en minutes
    float decimalHours = intHours + decimalMinutes / 60.0; // Convertir les minutes en base 10 et les ajouter aux heures

    return decimalHours;
}
String getCurrentDate(){
  timeClient.update();

    // Obtenir l'heure et la date actuelles
    unsigned long epochTime = timeClient.getEpochTime();

    // Convertir le timestamp en structure tm
    struct tm *ptm = gmtime((time_t *)&epochTime);
    
    // Ajuster le fuseau horaire si nécessaire
    ptm->tm_hour += 3; // Remplacer '3' par votre décalage horaire

    // Formatter la date
    char dateBuffer[11]; // MM/DD/YYYY + '\0'
    snprintf(dateBuffer, sizeof(dateBuffer), "%04d-%02d-%02d", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday);

    Serial.println(dateBuffer);
    return String(dateBuffer);
    }
void drawTitle(){
  display.setTextSize(2);
  display.setCursor(150, 12); 
  display.print(padTitle);
  }
void drawButtons(std::vector<Bouton> boutons){
    int i=0;
    int j=0;
    display.clearDisplay();
     drawTitle();
    for(int k=0;k<boutons.size();k++){
      boutons[k].x0=(i+1)*buttonMargin+i*buttonWidth;
      boutons[k].y0=(j+1)*buttonMargin+j*buttonHeight;
      boutons[k].drawButton();
      if(i==3)
        {
          if(j==1){break;}
          i=0;
          j++;
        }
        else
        {
          i++;
        }
  }
  display.display();  
  }


void triggerTaskTimer(int buttonID,int taskId,int projectId,String tag){
   if(boutons[buttonID].etat){
        timeClient.update();
        boutons[buttonID].etat =false ;
        Serial.print("Time diff is equal to [s]");
        Serial.println(timeClient.getEpochTime()-boutons[buttonID].startTime);
        int secondsSinceActivation = timeClient.getEpochTime()-boutons[buttonID].startTime;
        String dateString = getCurrentDate();
        UpdateTimeSheet(auth.session_id,auth.uid,taskId,projectId,convertSecondsToDecimalHours(secondsSinceActivation),dateString);
        getTaskAndDisplay(projectId,String(),tag);
      }
   else{
        timeClient.update();
        boutons[buttonID].etat =true ;
        boutons[buttonID].startTime = timeClient.getEpochTime();
       }
  drawButtons(boutons);
      }
      
void getTaskAndDisplay(int project_id,String displayName,String tag){
  DynamicJsonDocument responseDoc =  getTaskByProjectID(auth.session_id,project_id);
  JsonArray records = responseDoc["result"]["records"];
  boutons.clear();
  if(!displayName.isEmpty()) { padTitle += " -> " + displayName; }
  int i=0;
  for (JsonObject record : records) {
    const char* displayName = record["name"];
    float total_hours_spent =record["total_hours_spent"];
    int taskID = record["id"];
    Serial.println(displayName);
    boutons.emplace_back(displayName,std::to_string(total_hours_spent)+" H", false,[i,taskID,project_id,tag]() {triggerTaskTimer(i,taskID,project_id,tag); },0,0);
    i++;
    }
     drawButtons(boutons);
     previousAction = [tag]() { getProjectsByTags(tag); };
  }
  
void getProjectsByTags(String tag){
  Serial.print("In get project by tags !");
  Serial.println(tag);
  padTitle = tag;
  DynamicJsonDocument responseDoc = getProjectDataByTags(auth.session_id,tag);
    JsonArray records = responseDoc["result"]["records"];
    boutons.clear();
    for (JsonObject record : records) {
        int id = record["id"];
        Serial.print("id : ");
         Serial.println(id);
        int hours = getRemainingsHours(auth.session_id,id);
        const char* displayName = record["display_name"];
        // Si displayName est "Internal", passe à l'itération suivante
        if (String(displayName) == "Internal") {
            continue;
        }
        boutons.emplace_back(displayName,std::to_string(hours)+" H", false, [id,displayName,tag]() { getTaskAndDisplay(id,String(displayName),tag); },0,0);
    }
    drawButtons(boutons);
    previousAction = displayMainMenu;
  }

void displayMainMenu(){
  // Assuming a method_id for demonstration purposes.
    DynamicJsonDocument responseTagsDoc = getProjectTags(auth.session_id);
    JsonArray recordsTags = responseTagsDoc["result"];
    int i=0;
    int j=0;
    boutons.clear();
    padTitle = "SmartPad V0.1";
    display.clearDisplay();
    for(JsonArray record : recordsTags){
      // Chaque 'record' est une JsonArray avec deux éléments
      int tagId = record[0]; // Premier élément: ID
      const char* tagName = record[1]; // Deuxième élément: Nom du tag
  
      Serial.println(tagName);
      boutons.emplace_back(tagName,"", false, [tagName]() { getProjectsByTags(tagName); },0,0);
  }
    drawButtons(boutons);
  }
  
void setup()
{   
      //UpdateTimeSheet(auth.session_id,auth.uid,4,35.23);
    display.begin();        // Init library (you should call this function ONLY ONCE)
    display.clearDisplay(); // Clear any data that may have been in (software) frame buffer.
    display.setCursor(150, 320);
    display.setTextSize(4);
    display.print("Welcome to odooPad!");
    display.setCursor(150, 360);
    display.setTextSize(2);
    display.print("Created by me");
    display.display(); // Write hello message
    Serial.begin(115200);
    Serial.println("Welcome to odooPad");
    initWiFi(); 
    // Initialisation du client NTP
    timeClient.begin(); 

    //Auth to odoo
    auth = authenticate_odoo();
    Serial.println("Session ID: " + auth.session_id);
    Serial.println("UID: " + String(auth.uid));
    displayMainMenu();
    previousAction = displayMainMenu;
}

void loop()
{
 
 timeClient.update();
 delay(10);
if (Serial.available() > 0) {
        String input = Serial.readString(); // Lire les données entrantes
        input.trim(); // Enlève les espaces blancs et les nouvelles lignes

         // Convertir la chaîne de caractères en entier
        int inputInt = input.toInt();
        if(inputInt>7){previousAction();}
        else{
            boutons[inputInt].action();
        }
    }
}
