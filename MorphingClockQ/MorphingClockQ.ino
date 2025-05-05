#include <TimeLib.h>
#include <NtpClientLib.h>
#include <WiFiClient.h>

#include <ESPAsyncWebServer.h>

#include <PxMatrix.h>
#include <ArduinoJson.h>
#include "TinyFont.h"
#include "Digit.h"
#include "Digitsec.h"
#include "params.h"   

#include <ArduinoOTA.h>



//ESP8266 setup
#ifdef ESP8266
#include <Ticker.h>
Ticker display_ticker;
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_D 12
#define P_E 0
#define P_OE 2
#endif


PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D, P_E);



byte hh;
byte hh24;
byte mm;
byte ss;
byte ntpsync = 1;
  const char ntpsvr[]   = "pool.ntp.org";
//const char ntpsvr[] = "time.google.com";


const char *WiFi_hostname = "MorphClockQ2";


byte digit_offset_amount;

byte Digitsec_x = 56 + digit_offset_amount;
byte Digit_x = 62 + digit_offset_amount;

Digitsec digit0(&display, 0, Digitsec_x - 7 * 1, 14, display.color565(255, 255, 255));
Digitsec digit1(&display, 0, Digitsec_x - 7 * 2, 14, display.color565(255, 255, 255));
Digit digit2(&display, 0, Digit_x - 4 - 9 * 3, 8, display.color565(255, 255, 255));
Digit digit3(&display, 0, Digit_x - 4 - 9 * 4, 8, display.color565(255, 255, 255));
Digit digit4(&display, 0, Digit_x - 7 - 9 * 5, 8, display.color565(255, 255, 255));
Digit digit5(&display, 0, Digit_x - 7 - 9 * 6, 8, display.color565(255, 255, 255));



int color_disp = display.color565(40, 40, 50);  // primary color



// R G B
int cc_blk = display.color565(0, 0, 0);         // black
int cc_wht = display.color565(25, 25, 25);      // white
int cc_bwht = display.color565(255, 255, 255);  // bright white
int cc_red = display.color565(50, 0, 0);        // red
int cc_bred = display.color565(255, 0, 0);      // bright red
int cc_org = display.color565(25, 10, 0);       // orange
int cc_borg = display.color565(255, 165, 0);    // bright orange
int cc_grn = display.color565(0, 45, 0);        // green
int cc_bgrn = display.color565(0, 255, 0);      // bright green
int cc_blu = display.color565(0, 0, 150);       // blue
int cc_bblu = display.color565(0, 128, 255);    // bright blue
int cc_ylw = display.color565(45, 45, 0);       // yellow
int cc_bylw = display.color565(255, 255, 0);    // bright yellow
int cc_gry = display.color565(10, 10, 10);      // gray
int cc_bgry = display.color565(128, 128, 128);  // bright gray
int cc_dgr = display.color565(3, 3, 3);         // dark grey
int cc_cyan = display.color565(0, 30, 30);      // cyan
int cc_bcyan = display.color565(0, 255, 255);   // bright cyan
int cc_ppl = display.color565(25, 0, 25);       // purple
int cc_bppl = display.color565(255, 0, 255);    // bright purple

int cc_time;
int cc_wind;
int cc_date;
int cc_wtext;

int ani_speed = 500;      
int weather_refresh = 10;  
int morph_off = 0;        


byte nosee = 100;

byte img_x = 7 * TF_COLS + 1;
byte img_y = 2;

byte date_x = 2;
byte date_y = 26;

byte tmp_x = 12 * TF_COLS;
byte tmp_y = 2;

//  byte wind_x = 1*TF_COLS;
byte wind_x = 0;
byte wind_y = 2;

byte humi_x = 0;
byte humi_y = 2;
char humi_label[] = "%";

byte wtext_x = 5 * TF_COLS - 3;
byte wtext_y = 2;

String wind_lstr = "   ";
String humi_lstr = "   ";
int wind_humi = 0;   
//bool UseWTcode = true;
bool drawWeather = false;
int lcc;
String wind_direction[10] = { "  ", "N ", "NE", "E ", "SE", "S ", "SW", "W ", "NW", "N " };
String weather_text[27] = {"        ","CLEARSKY","P-CLOUDY","OVERCAST","  RAIN  ","T-STORMS","  SNOW  ","  HAZY  "," CLEAR  "," FOGGY  "," CLOUDY ","BLIZZARD",
                           " MISTY  ", "FLURRIES", "  SLEET ", "FRZ RAIN", " SHOWER ", "DRIZZLE ", "  HAIL  ", "  SMOKE ", "SANDDUST", "FRZ FOG ", "FRZ DRIZ", "SNOWRAIN", "THUNDER ",
                           "ICE PELT", " SUNNY  " };

String weatherserver = "";
bool ForceWeatherCheck = false; 

//JSON 
JsonDocument doc;

WiFiClient client;   

int tempM = -10000;
int humiM = -10000;
int wind_speed = -10000;
int wind_dir = -10000;

int codeWA = -1;
int codeWT = 0;
int isDay = -10000;
int weatherCode;
String tmp_line = "";  
String condS = "";


int failed_connect = 0;


#define NVARS 19
#define LVARS 50
char c_vars[NVARS][LVARS];

typedef enum e_vars {
  EV_SSID,
  EV_PASS,
  EV_TZ,
  EV_24H,
  EV_METRIC,
  EV_DATEFMT,
  EV_APIKEY,
  EV_POSTAL,
  EV_DST,
  EV_WANI,
  EV_PALET,
  EV_BRIGHT,
  EV_LONG,
  EV_LAT,
  EV_COUNTRY,
  EV_WSERVICE,
  EV_NIGHTB,
  EV_NIGHTE,
  EV_WDEFINE     
};

