#include <Arduino.h>
#include <Crescer.h>
#include <WiFi.h>

//timer withou delay (crescer automacao)
Tempora tempCom;

IPAddress ip (192, 168, 10, 50); //esp32 ip
IPAddress ip1(192, 168, 10, 1); //gateway padrao
IPAddress ip2(255, 255, 255, 0); //subnetwork mask

const char* ssid = "RVT_UHTRED";
const char* password = "0390sharpe";

#define rele_pin 4

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  //run timer with 5 seconds
  tempCom.defiSP(5000);

  // prepare GPIO2
  pinMode(rele_pin, OUTPUT);
  digitalWrite(rele_pin, 0);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  WiFi.config(ip, ip1, ip2);

  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
    while(!client.available()){
    delay(1);
    if (tempCom.Saida(1))
    {
      Serial.println("Tempo excedeu");
      client.flush();
       String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nNAO COMUNICOU !!! ";
       s += "</html>\n";
      // Send the response to the client
      client.print(s);
      delay(1);
      Serial.println("Client desconectado depois da nao comunicacao");
      tempCom.Saida(0);
      return;
    }
  }
tempCom.Saida(0);


  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  // Match the request
  int val;
  if (req.indexOf("/gpio/0") != -1) {
    val = 0;
  } else if (req.indexOf("/gpio/1") != -1) {
    val = 1;
  } else {
    Serial.println("invalid request");
    client.stop();
    return;
  }

  // Set GPIO2 according to the request
  digitalWrite(rele_pin, val);

  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now ";
  s += (val) ? "high" : "low";
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}