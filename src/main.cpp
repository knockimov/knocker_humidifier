#include <FS.h>
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h> //https://github.com/kentaylor/WiFiManager
#include <Globals.h>
#include <ArduinoJson.h>
#include <Sender.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <SSD1306.h>

bool shouldSaveConfig = false;

char my_token[TKIDSIZE];
char my_name[TKIDSIZE] = "KnockHum000";
char my_server[TKIDSIZE];
char my_url[TKIDSIZE];
char my_db[TKIDSIZE] = "KnockHum";
char my_job[TKIDSIZE] = "KnockHum";
char my_instance[TKIDSIZE] = "000";

char offset[4] = "0";
char maxHumidity[4] = "0";

float Humidity = 0, Temperatur = 0;
String my_ssid;
String my_psk;
uint8_t my_api;
uint8_t my_tempscale = TEMP_CELSIUS;
uint16_t my_port = 80;
uint32_t my_sleeptime = 2 * 60;

unsigned long previousMillisDHT = 0;    // will store last time the DHT was updated
unsigned long previousMillisUpdate = 0; // will store last time the DHT was updated
const long intervalDHT = 2000;          // interval at which to update DHT-sensor
const long intervalUpdate = 120000;     // interval at which to update DHT-sensor

DHT dht(DHTPIN, DHTTYPE, 30);
SSD1306 display(0x3c, 4, 5);

void formatSpiffs()
{
  CONSOLE(F("\nneed to format SPIFFS: "));
  SPIFFS.end();
  SPIFFS.begin();
  CONSOLELN(SPIFFS.format());
}

void validateInput(const char *input, char *output)
{
  String tmp = input;
  tmp.trim();
  tmp.replace(' ', '_');
  tmp.toCharArray(output, tmp.length() + 1);
}

String urlencode(String str)
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++)
  {
    c = str.charAt(i);
    if (c == ' ')
    {
      encodedString += '+';
    }
    else if (isalnum(c))
    {
      encodedString += c;
    }
    else
    {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9)
      {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9)
      {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
      //encodedString+=code2;
    }
    yield();
  }
  return encodedString;
}

String htmlencode(String str)
{
  String encodedstr = "";
  char c;
  uint8_t b;

  for (int i = 0; i < str.length(); i++)
  {
    c = str.charAt(i);

    if (isalnum(c))
    {
      encodedstr += c;
    }
    else
    {
      encodedstr += "&#";
      encodedstr += String((uint8_t)c);
      encodedstr += ';';
    }
  }
  return encodedstr;
}

void saveConfigCallback()
{
  shouldSaveConfig = true;
}

float scaleTemperature(float t)
{
  if (my_tempscale == TEMP_CELSIUS)
    return t;
  else if (my_tempscale == TEMP_FAHRENHEIT)
    return (1.8f * t + 32);
  else if (my_tempscale == TEMP_KELVIN)
    return t + 273.15f;
  else
    return t; // Invalid value for my_tempscale => default to celsius
}