AsyncWebServer server(80);
String index_html = "<!DOCTYPE html><html><body>\
<form id='configForm'>\
<h2>Morphing ClockQ Configuration</h2>\
SSID:     <input type='text' name='wifi_ssid' value='%WIFI_SSID%' maxlength='50'><br>\
Password: <input type='password' name='wifi_pass' value='%WIFI_PASS%' maxlength='50'><br><br>\
Timezone: <input type='text' name='timezone' value='%TIMEZONE%' maxlength='3'><br>\
24 Hour format: <input type='text' name='military' value='%MILITARY%'><br>\
Date Format: <input type='text' name='date_fmt' value='%DATE_FMT%'><br>\
Daylight Savings Time (true/false): <input type='text' name='dst_sav' value='%DST_SAV%'><br><br>\
<p>1=Cyan, 2=Red, 3=Blue, 4=Yellow, 5=Bright Blue, 6=Orange, 7=Green</p>\
Color Palette: <input type='number' name='c_palet' value='%C_PALET%' maxlength='3' min='1' max='7'><br>\
Brightness: <input type='number' name='brightness' value='%BRIGHTNESS%' maxlength='2' min='20' max='70'>\
<p>Dim the display for night time hours in 24hr format, enter 0 in both fields to disable</p>\
Night Start: <input type='number' name='nightStart' value='%NIGHT_START%' maxlength='2' min='0' max='24'><br>\
Night End: <input type='number' name='nightEnd' value='%NIGHT_END%' maxlength='2' min='0' max='24'><br><br>\
<p>Read the weather docs for all the details - weather.pdf</p>\
<p>1=WeatherAPI.com, 2=WeatherBit.io, 3=PirateWeather.net, 4=OpenMeteo.io, 5=WeatherUnlocked</p>\
Weather Service: <input type='number' name='weatherservice' value='%WEATHER_SERVICE%' maxlength='3' min='1' max='9'><br>\
API Key:         <input type='text' name='apiKey' value='%API_KEY%' maxlength='50'><br>\
Postal Code:     <input type='text' name='postal_code' value='%POSTAL_CODE%'><br>\
Country Code:    <input type='text' name='country_code' value='%COUNTRY_CODE%'><br>\
Latitude:        <input type='text' name='latitude' value='%LATITUDE%'><br>\
Longitude:       <input type='text' name='longitude' value='%LONGITUDE%'><br>\
Define:          <input type='text' name='wdefine' value='%WDEFINE%'><br>\
Metric (Y/N):    <input type='text' name='u_metric' value='%U_METRIC%' maxlength='1'><br>\
Weather Animation (Y/N): <input type='text' name='w_animation' value='%W_ANIMATION%' maxlength='1'><br><br>\
<input type='button' onclick='saveSettings()' value='Save'>\
</form>\
<script>\
function saveSettings() {\
var form = document.getElementById('configForm');\
var formData = new FormData(form);\
fetch('/save', {method: 'POST',body: formData}).then(response => {}).catch(error => {});\
}\
</script>\
</body></html>";


void handleRoot(AsyncWebServerRequest *request) {
  String page = index_html;

  page.replace("%WIFI_SSID%", c_vars[EV_SSID]);
  page.replace("%WIFI_PASS%", c_vars[EV_PASS]);
  page.replace("%TIMEZONE%", c_vars[EV_TZ]);
  page.replace("%MILITARY%", c_vars[EV_24H]);
  page.replace("%DATE_FMT%", c_vars[EV_DATEFMT]);
  page.replace("%DST_SAV%", c_vars[EV_DST]);
  page.replace("%C_PALET%", c_vars[EV_PALET]);
  page.replace("%BRIGHTNESS%", c_vars[EV_BRIGHT]);
  page.replace("%NIGHT_START%", c_vars[EV_NIGHTB]);
  page.replace("%NIGHT_END%", c_vars[EV_NIGHTE]);
  page.replace("%WEATHER_SERVICE%", c_vars[EV_WSERVICE]);
  page.replace("%API_KEY%", c_vars[EV_APIKEY]);
  page.replace("%POSTAL_CODE%", c_vars[EV_POSTAL]);
  page.replace("%COUNTRY_CODE%", c_vars[EV_COUNTRY]);
  page.replace("%LATITUDE%", c_vars[EV_LAT]);
  page.replace("%LONGITUDE%", c_vars[EV_LONG]);
  page.replace("%U_METRIC%", c_vars[EV_METRIC]);
  page.replace("%W_ANIMATION%", c_vars[EV_WANI]);
  page.replace("%WDEFINE%", c_vars[EV_WDEFINE]);

  request->send(200, "text/html", page);
}

// Handle save request
void handleSave(AsyncWebServerRequest *request) {
  for (uint8_t i = 0; i < request->params(); i++) {
    if (request->getParam(i)->name() == "wifi_ssid") {
      strcpy(c_vars[EV_SSID], request->getParam(i)->value().c_str());
    } else if (request->getParam(i)->name() == "wifi_pass") {
      strcpy(c_vars[EV_PASS], request->getParam(i)->value().c_str());
    } else if (request->getParam(i)->name() == "timezone") {
      strcpy(c_vars[EV_TZ], request->getParam(i)->value().c_str());
    } else if (request->getParam(i)->name() == "military") {
        String upperTMP = request->getParam(i)->value();
        upperTMP.toUpperCase();
        strcpy(c_vars[EV_24H], upperTMP.c_str());
    } else if (request->getParam(i)->name() == "date_fmt") {
        String upperTMP = request->getParam(i)->value();
        upperTMP.toUpperCase();
        strcpy(c_vars[EV_DATEFMT], upperTMP.c_str());
    } else if (request->getParam(i)->name() == "dst_sav") {
      strcpy(c_vars[EV_DST], request->getParam(i)->value().c_str());
    } else if (request->getParam(i)->name() == "c_palet") {
      strcpy(c_vars[EV_PALET], request->getParam(i)->value().c_str());
    } else if (request->getParam(i)->name() == "brightness") {
      strcpy(c_vars[EV_BRIGHT], request->getParam(i)->value().c_str());
    } else if (request->getParam(i)->name() == "nightStart") {
      strcpy(c_vars[EV_NIGHTB], request->getParam(i)->value().c_str());
    } else if (request->getParam(i)->name() == "nightEnd") {
      strcpy(c_vars[EV_NIGHTE], request->getParam(i)->value().c_str());
    } else if (request->getParam(i)->name() == "weatherservice") {
      strcpy(c_vars[EV_WSERVICE], request->getParam(i)->value().c_str());
    } else if (request->getParam(i)->name() == "apiKey") {
      strcpy(c_vars[EV_APIKEY], request->getParam(i)->value().c_str());
    } else if (request->getParam(i)->name() == "postal_code") {
      strcpy(c_vars[EV_POSTAL], request->getParam(i)->value().c_str());
    } else if (request->getParam(i)->name() == "country_code") {
      strcpy(c_vars[EV_COUNTRY], request->getParam(i)->value().c_str());
    } else if (request->getParam(i)->name() == "latitude") {
      strcpy(c_vars[EV_LAT], request->getParam(i)->value().c_str());
    } else if (request->getParam(i)->name() == "longitude") {
      strcpy(c_vars[EV_LONG], request->getParam(i)->value().c_str());
    } else if (request->getParam(i)->name() == "u_metric") {
        String upperTMP = request->getParam(i)->value();
        upperTMP.toUpperCase();
        strcpy(c_vars[EV_METRIC], upperTMP.c_str());
    } else if (request->getParam(i)->name() == "w_animation") {
        String upperTMP = request->getParam(i)->value();
        upperTMP.toUpperCase();
        strcpy(c_vars[EV_WANI], upperTMP.c_str());
    } else if (request->getParam(i)->name() == "wdefine") {
      strcpy(c_vars[EV_WDEFINE], request->getParam(i)->value().c_str());
    }
  }

  vars_write();
  request->send(200, "text/plain", "Settings saved");
  delay(1000);
  ESP.restart();  //Always restart device after changes


}


void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}



void web_server() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound(notFound);
  server.begin();
}

