#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define led D3

ESP8266WebServer server(80);
String randomNumber;
String WifiName = "message";

void setup() {
  Serial.begin(115200);

  randomNumber = String(random(1000, 9999));

  pinMode(led, OUTPUT);

  startAPMode();

  setupServer();
  server.begin();
  Serial.println("HTTP Server started");
}

void loop() {
  server.handleClient();
}

void startAPMode() {
  String ssid = WifiName + "-" + randomNumber;
  String password = randomNumber + "message";

  WiFi.softAP(ssid.c_str(), password.c_str());

  Serial.println("ESP8266 in Access Point Mode");
  Serial.print("AP SSID: ");
  Serial.println(ssid);
  Serial.print("AP Password: ");
  Serial.println(password);
}

void setupServer() {
  server.on("/ap-list", HTTP_GET, []() {
    String apList = scanNetworks();
    server.send(200, "text/plain", apList);
  });

  server.on("/connect", HTTP_GET, []() {
    String ssid = server.arg("SSID");
    String password = server.arg("PW");

    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);

    if (ssid == "" || password == "") {
      server.send(400, "text/plain", "Missing SSID or Password");
      return;
    }

    WiFi.begin(ssid.c_str(), password.c_str());
    int maxAttempts = 20;
    while (WiFi.status() != WL_CONNECTED && maxAttempts-- > 0) {
      delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("AP mode disabled");
      server.send(200, "text/plain", WiFi.localIP().toString());

    } else {
      Serial.println("Connection Failed, AP mode still active");
      server.send(200, "text/plain", "Connection Failed");
    }
  });

  server.on("/Texting", HTTP_GET, []() {
    String msg = server.arg("Msg");

    if (msg == "") {
      server.send(400, "text/plain", "Missing Msg parameter");
      return;
    }

    Serial.println("Received message: " + msg);

    if (msg == "Info") {
      Serial.println("Local IP : " + WiFi.localIP().toString());
    } else if (msg == "Test") {
      Serial.println("Test!");
    } else if (msg == "LedOn") {
      digitalWrite(led, LOW);
      Serial.println("Led On!");
    } else if (msg == "LedOff") {
      digitalWrite(led, HIGH);
      Serial.println("Led Off!");
    } else {
      Serial.println("Unknown message");
    }

    server.send(200, "text/plain", "Message received: " + msg);
  });

  server.on("/APdisconnect", HTTP_GET, []() {
      server.send(200, "text/plain", "ok");
       WiFi.softAPdisconnect(true);
  });
}

String scanNetworks() {
  int n = WiFi.scanNetworks();
  String result = "";

  for (int i = 0; i < n; ++i) {
    result += WiFi.SSID(i);
    if (i < n - 1) result += ",";
  }

  return result;
}