bool startConfiguration()
{
  WiFiManager wifiManager;
  if (WiFi.SSID() != "")
    wifiManager.setConfigPortalTimeout(PORTALTIMEOUT); //If no access point name has been previously entered disable timeout.

  wifiManager.setSaveConfigCallback(saveConfigCallback);

  WiFiManagerParameter api_list(HTTP_API_LIST);
  WiFiManagerParameter custom_api("selAPI", "selAPI", String(my_api).c_str(),
                                  20, TYPE_HIDDEN, WFM_NO_LABEL);

  WiFiManagerParameter custom_name("name", "iSpindel Name", htmlencode(my_name).c_str(),
                                   TKIDSIZE);
  WiFiManagerParameter custom_sleep("sleep", "Update Intervall (s)",
                                    String(my_sleeptime).c_str(), 6, TYPE_NUMBER);
  WiFiManagerParameter custom_offset("offset", "Humidity offset",
                                    String(offset).c_str(), 6, TYPE_NUMBER);
  WiFiManagerParameter custom_maxhumidity("maxhumidity", "Maximum humidity",
                                    String(maxHumidity).c_str(), 6, TYPE_NUMBER);
  WiFiManagerParameter custom_token("token", "Token", htmlencode(my_token).c_str(),
                                    TKIDSIZE);
  WiFiManagerParameter custom_server("server", "Server Address",
                                     my_server, TKIDSIZE);
  WiFiManagerParameter custom_port("port", "Server Port",
                                   String(my_port).c_str(), TKIDSIZE,
                                   TYPE_NUMBER);
  WiFiManagerParameter custom_url("url", "Server URL", my_url, TKIDSIZE);
  WiFiManagerParameter custom_db("db", "InfluxDB db", my_db, TKIDSIZE);
  WiFiManagerParameter custom_job("job", "Prometheus job", my_job, TKIDSIZE);
  WiFiManagerParameter custom_instance("instance", "Prometheus instance", my_instance, TKIDSIZE);
  WiFiManagerParameter tempscale_list(HTTP_TEMPSCALE_LIST);
  WiFiManagerParameter custom_tempscale("tempscale", "tempscale",
                                        String(my_tempscale).c_str(),
                                        5, TYPE_HIDDEN, WFM_NO_LABEL);

  wifiManager.addParameter(&custom_name);
  wifiManager.addParameter(&custom_sleep);
  wifiManager.addParameter(&custom_offset);
  wifiManager.addParameter(&custom_maxhumidity);

  WiFiManagerParameter custom_tempscale_hint("<label for=\"TS\">Unit of temperature</label>");
  wifiManager.addParameter(&custom_tempscale_hint);
  wifiManager.addParameter(&tempscale_list);
  wifiManager.addParameter(&custom_tempscale);
  WiFiManagerParameter custom_api_hint("<hr><label for=\"API\">Service Type</label>");
  wifiManager.addParameter(&custom_api_hint);

  wifiManager.addParameter(&api_list);
  wifiManager.addParameter(&custom_api);

  wifiManager.addParameter(&custom_token);
  wifiManager.addParameter(&custom_server);
  wifiManager.addParameter(&custom_port);
  wifiManager.addParameter(&custom_url);
  wifiManager.addParameter(&custom_db);
  wifiManager.addParameter(&custom_job);
  wifiManager.addParameter(&custom_instance);

  wifiManager.setConfSSID(htmlencode(my_ssid));
  wifiManager.setConfPSK(htmlencode(my_psk));

  Serial.println("Opening configuration portal");

  if (!wifiManager.startConfigPortal("HumidorAP", "partagas"))
  {
    Serial.println("Not connected to WiFi but continuing anyway.");
  }
  else
  {
    Serial.println("connected...yeey :)");
  }

  validateInput(custom_name.getValue(), my_name);
  validateInput(custom_token.getValue(), my_token);
  validateInput(custom_server.getValue(), my_server);
  validateInput(custom_db.getValue(), my_db);
  validateInput(custom_job.getValue(), my_job);
  validateInput(custom_instance.getValue(), my_instance);
  validateInput(custom_url.getValue(), my_url);

  my_sleeptime = String(custom_sleep.getValue()).toInt();
  offset = String(offset.getValue()).toInt();
  custom_maxhumidity = String(custom_maxhumidity.getValue()).toInt();
  my_api = String(custom_api.getValue()).toInt();
  my_port = String(custom_port.getValue()).toInt();
  my_tempscale = String(custom_tempscale.getValue()).toInt();

  // save the custom parameters to FS
  if (shouldSaveConfig)
  {
    // Wifi config
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);

    return saveConfig();
  }
  return false;
}