//Setup Weather services
void select_weatherservice() {
  switch (atoi(c_vars[EV_WSERVICE])) {
    case 1:
      weatherserver = String("api.weatherapi.com");
      weather_refresh = 5;
      break;
    case 2:
      weatherserver = String("api.weatherbit.io");
      weather_refresh = 30;  //Must be at least 30 due to 50 calls per day limit
      break;
    case 3:
      weatherserver = String("api.pirateweather.net");
      weather_refresh = 5;
      break;
    case 4:
      weatherserver = String("api.open-meteo.com");
      weather_refresh = 5;
      break;
    case 5:
      weatherserver = String("api.weatherunlocked.com");
      weather_refresh = 5;
      break;
    case 7:  //Not working
      weatherserver = String("dataservice.accuweather.com");
      weather_refresh = 30;
      break;
    default:
      Serial.println("Invalid weather service setup");

  }
}



void select_palette() {
  int x;
  x = atoi(c_vars[EV_PALET]);

  switch (x) {
    case 1:
      cc_time = cc_cyan;
      cc_wind = cc_ylw;
      cc_date = cc_grn;
      cc_wtext = cc_wht;
      break;
    case 2:
      cc_time = cc_red;
      cc_wind = cc_ylw;
      cc_date = cc_blu;
      cc_wtext = cc_grn;
      break;
    case 3:
      cc_time = cc_blu;
      cc_wind = cc_grn;
      cc_date = cc_ylw;
      cc_wtext = cc_wht;
      break;
    case 4:
      cc_time = cc_ylw;
      cc_wind = cc_cyan;
      cc_date = cc_blu;
      cc_wtext = cc_grn;
      break;
    case 5:
      cc_time = cc_bblu;
      cc_wind = cc_grn;
      cc_date = cc_ylw;
      cc_wtext = cc_grn;
      break;
    case 6:
      cc_time = cc_org;
      cc_wind = cc_red;
      cc_date = cc_grn;
      cc_wtext = cc_ylw;
      break;
    case 7:
      cc_time = cc_grn;
      cc_wind = cc_ppl;
      cc_date = cc_cyan;
      cc_wtext = cc_ylw;
      break;
    default:
      cc_time = cc_cyan;
      cc_wind = cc_ylw;
      cc_date = cc_grn;
      cc_wtext = cc_wht;
      break;
  }
}


void display_updater() {
  int x;
  x = atoi(c_vars[EV_BRIGHT]);
  if (x > 70 or x < 0)
    x = 70;



 if (atoi(c_vars[EV_NIGHTB]) != 0 && atoi(c_vars[EV_NIGHTE]) != 0)
  if ( atoi(c_vars[EV_NIGHTE]) < atoi(c_vars[EV_NIGHTB]) ) {
    if (hh24 >= atoi(c_vars[EV_NIGHTB]) || hh24 < atoi(c_vars[EV_NIGHTE])) {
      x = 5;
    }
  } else {
    if ( hh24 >= atoi(c_vars[EV_NIGHTB]) && hh24 < atoi(c_vars[EV_NIGHTB]) ) {
      x = 5;
    }
  }

  display.display(x);  
}



bool toBool(String s) {
  return s.equals("true");
}

int vars_read() {
  File varf = SPIFFS.open("/mvars2.cfg", "r");
  if (!varf) {
    Serial.println("Failed to open config file");
    return 0;
  }
  for (int i = 0; i < NVARS; i++) {
    for (int j = 0; j < LVARS; j++) {
      c_vars[i][j] = (char)varf.read();
    //  Serial.print(c_vars[i][j]);
    }
  //  Serial.println(":");
  }

  varf.close();
  return 1;
}

int vars_write() {
  Serial.println("Writing config file ....");
  File varf = SPIFFS.open("/mvars2.cfg", "w");
  
  for (int i = 0; i < NVARS; i++) {
    for (int j = 0; j < LVARS; j++) {
      if (varf.write(c_vars[i][j]) != 1)
        Serial.println("error writing var");
 //     else
 //       Serial.print(c_vars[i][j]);
    }

  }
  varf.close();
  Serial.println("Finished writing config");
  return 1;
}

int vars_remove() {
 Serial.println("Remove config file");
 if (SPIFFS.exists("/mvars2.cfg")) {
    if (SPIFFS.remove("/mvars2.cfg")) {
      Serial.println("File deleted successfully");
    } else {
      Serial.println("Failed to delete file");
    }
 }
}


//Debugging
void show_config_vars() {
  Serial.println("From config file ....");
  return;

  Serial.print("SSID=");
  Serial.println(c_vars[EV_SSID]);
  Serial.print("Password=");
  Serial.println(c_vars[EV_PASS]);
  Serial.print("Timezone=");
  Serial.println(c_vars[EV_TZ]);
  Serial.print("Military=");
  Serial.println(c_vars[EV_24H]);
  Serial.print("Metric=");
  Serial.println(c_vars[EV_METRIC]);
  Serial.print("Date-Format=");
  Serial.println(c_vars[EV_DATEFMT]);
  Serial.print("apiKey=");
  Serial.println(c_vars[EV_APIKEY]);
  Serial.print("Post Code=");
  Serial.println(c_vars[EV_POSTAL]);
  Serial.print("DST=");
  Serial.println(c_vars[EV_DST]);
  Serial.print("Weather Animation=");
  Serial.println(c_vars[EV_WANI]);
  Serial.print("Color Palette=");
  Serial.println(c_vars[EV_PALET]);
  Serial.print("Brightness=");
  Serial.println(c_vars[EV_BRIGHT]);
  Serial.print("Longitude=");
  Serial.println(c_vars[EV_LONG]);
  Serial.print("Latitude=");
  Serial.println(c_vars[EV_LAT]);
  Serial.print("Country Code=");
  Serial.print(c_vars[EV_COUNTRY]);
  Serial.print("Weather Service=");
  Serial.println(c_vars[EV_WSERVICE]);
  Serial.print("Night Begin=");
  Serial.println(c_vars[EV_NIGHTB]);
  Serial.print("Night End=");
  Serial.println(c_vars[EV_NIGHTE]);
  Serial.print("Weather Define=");
  Serial.println(c_vars[EV_WDEFINE]);
  
}

//If the config file is not setup copy from params.h
void init_config_vars() {
  Serial.println("Initialize vars from params.h");
  strcpy(c_vars[EV_SSID], wifi_ssid);
  strcpy(c_vars[EV_PASS], wifi_pass);
  strcpy(c_vars[EV_TZ], timezone);
  strcpy(c_vars[EV_24H], military);
  strcpy(c_vars[EV_METRIC], u_metric);
  strcpy(c_vars[EV_DATEFMT], date_fmt);
  strcpy(c_vars[EV_APIKEY], apiKey);
  strcpy(c_vars[EV_POSTAL], postal_code);
  strcpy(c_vars[EV_DST], dst_sav);
  strcpy(c_vars[EV_WANI], w_animation);
  strcpy(c_vars[EV_PALET], c_palet);
  strcpy(c_vars[EV_BRIGHT], brightness);
  strcpy(c_vars[EV_LONG], longitude);
  strcpy(c_vars[EV_LAT], latitude);
  strcpy(c_vars[EV_COUNTRY], country_code);
  strcpy(c_vars[EV_WSERVICE], weatherservice);
  strcpy(c_vars[EV_NIGHTB], nightStart);
  strcpy(c_vars[EV_NIGHTE], nightEnd);
  strcpy(c_vars[EV_WDEFINE], wdefine);

  vars_write();

}

