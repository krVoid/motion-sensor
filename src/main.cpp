#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <sstream>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>

ESP8266WebServer server(80);
int czujnik = D0;   //pin 8 połączony z sygnałem z czujnika
 
 template <typename T>
std::string NumberToString ( T Number )
{
   std::ostringstream ss;
   ss << Number;
   return ss.str();
}

void sendSensorValue(float (*dataGetter)(void))
{
	float val = dataGetter();
	server.send(200, "text/plain", NumberToString(val).c_str());
}
void handleNotFound()
{
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for (uint8_t i = 0; i < server.args(); i++)
	{
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}

	server.send(404, "text/plain", message);
}

float getLightLevel() {
	return digitalRead(D0);
}


struct SensorConfig {
	std::string name;
	std::string description;
	
	float (*dataGetter)(void);
};

class ApiConfigurator {
	ESP8266WebServer *server;
	std::string sensorsCfgJSON;

	public:

	ApiConfigurator(ESP8266WebServer *srv) {
		server = srv;
	}

	void AddSensors(std::vector<SensorConfig> sCfg) {
		int i = 0;
		sensorsCfgJSON = "'inputs': [";
		for(const auto &cfg : sCfg){
			sensorsCfgJSON += "{'name':'" + cfg.name + "', 'description':'"+cfg.description+ "','inputId':'"+NumberToString(i) + "'},";
			std::string url = std::string("/sensor/") + NumberToString(i);
			server->on(url.c_str(), [cfg]() { sendSensorValue(cfg.dataGetter); });
			i++;
		}
		sensorsCfgJSON += "]";
	}

	void Build() {
		std::string registerString = "{ 'outputs':[]," + sensorsCfgJSON + "}";
		server->on("/register", [this, registerString]() { 	this->server->send(200, "text/plain", registerString.c_str());
});
		server->onNotFound(handleNotFound);
		server->begin();
	}
};


ApiConfigurator api(&server);

void setup()
{
	delay(5000);
	Serial.begin(9600);	
 pinMode(czujnik, INPUT); 
	WiFiManager wifiManager;
	wifiManager.autoConnect("NodeMCU-Arduino-PlatformIO");
	Serial.println("Connected!");
	if (MDNS.begin("esp8266"))
	{
		Serial.println("MDNS responder started");
	}

	api.AddSensors({{"Czujnik swiatla", "To jest czujnik swiatla o zakresie x-y.", getLightLevel}});
	api.Build();
	

	Serial.println("HTTP server started");
	Serial.println("local ip");
	Serial.println(WiFi.localIP());
	Serial.println(WiFi.gatewayIP());
	Serial.println(WiFi.subnetMask());
	Serial.println("Idle...");
}

void loop()
{
	server.handleClient();
}

// void setup(){
//   Serial.begin(9600);        //inicjalizacja monitora szeregowego
//   pinMode(czujnik, INPUT);   //ustawienie pinu Arduino jako wejście
  
//   Serial.println("---- TEST CZUJNIKA RUCHU ----"); 
// }
 
// void loop(){
//   int ruch = digitalRead(czujnik);      //odczytanie wartości z czujnika
//   if(ruch == HIGH)                      //wyświetlenie informacji na monitorze szeregowym
//   {                                     //stan wysoki oznacza wykrycie ruchu, stan niski - brak ruchu
//     Serial.println("RUCH WYKRYTY!");
//   }  else {
//         Serial.println("Kailek sie nie rusza");

//   }
//   delay(100);
// }