bool readConfig()
{
  CONSOLE(F("mounting FS..."));

  if (SPIFFS.begin())
  {
    CONSOLELN(F(" mounted!"));
    if (SPIFFS.exists(CFGFILE))
    {
      // file exists, reading and loading
      CONSOLELN(F("reading config file"));
      File configFile = SPIFFS.open(CFGFILE, "r");
      if (configFile)
      {
        CONSOLELN(F("opened config file"));
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());

        if (json.success())
        {
          CONSOLELN(F("\nparsed json"));

          if (json.containsKey("Name"))
            strcpy(my_name, json["Name"]);
          if (json.containsKey("Token"))
            strcpy(my_token, json["Token"]);
          if (json.containsKey("Server"))
            strcpy(my_server, json["Server"]);
          if (json.containsKey("Sleep"))
            my_sleeptime = json["Sleep"];
          if (json.containsKey("Offset"))
            offset = json["Offset"];
          if (json.containsKey("MaxHum"))
            custom_maxhumidity = json["MaxHum"];
          if (json.containsKey("API"))
            my_api = json["API"];
          if (json.containsKey("Port"))
            my_port = json["Port"];
          if (json.containsKey("URL"))
            strcpy(my_url, json["URL"]);
          if (json.containsKey("DB"))
            strcpy(my_db, json["DB"]);
          if (json.containsKey("Job"))
            strcpy(my_job, json["Job"]);
          if (json.containsKey("Instance"))
            strcpy(my_instance, json["Instance"]);
          if (json.containsKey("TS"))
            my_tempscale = json["TS"];
          if (json.containsKey("SSID"))
            my_ssid = (const char *)json["SSID"];
          if (json.containsKey("PSK"))
            my_psk = (const char *)json["PSK"];

          CONSOLELN(F("parsed config:"));
#ifdef DEBUG
          json.printTo(Serial);
#endif
          return true;
        }
        else
        {
          CONSOLELN(F("ERROR: failed to load json config"));
          return false;
        }
      }
      CONSOLELN(F("ERROR: unable to open config file"));
    }
  }
  else
  {
    CONSOLELN(F(" ERROR: failed to mount FS!"));
    return false;
  }
}

bool saveConfig()
{
  CONSOLE(F("saving config..."));

  // if SPIFFS is not usable
  if (!SPIFFS.begin() || !SPIFFS.exists(CFGFILE) ||
      !SPIFFS.open(CFGFILE, "w"))
    formatSpiffs();

  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();

  json["Name"] = my_name;
  json["Token"] = my_token;
  json["Sleep"] = my_sleeptime;
  json["Offset"] = offset;
  json["MaxHum"] = maxHumidity;
  json["Server"] = my_server;
  json["API"] = my_api;
  json["Port"] = my_port;
  json["URL"] = my_url;
  json["DB"] = my_db;
  json["Job"] = my_job;
  json["Instance"] = my_instance;
  json["TS"] = my_tempscale;

  // Store current Wifi credentials
  json["SSID"] = WiFi.SSID();
  json["PSK"] = WiFi.psk();

  File configFile = SPIFFS.open(CFGFILE, "w+");
  if (!configFile)
  {
    CONSOLELN(F("failed to open config file for writing"));
    SPIFFS.end();
    return false;
  }
  else
  {
#ifdef DEBUG
    json.printTo(Serial);
#endif
    json.printTo(configFile);
    configFile.close();
    SPIFFS.end();
    CONSOLELN(F("saved successfully"));
    return true;
  }
}