//Wifi Connection
int connect_wifi(String n_wifi, String n_pass) {
  int c_cnt = 0;
  Serial.print("Trying WiFi Connect:");
  Serial.println(n_wifi);

  WiFi.hostname(WiFi_hostname);  

  WiFi.begin(n_wifi, n_pass);
  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    c_cnt++;
    if (c_cnt > 50) {
      Serial.println("Wifi Connect Failed");
      return 1;
    }
  }
  Serial.println("success!");
  Serial.print("IP Address is: ");
  Serial.println(WiFi.localIP());  //
  return 0;
}

void setup_OTA()
{
    ArduinoOTA.onStart([]() {
        Serial.println("Start updating...");
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("\nUpdate Complete!");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress * 100) / total);
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
    Serial.println("Ready for OTA updates.");
}

int convert_windir(String x) {
  int w;
  w = round(((x.toInt() % 360)) / 45.0) + 1;
  return w;
}

//
//SETUP BEGIN
//
void setup() {
  Serial.begin(9600);
  while (!Serial)
    delay(500);  //delay for Leonardo
    display.begin(16);
  //     display.setDriverChip(FM6126A);
  //     display.setMuxDelay(0,1,0,0,0);

#ifdef ESP8266
  display_ticker.attach(0.002, display_updater);
#endif

  TFDrawText(&display, String(" MORPH CLOCKQ  "), 0, 1, cc_blu);
  TFDrawText(&display, String("  STARTING UP  "), 0, 10, cc_blu);

  //Use this for serial console delay to see all serial.prints for debugging
  //Serial.println("Waiting for input from keyboard before continuing .....");
  //while (Serial.available() == 0 ) {}
 

  Serial.println("Setup begin");

  if (SPIFFS.begin()) {
    if (!vars_read()) {
      Serial.println("Unable to open config file, will create it");
      init_config_vars(); 
    }
  } else {
    Serial.println("SPIFFS Initialization...failed");
  }

  String lstr = String("TIMEZONE:") + String(c_vars[EV_TZ]);
  TFDrawText(&display, lstr, 4, 24, cc_cyan);


  if (connect_wifi(c_vars[EV_SSID], c_vars[EV_PASS]) == 1) {  
    TFDrawText(&display, String("WIFI FAILED CONFIG"), 1, 10, cc_grn);
    if (connect_wifi(wifi_ssid, wifi_pass) == 1) {  
      Serial.println("Cannot connect to anything, RESTART ESP");
      TFDrawText(&display, String("WIFI FAILED PARAMS.H"), 1, 10, cc_grn);
      resetclock();
    }
  }

  TFDrawText(&display, String("WIFI CONNECTED "), 3, 10, cc_grn);
  TFDrawText(&display, String(WiFi.localIP().toString()), 4, 17, cc_grn);

  select_palette();  
  select_weatherservice();
  web_server();  

  getWeather();

  //start NTP
  Serial.print("TimeZone for NTP.Begin:");
  Serial.println(c_vars[EV_TZ]);
  NTP.begin(ntpsvr, String(c_vars[EV_TZ]).toInt(), toBool(String(c_vars[EV_DST])));
  NTP.setInterval(10);  //force rapid sync in 10sec
  //
  NTP.onNTPSyncEvent([](NTPSyncEvent_t ntpEvent) {
    switch (ntpEvent) {
      case noResponse:
        Serial.print("Time Sync error: ");
        Serial.println("NTP server not reachable");
        break;
      case invalidAddress:
        Serial.print("Time Sync error: ");
        Serial.println("Invalid NTP server address");
        break;
      default:
        Serial.print("Got NTP time: ");
        Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
        ntpsync = 1;
        break;
    }
  });

  //prep screen for clock display

  display.fillScreen(0);

  setup_OTA();
}
// End Setup

//Get the weather, parse the JSON response
void getWeather() {

  weatherCode = -10000;
  humiM = -10000;
  wind_dir = -10000;
  wind_speed = -10000;
  isDay = -10000;
  float float_tmp;
  
  bool latlong;

  ForceWeatherCheck = false;  //If we don't get a valid response override the interval for checking the weather
  bool noTemp = false;
  String pod = "";
  String uom = "";
  String cord = "";
  String url = "";
  String loc = "";
  String WeatherTXT = "";
  
  //If the weather API keys are in the array it will use them instead of the config file
   if ( apikeys[atoi(c_vars[EV_WSERVICE]) - 1].length() != 0) {
     String apiKey = apikeys[ atoi(c_vars[EV_WSERVICE]) - 1 ] ;
     strcpy(c_vars[EV_APIKEY], apiKey.c_str());
   }

  if (!sizeof(apiKey) && String(c_vars[EV_WSERVICE]) != "4") {  //Open-meteo does not require API key
    Serial.println("Weather: Missing API KEY");
    return;
  }

  if ( !client.connect(weatherserver, 80)) {
    Serial.print("Weather: Unable to connect ");
    Serial.println(weatherserver);
    ForceWeatherCheck = true;
    failed_connect++;
    if (failed_connect >= 10) {
      Serial.println("Weather: 10 failed attempts to connect.  Reset");
      resetclock();
    }
    return;
  }

  //Is Latitude & Longtitude setup?
  loc = String(c_vars[EV_LAT]) + String(c_vars[EV_LONG]);
  if (loc.length() < 3)
    latlong = false;
  else
    latlong = true;

  //Build GET request per weather service API
  switch (atoi(c_vars[EV_WSERVICE])) {
    case 1:  //weatherAPI
      if (latlong)
        loc = String(c_vars[EV_LAT]) + "," + String(c_vars[EV_LONG]);
      else
        loc = String(c_vars[EV_POSTAL]);  //No lat/long so use the postal code for location

        url = "/v1/current.json?key=" + String(c_vars[EV_APIKEY]) + "&q=" +  String(loc);
      break;
    case 2:  //WeatherBit
      if (latlong)
        loc = "lat=" + String(c_vars[EV_LAT]) + "&lon=" + String(c_vars[EV_LONG]);
      else
        loc = "postal_code=" + String(c_vars[EV_POSTAL]);

      if (String(c_vars[EV_METRIC]) == "Y")
        uom = "&units=M";
      else
        uom = "&units=I";
      
      url = "/v2.0/current?" + String(loc) + String(uom) + "&key=" + String(c_vars[EV_APIKEY]) + "&include=minutely";
      break;
    case 3:  //PirateWeather
      loc = String(c_vars[EV_LAT]) + "," + String(c_vars[EV_LONG]);
      if (String(c_vars[EV_METRIC]) == "Y")
        uom = "?units=ca";
      else
        uom = "?units=us";
      
      url = "/forecast/" + String(c_vars[EV_APIKEY]) + "/" + String(loc) + String(uom) + "&exclude=minutely,hourly,daily";
      break;
    case 4:  //Openmeteo
      if (String(c_vars[EV_METRIC]) == "Y")
        uom = "&temperature_unit=celsius&wind_speed_unit=ms";
      else
        uom = "&temperature_unit=fahrenheit&wind_speed_unit=mph";
                                                            
       url = "/v1/forecast?latitude=" + String(c_vars[EV_LAT]) + "&longitude=" + String(c_vars[EV_LONG]) + "&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m,wind_direction_10m,is_day" + String(uom);
       break;
    case 5:  //WeatherUnlocked  this service requires a 2nd key, we will use WDEFINE to store it
      if (latlong)
        loc = String(c_vars[EV_LAT]) + "," + String(c_vars[EV_LONG]);
      else
        loc = String(c_vars[EV_COUNTRY]) + "." + String(c_vars[EV_POSTAL]);

      url = "/api/current/" + String(loc) + "?app_id=" + String(c_vars[EV_WDEFINE]) + "&app_key=" + String(c_vars[EV_APIKEY]);
      break;
    case 7:  //AccuWeather (not working)
      url = "/currentconditions/v1/" + String(c_vars[EV_POSTAL]) + "?apikey=" + String(c_vars[EV_APIKEY]) + "&details=true";
      break;
    default:
      Serial.println("Invalid weather service configured");
      return;
   
  }



//Send GET Request
  client.print("GET " + url + " HTTP/1.1\r\n" + "Host: " + String(weatherserver) + "\r\n" + "Accept: application/json\r\n" + "Connection: close\r\n\r\n");

  client.setTimeout(600);    //We need to reduce the timeout delay for the readStringUntil
  Serial.print("Reading data from: ");
  Serial.println(weatherserver);

  tmp_line = client.readStringUntil('{');   
  Serial.println(tmp_line);

  tmp_line = "{" + client.readString(); 
  if (tmp_line.length() < 3) {
    Serial.println("Weather: Unable to retrieve JSON data");
    ForceWeatherCheck = true;
    return;
  }


  

//Parse JSON Weather response
DeserializationError error = deserializeJson(doc, tmp_line);
if (error) {
  Serial.print("ERROR: DeserializeJson() returned ");
  Serial.println(error.c_str());
  ForceWeatherCheck = true;
  return;
}

switch (atoi(c_vars[EV_WSERVICE])) {
  case 1:  //weatherAPI
    if (String(c_vars[EV_METRIC]) == "Y") {
      if (!doc["current"]["temp_c"]) { noTemp = true; break; }
      tempM = doc["current"]["temp_c"];
      wind_speed = doc["current"]["wind_kph"];
    }
    else {
      if (!doc["current"]["temp_f"]) { noTemp = true; break; }
      tempM = doc["current"]["temp_f"];
      wind_speed = doc["current"]["wind_mph"];
    }
    humiM = doc["current"]["humidity"];
    wind_dir = convert_windir((doc["current"]["wind_degree"]));
    weatherCode = doc["current"]["condition"]["code"];
    isDay = doc["current"]["is_day"];
    break;
  case 2:  //WeatherBit
    if (!doc["data"][0]["app_temp"]) { noTemp = true; break; }
    tempM = doc["data"][0]["app_temp"];
    wind_speed = doc["data"][0]["wind_spd"];
    wind_dir = convert_windir((doc["data"][0]["wind_dir"]));
    humiM = doc["data"][0]["rh"];
    weatherCode = doc["data"][0]["weather"]["code"];
    pod = doc["data"][0]["pod"].as<String>();
    if (pod == "d")
      isDay = 1;
    else
      isDay = 0;
    break;
  case 3:  //PirateWeather
    if (!doc["currently"]["temperature"]) { noTemp = true; break; }
    tempM = doc["currently"]["temperature"];
    wind_speed = doc["currently"]["windSpeed"];
    wind_dir = convert_windir((doc["currently"]["windBearing"]));
    float_tmp = doc["currently"]["humidity"];
    humiM = float_tmp * 100;
    weatherCode = 0;  //They don't support it
    //const char *xyz = doc["currently"]["icon"];
    break;
  case 4:  //OpenMeteo
    if (!doc["current"]["temperature_2m"]) { noTemp = true; break; }
    tempM = doc["current"]["temperature_2m"];
    wind_speed = doc["current"]["wind_speed_10m"];
    wind_dir = convert_windir((doc["current"]["wind_direction_10m"]));
    humiM = doc["current"]["relative_humidity_2m"];
    weatherCode = doc["current"]["weather_code"];
    isDay = doc["current"]["is_day"];
    break;
  case 5:  //WeatherUnlocked
    if (String(c_vars[EV_METRIC]) == "Y") {
      if (!doc["temp_c"]) { noTemp = true; break; }
      tempM = doc["temp_c"];
      wind_speed = doc["current"]["wind_kph"];
      }
    else {
      if (!doc["temp_f"]) { noTemp = true; break; }
      tempM = doc["temp_f"];
      wind_speed = doc["windspd_mph"];
    }
    humiM = doc["humid_pct"];
    wind_dir = convert_windir((doc["winddir_deg"]));
    weatherCode = doc["wx_code"];
    break;
  case 7:  //Accuweather
    if (!doc["Temperature"]["Imperial"]["Value"]) { noTemp = true; break; }
    tempM = doc["Temperature"]["Imperial"]["Value"];
    humiM = doc["RelativeHumidity"];
    wind_dir = convert_windir((doc["Wind"]["Direction"]["Degrees"]));
    wind_speed = doc["Wind"]["Speed"]["Imperial"]["Value"];
    break;
}

if (noTemp == true) {
  Serial.println("Unable to find temperature key in JSON table");
  return;
}

  switch (atoi(c_vars[EV_WSERVICE])) {
    case 1:
      convert_weathercode_weatherAPI();
      break;
    case 2:
      convert_weathercode_weatherbitIO();
      break;
    case 4:
      convert_weathercode_openmeteo();
      break;
    case 5:
      convert_weathercode_weatherUnlocked();
      break;
  }

    if (isDay == 0) {      
      switch (codeWT) {
        case 1:
          codeWA = 8;    //Clear night
          break;
        case 2:
          codeWA = 10;   //Partly cloudy night
          break;
        case 3:
          codeWA = 11;   //Cloudy night
          break;
        case 8:
          codeWA = 8;    //Clear night
          break;
        case 9:
          codeWA = 9;    //Foggy night
          break;
        case 10:
          codeWA = 11;   //Cloudy night
          break;
      }
    }
  

/*
//Comment out after debugging is done all these Serial.print lines
  Serial.print("Temp:");
  Serial.println(tempM);
  Serial.print("Humidity:");
  Serial.println(humiM);
  Serial.print("Wind_mph:");
  Serial.println(wind_speed);
  Serial.print("Wind_Direction:");
  Serial.println(wind_dir);
  Serial.print("Weathercode:");
  Serial.println(weatherCode);
  Serial.print("isDay:");
  Serial.println(isDay);
  Serial.print("codeWT:");
  Serial.println(codeWT);
  Serial.print("codeWA:");
  Serial.println(codeWA);
*/

}
// End of Get Weather