bool uploadData(uint8_t service)
{
  SenderClass sender;

#ifdef API_UBIDOTS
  if (service == DTUbiDots)
  {
    sender.add("humidity", Humidity);
    sender.add("temperature", scaleTemperature(Temperatur));
    sender.add("interval", my_sleeptime);
    sender.add("RSSI", WiFi.RSSI());
    CONSOLELN(F("\ncalling Ubidots"));
    return sender.sendUbidots(my_token, my_name);
  }
#endif

#ifdef API_INFLUXDB
  if (service == DTInfluxDB)
  {
    sender.add("humidity", Humidity);
    sender.add("temperature", scaleTemperature(Temperatur));
    sender.add("interval", my_sleeptime);
    sender.add("RSSI", WiFi.RSSI());
    CONSOLELN(F("\ncalling InfluxDB"));
    CONSOLELN(String(F("Sending to db: ")) + my_db);
    return sender.sendInfluxDB(my_server, my_port, my_db, my_name);
  }
#endif

#ifdef API_PROMETHEUS
  if (service == DTPrometheus)
  {
    sender.add("humidity", Humidity);
    sender.add("temperature", scaleTemperature(Temperatur));
    sender.add("interval", my_sleeptime);
    sender.add("RSSI", WiFi.RSSI());
    CONSOLELN(F("\ncalling Prometheus Pushgateway"));
    return sender.sendPrometheus(my_server, my_port, my_job, my_instance);
  }
#endif

#ifdef API_GENERIC
  if ((service == DTHTTP) || (service == DTCraftBeerPi) || (service == DTiSPINDELde) || (service == DTTCP))
  {

    sender.add("name", my_name);
    sender.add("ID", ESP.getChipId());
    if (my_token[0] != 0)
      sender.add("token", my_token);
    sender.add("humidity", Humidity);
    sender.add("temperature", scaleTemperature(Temperatur));
    sender.add("interval", my_sleeptime);
    sender.add("RSSI", WiFi.RSSI());

    if (service == DTHTTP)
    {
      CONSOLELN(F("\ncalling HTTP"));
      return sender.sendGenericPost(my_server, my_url, my_port);
    }
    else if (service == DTCraftBeerPi)
    {
      CONSOLELN(F("\ncalling CraftbeerPi"));
      return sender.sendGenericPost(my_server, CBP_ENDPOINT, 5000);
    }
    else if (service == DTiSPINDELde)
    {
      CONSOLELN(F("\ncalling iSPINDELde"));
      return sender.sendTCP("ispindle.de", 9501);
    }
    else if (service == DTTCP)
    {
      CONSOLELN(F("\ncalling TCP"));
      return sender.sendTCP(my_server, my_port);
    }
  }
#endif // DATABASESYSTEM

#ifdef API_FHEM
  if (service == DTFHEM)
  {
    sender.add("humidity", Humidity);
    sender.add("temperature", scaleTemperature(Temperatur));
    sender.add("ID", ESP.getChipId());
    CONSOLELN(F("\ncalling FHEM"));
    return sender.sendFHEM(my_server, my_port, my_name);
  }
#endif // DATABASESYSTEM ==
#ifdef API_TCONTROL
  if (service == DTTcontrol)
  {
    sender.add("T", scaleTemperature(Temperatur));
    sender.add("H", Humidity);
    CONSOLELN(F("\ncalling TCONTROL"));
    return sender.sendTCONTROL(my_server, my_port);
  }
#endif // DATABASESYSTEM ==
}

void setup()
{
  Serial.begin(115200);
  dht.begin();
  pinMode(FANPIN, OUTPUT);

  Serial.println("\n Starting");
  unsigned long startedAt = millis();
  WiFi.printDiag(Serial); //Remove this line if you do not want to see WiFi password printed
  Serial.print("\nFW ");
  Serial.println(FIRMWAREVERSION);

  bool _validConf = readConfig();
  if (!_validConf)
    CONSOLELN(F("\nERROR config corrupted"));

  startConfiguration();

  Serial.print("After waiting ");
  int connRes = WiFi.waitForConnectResult();
  float waited = (millis() - startedAt);
  Serial.print(waited / 1000);
  Serial.print(" secs in setup() connection result is ");
  Serial.println(connRes);
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("failed to connect, finishing setup anyway");
  }
  else
  {
    Serial.print("local ip: ");
    Serial.println(WiFi.localIP());
  }
}

void loop()
{
  unsigned long currentMillisDHT = millis();
  if (currentMillisDHT - previousMillisDHT >= intervalDHT)
  {
    previousMillisDHT = currentMillisDHT;
    double OS = atof(offset);
    Temperatur = dht.readTemperature();
    Humidity = dht.readHumidity() + OS;

    if (isnan(Humidity) || isnan(Temperatur))
    {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    Serial.print("Humidity: ");
    Serial.print(Humidity);
    Serial.print(" %\t");

    Serial.print("Temperature: ");
    Serial.print(Temperatur);
    Serial.print(" *C ");
  }

  unsigned long currentMillisUpdate = millis();
  if (currentMillisUpdate - previousMillisUpdate >= my_sleeptime)
  {
    previousMillisUpdate = currentMillisUpdate;
    uploadData(my_api);     /* UPLOADING THINGS TO SELECTED API */
  }

  double maxHum = atof(maxHumidity);
  if (Humidity > 10 && Humidity < maxHum) {
    digitalWrite(FANPIN, HIGH);
  } else {
    digitalWrite(FANPIN, LOW);
  }

}