#include "TinyIcons.h"
#include "WeatherIcons.h"

int xo = 1, yo = 26;
char use_ani = 0;

void convert_weathercode_weatherAPI() {

  codeWT = 0;
  codeWA = 0;
  switch (weatherCode) {
    case 1000:
      codeWT = 1;
      codeWA = 1;
      break;
    case 1003:
      codeWT = 2;
      codeWA = 2;
      break;
    case 1006:
      codeWT = 10;
      codeWA = 10;
      break;
    case 1009:
      codeWT = 3;
      codeWA = 3;
      break;
    case 1030:
      codeWT = 12;
      codeWA = 7;
    case 1063:
      codeWT = 4;
      codeWA = 4;
      break;
    case 1066:
      codeWT = 6;
      codeWA = 6;
      break;
    case 1069:
      codeWT = 14;
      codeWA = 4;
      break;
    case 1072:
      codeWT = 15;
      codeWA = 4;
      break;
    case 1087:
      codeWT = 24;
      codeWA = 5;
      break;
    case 1114:
      codeWT = 6;
      codeWA = 6;
      break;
    case 1117:
      codeWT = 11;
      codeWA = 6;
      break;
    case 1135:
      codeWT = 9;
      codeWA = 9;
      break;
    case 1147:
      codeWT = 21;
      codeWA = 9;
      break;
    case 1150:
      codeWT = 17;
      codeWA = 4;
      break;
    case 1153:
      codeWT = 17;
      codeWA = 4;
      break;
    case 1168:
      codeWT = 22;
      codeWA = 4;
      break;
    case 1171:
      codeWT = 22;
      codeWA = 4;
      break;
    case 1180 ... 1195:
      codeWT = 4;
      codeWA = 4;
      break;
    case 1198 ... 1201:
      codeWT = 15;
      codeWA = 4;
      break;
    case 1204 ... 1207:
      codeWT = 14;
      codeWA = 4;
      break;
    case 1210 ... 1225:
      codeWT = 6;
      codeWA = 6;
      break;
    case 1237:
      codeWT = 18;
      codeWA = 4;
      break;
    case 1240 ... 1243:
      codeWT = 16;
      codeWA = 4;
      break;
    case 1246:
      codeWT = 4;
      codeWA = 4;
      break;
    case 1249 ... 1252:
      codeWT = 14;
      codeWA = 4;
      break;
    case 1255 ... 1258:
      codeWT = 23;
      codeWA = 4;
      break;
    case 1261 ... 1264:
      codeWT = 18;
      codeWA = 4;
      break;
    case 1273 ... 1276:
      codeWT = 5;
      codeWA = 5;
      break;
    case 1279 ... 1282:
      codeWT = 6;
      codeWA = 6; 
      break;
    default:
      Serial.print("Weather code not mapped:");
      Serial.println(weatherCode);
      break;
  }

}

void convert_weathercode_openmeteo() {
  codeWT = 0;
  codeWA = 0;
  switch (weatherCode) {
    case 0:
      codeWT = 1;
      codeWA = 1;
      break;
    case 1:
      codeWT = 1;
      codeWA = 1;
      break;
    case 2:
      codeWT = 2;
      codeWA = 2;
      break;
    case 3:
      codeWT = 3;
      codeWA = 3;
      break;
    case 45 ... 48:
      codeWT = 9;
      codeWA = 9;
      break;
    case 51 ... 55:
      codeWT = 17;
      codeWA = 4;
      break;
    case 56 ... 57:
      codeWT = 22;
      codeWA = 4;
      break;
    case 61 ... 65:
      codeWT = 4;
      codeWA = 4;
      break;
    case 66 ... 67:
      codeWT = 15;
      codeWA = 4;
      break;
    case 71 ... 77:
      codeWT = 6;
      codeWA = 6;
      break;
    case 80 ... 86:
      codeWT = 16;
      codeWA = 4;
      break;
    case 95 ... 99:
      codeWT = 5;
      codeWA = 5;
      break;
    default:
      Serial.print("Weather code not mapped:");
      Serial.println(weatherCode);
      break;
  }

}


void convert_weathercode_weatherbitIO() {

  codeWA = 0;
  codeWT = 0;
      switch (weatherCode) {
        case 200 ... 232:
          codeWT = 5;
          codeWA = 5;
          break;
        case 233:
          codeWT = 18;
          codeWA = 4;
          break;
        case 300 ... 302:
          codeWT = 17;
          codeWA = 4;
          break;
        case 500 ... 502:
          codeWT = 4;
          codeWA = 4;
          break;
        case 511:
          codeWT = 15;
          codeWA = 4;
          break;
        case 520 ... 521:
          codeWT = 16;
          codeWA = 4;
          break;
        case 522:
          codeWT = 4;
          codeWA = 4;
          break;
        case 600 ... 610:
          codeWT = 6;
          codeWA = 6;
          break;
        case 611 ... 612:
          codeWT = 14;
          codeWA = 4;
          break;
        case 621 ... 622:
          codeWT = 6;
          codeWA = 6;
          break;
        case 623:
          codeWT = 13;
          codeWA = 6;
          break;
        case 700:
          codeWT = 12;
          codeWA = 4;
          break;
        case 711:
          codeWT = 19;
          codeWA = 9;
          break;
        case 721:
          codeWT = 7;
          codeWA = 7;
          break;
        case 731:
          codeWT = 20;
          codeWA = 9;
          break;
        case 741:
          codeWT = 9;
          codeWA = 9;
          break;
        case 800 ... 802:
          codeWT = 1;
          codeWA = 1;
          break;
        case 803:
          codeWT = 2;
          codeWA = 2;
          break;
        case 804:
          codeWT = 3;
          codeWA = 3;
          break;
        default:
          Serial.print("Weather code not mapped:");
          Serial.println(weatherCode);
          break;
      }

}

void convert_weathercode_weatherUnlocked() {

  codeWA = 0;
  codeWT = 0;
      switch (weatherCode) {
        case 0:
          codeWT = 1;
          codeWA = 1;
          break;
        case 1:
        case 2:
        case 3:
          codeWT = 2;
          codeWA = 2;
          break;
        case 10:
          codeWT = 12;
          codeWA = 7;
          break;
        case 21:
          codeWT = 4;
          break;
        case 22:
          codeWT = 6;
          break;
        case 23:
          codeWT = 14;
          break;
        case 24:
          codeWT = 15;
          break;
        case 29:
          codeWT = 5;
          break;
        case 38:
          codeWT = 6;
          break;
        case 39:
          codeWT = 11;
          break;
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
          codeWT = 9;
          break;
        case 50 ... 57:
          codeWT = 17;
          break;
        case 60:
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
          codeWT = 4;
          break;
        case 66:
        case 67:
          codeWT = 15;
          break;
        case 68:
        case 69:
          codeWT = 14;
          break;
        case 70:
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
          codeWT = 6;
          break;
        case 79:
          codeWT = 25;
          break;
        case 80:
        case 81:
        case 82:
          codeWT = 4;
          break;
        case 83:
        case 84:
          codeWT = 14;
          break;
        case 85 ... 86:
          codeWT = 6;
          break;
        case 87 ... 88:
          codeWT = 25;
          break;
        case 91 ... 93:
          codeWT = 5;
          break;
        case 94:
          codeWT = 6;
          break;
        default:
          Serial.print("Weather code not mapped:");
          Serial.println(weatherCode);
          break;
      }
}


void draw_wind_humidity() {
  //drawWeather checks to confirm if all the weather was actually displayed first. don't toggle if not true
  if (wind_speed < 0 || wind_dir < 0  || humiM < 0 || !drawWeather)
    return;

  switch (wind_humi) {
  case 0:  //Wind
    wind_humi = 1;
    wind_lstr = String(wind_speed);
    if (wind_speed != 0) {
      switch (wind_lstr.length()) {  
        case 1:
          wind_lstr = String(wind_lstr) + String(" ");
          break;
      }
      TFDrawText(&display, wind_direction[wind_dir], wind_x, wind_y, cc_wht);  
      TFDrawText(&display, wind_lstr, wind_x + 8, wind_y, cc_wind);

    } else {
      wind_lstr = String("CALM");
      TFDrawText(&display, wind_lstr, wind_x, wind_y, cc_wind);
    }
    break;
  case 1:  //Humidity
    wind_humi = 0;
    lcc = cc_red;
    if (humiM < 80)
      lcc = cc_ylw;
    if (humiM < 60)
      lcc = cc_grn;
    if (humiM < 40)
      lcc = cc_blu;
    if (humiM < 20)
      lcc = cc_wht;

    // Pad humi to exactly 4 characters
    humi_lstr = String(humiM) + String(humi_label);
    switch (humi_lstr.length()) {
      case 2:
        humi_lstr = String(humi_lstr) + String("  ");
        break;
      case 3:
        humi_lstr = String(humi_lstr) + String(" ");
        break;
    }
   TFDrawText(&display, humi_lstr, humi_x, humi_y, lcc);  // humidity
   break;
  }
}

//
void draw_weather() {
  drawWeather = true;   
  wind_humi = 0;

  if (tempM == -10000 || humiM == -10000) {
     Serial.println("Weather: No data to display weather");
     return;
  }

    //-temperature
    lcc = cc_red;
    char tmp_Metric;
    if (String(c_vars[EV_METRIC]) == "Y") {
      tmp_Metric = 'C';
      lcc = cc_red;
      if (tempM < 30)
        lcc = cc_org;
      if (tempM < 25)
        lcc = cc_ylw;
      if (tempM < 20)
        lcc = cc_grn;
      if (tempM < 15)
        lcc = cc_blu;
      if (tempM < 10)
        lcc = cc_cyan;
      if (tempM < 1)
        lcc = cc_wht;
    } else {
      tmp_Metric = 'F';
      //F
      if (tempM < 90)
        lcc = cc_grn;
      if (tempM < 75)
        lcc = cc_blu;
      if (tempM < 43)
        lcc = cc_wht;
    }

    String lstr = String(tempM) + String(tmp_Metric);

    //Padding Temp with spaces to keep position the same
    switch (lstr.length()) {
      case 2:
        lstr = String("  ") + String(lstr);
        break;
      case 3:
        lstr = String(" ") + String(lstr);
        break;
    }

    TFDrawText(&display, lstr, tmp_x, tmp_y, lcc);  // draw temperature  

    draw_wind_humidity();

    int cc = color_disp;
    cc = color_disp;

    if (String(c_vars[EV_WANI]) == "N") {
      if (isDay == 1  && codeWT == 1)   
        codeWT = 26;
      TFDrawText(&display, weather_text[codeWT], wtext_x, wtext_y, cc_wtext);
    } else {
      draw_weather_conditions();
    }
}
// End display weather

void draw_date() {
  //date below the clock
  long tnow = now();
  String lstr = "";

  for (int i = 0; i < 5; i += 2) {
    switch (c_vars[EV_DATEFMT][i]) {
      case 'D':
        lstr += (day(tnow) < 10 ? " " + String(day(tnow)) : String(day(tnow)));
        if (i < 4)
          lstr += date_fmt[i + 1];
        break;
      case 'M':  //Replaced leading 0 with space where double quotes are
        lstr += (month(tnow) < 10 ? " " + String(month(tnow)) : String(month(tnow)));
        if (i < 4)
          lstr += date_fmt[i + 1];
        break;
      case 'Y':
        lstr += String(year(tnow));
        if (i < 4)
          lstr += date_fmt[i + 1];
        break;
    }
  }
  //
  String DayofWeek = "  ";
  switch (weekday(tnow)) {
    case 1:
      DayofWeek = " SUN";
      break;
    case 2:
      DayofWeek = " MON";
      break;
    case 3:
      DayofWeek = " TUE";
      break;
    case 4:
      DayofWeek = " WED";
      break;
    case 5:
      DayofWeek = " THR";
      break;
    case 6:
      DayofWeek = " FRI";
      break;
    case 7:
      DayofWeek = " SAT";
      break;
  }

  lstr += String(DayofWeek);

  if (lstr.length())
    TFDrawText(&display, lstr, date_x, date_y, cc_date);
}


void draw_weather_conditions() {
  if (codeWA < 0)
    return;

  xo = img_x;
  yo = img_y;

  switch (codeWA) {
    case 1:  //sunny
      DrawIcon(&display, sunny_ico, xo, yo, 10, 5);
      use_ani = 1;
      break;
    case 2:  //cloudy
      DrawIcon(&display, cloudy_ico, xo, yo, 10, 5);
      use_ani = 1;
      break;
    case 3:  //overcast
      DrawIcon(&display, ovrcst_ico, xo, yo, 10, 5);
      use_ani = 1;
      break;
    case 4:  //rainy
      DrawIcon(&display, rain_ico, xo, yo, 10, 5);
      use_ani = 1;
      break;
    case 5:  //thunders
      DrawIcon(&display, thndr_ico, xo, yo, 10, 5);
      use_ani = 1;
      break;
    case 6:  //snow
      DrawIcon(&display, snow_ico, xo, yo, 10, 5);
      use_ani = 1;
      break;
    case 7:  //mist
      DrawIcon(&display, mist_ico, xo, yo, 10, 5);
      use_ani = 1;
      break;
    case 8:  //clear night
      DrawIcon(&display, moony_ico, xo, yo, 10, 5);
      use_ani = 1;
      break;
    case 9:  //fog night
      DrawIcon(&display, mistn_ico, xo, yo, 10, 5);
      use_ani = 1;
      break;
    case 10:  //partly cloudy night
      DrawIcon(&display, cloudyn_ico, xo, yo, 10, 5);
      use_ani = 1;
      break;
    case 11:  //cloudy night
      DrawIcon(&display, ovrcstn_ico, xo, yo, 10, 5);
      use_ani = 1;
      break;
    Default:
	  TFDrawText (&display, String("     "), xo, yo, 0);
	  use_ani = 1;
      break;
  }
}



void draw_animations(int stp) {
  //weather icon animation
  String lstr = "";
  xo = img_x;
  yo = img_y;
  if (use_ani) {
    int *af = NULL;
	
    switch (codeWA) {
      case 1:  //Sunny
        af = suny_ani[stp % 5];
        break;
      case 2:  //Cloudy
        af = clod_ani[stp % 10];
        break;
      case 3:  //Overcast
        af = ovct_ani[stp % 5];
        break;
      case 4:  //Rainy
        af = rain_ani[stp % 5];
        break;
      case 5:  //Thunder
        af = thun_ani[stp % 5];
        break;
      case 6:  //Snow
        af = snow_ani[stp % 5];
        break;
      case 7:  //Mist
        af = mist_ani[stp % 4];
        break;
      case 8:  //Clear night
        af = mony_ani[stp % 17];
        break;
      case 9:  //Fog night
        af = mistn_ani[stp % 4];
        break;
      case 10:  //Partly Cloudy
        af = clodn_ani[stp % 10];
        break;
      case 11:  //Cloudy night
        af = ovctn_ani[stp % 1];
        break;
    }
    //draw animation
    if (af)
      DrawIcon(&display, af, xo, yo, 10, 5);
  }
}


byte prevhh = 0;
byte prevmm = 0;
byte prevss = 0;
long tnow;



//Restart Clock
void resetclock() {
  Serial.println("************Reset clock called*************");
  display.fillScreen(0);
  TFDrawText(&display, String("  RESTART  "), 10, 9, cc_blu);
  TFDrawText(&display, String("MORPH CLOCKQ"), 10, 16, cc_blu);
  delay(3000);
  ESP.restart();
}


void draw_am_pm() {
  // this sets AM/PM display and is disabled when military time is used
  if (String(c_vars[EV_24H]) == "N") {
    if (hh >= 12)
      TFDrawText(&display, String(" PM"), 42, 19, cc_time);
    else
      TFDrawText(&display, String(" AM"), 42, 19, cc_time);
  }
}

void set_digit_color() {
  digit0.SetColor(cc_time);
  digit1.SetColor(cc_time);
  digit2.SetColor(cc_time);
  digit3.SetColor(cc_time);
  digit4.SetColor(cc_time);

  // Don't print leading zero if not Military
  if (String(c_vars[EV_24H]) == "N" && hh < 10)
    digit5.SetColor(cc_blk);
  else
    digit5.SetColor(cc_time);
}

//
// Main program
//
void loop() {

  ArduinoOTA.handle();

  digit1.DrawColon(cc_time);
  digit3.DrawColon(cc_time);

  static int i = 0;
  static int last = 0;
  static int cm;

  //animations?
  cm = millis();
  if ((cm - last) > ani_speed)  
  {
    last = cm;
    i++;
  }
  

  tnow = now();
  hh = hour(tnow);
  hh24 = hh;        
  mm = minute(tnow);
  ss = second(tnow);
  //

  if (ntpsync) {

    ntpsync = 0;
    //
    prevss = ss;
    prevmm = mm;
    prevhh = hh;

    //we had a sync so draw without morphing

    //clear screen
    display_updater();
    display.fillScreen(0);
 
    //date and weather
    draw_date();
    draw_am_pm();
    draw_weather();
    //

    //military time?
    if (hh > 12 && String(c_vars[EV_24H]) == "N")  
      hh -= 12;
    if (hh == 0 && String(c_vars[EV_24H]) == "N")  //
      hh += 12;

    set_digit_color();

    digit0.Draw(ss % 10);
    digit1.Draw(ss / 10);
    digit2.Draw(mm % 10);
    digit3.Draw(mm / 10);
    digit4.Draw(hh % 10);
    digit5.Draw(hh / 10);
  } else {
    //seconds
    if (ss != prevss) {

      int s0 = ss % 10;
      int s1 = ss / 10;
      set_digit_color;

      

      if (morph_off == 0) {
        if (s0 != digit0.Value()) digit0.Morph(s0);
      } else {
        digit0.SetColor(cc_blk);                         
        digit0.Draw(8);  
        morph_off = 0;
        digit0.SetColor(cc_time);
        digit0.Draw(s0);
      }
      if (s1 != digit1.Value()) digit1.Morph(s1);

      prevss = ss;
      //refresh weather at 31sec in the minute
      if ( ss == 31 && ( ((mm % weather_refresh) == 0) || ForceWeatherCheck) ) {
        getWeather();
        morph_off = 0;  
      } else if ((ss % 10) == 0) {  
          draw_wind_humidity();
      }
    }
    //minutes
    if (mm != prevmm) {
      int m0 = mm % 10;
      int m1 = mm / 10;
      if (m0 != digit2.Value()) digit2.Morph(m0);
      if (m1 != digit3.Value()) digit3.Morph(m1);
      prevmm = mm;
      draw_weather();
    }

    //hours
    if (hh != prevhh) {
      display_updater();
      prevhh = hh;

      draw_date();
      draw_am_pm();

      //military time?
      if (hh > 12 && String(c_vars[EV_24H]) == "N")
        hh -= 12;
      //
      if (hh == 0 && String(c_vars[EV_24H]) == "N")  
        hh += 12;


      int h0 = hh % 10;
      int h1 = hh / 10;
      set_digit_color();

      digit4.Morph(h0);

      if (String(c_vars[EV_24H]) == "N" && hh < 10)  
        digit5.Draw(h1);

      if (h1 != digit5.Value()) digit5.Morph(h1);
    }  //hh changed

    if (String(c_vars[EV_WANI]) == "Y")
      draw_animations(i);


    //set NTP sync interval as needed
    if (NTP.getInterval() < 3600 && year(now()) > 1970) {
      //reset the sync interval if we're already in sync
      NTP.setInterval(3600 * 24);  //re-sync once a day
      Serial.println("24h Sync Enabled");
    }
    //
    //delay (0);
  }
}
