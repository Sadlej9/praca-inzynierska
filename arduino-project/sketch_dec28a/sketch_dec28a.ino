//program z wyswietlaczem 16x2, dzialaja przyciuski, wcisniecie przycisku to wyswietlenie parametrow, czas, dodanie ethernetu
#include "DHT.h" // dht
#include <DS3231.h> // rtc
#include <LiquidCrystal_I2C.h> // lcd
#include <DallasTemperature.h> // sensors
#include <Ethernet.h> // ethernet server

#define ONE_WIRE_BUS 7
#define PRINT_ON_SERIAL_BUS true
#define DHTPIN 2  // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22  // DHT 22  (AM2302), AM2321


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
DS3231 rtc(SDA, SCL);
DHT dht(DHTPIN, DHTTYPE);

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {0x90, 0xA2, 0xDA, 0x00, 0x23, 0x36}; //MAC address found on the back of your ethernet shield.
IPAddress ip(192, 168, 1, 107);  // IP address dependent upon your network addresses.

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
byte mode = 1;




void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.println("Test projektu inzynierskiego");
  dht.begin();
  rtc.begin();

  rtc.setDOW(WEDNESDAY);     // ustawienie dnia tygodnia
  rtc.setTime(21, 33, 0);     // ustawienie godziny, minuty, sekundy
  rtc.setDate(25, 1, 2023); 

  pinMode(8, INPUT_PULLUP);   //Przycisk do wyswietlenia temp
  pinMode(9, INPUT_PULLUP);   //Przycisk do wyswietlenia wilgotnosci
  pinMode(10, INPUT_PULLUP);  //Przycisk do pokazania czasu

  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pomiar temp. i ");
  lcd.setCursor(0, 1);
  lcd.print("wilgotnosci");

  // start the Ethernet connection and the server:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  server.begin();
  delay(500);
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Adres serwera:");
  lcd.setCursor(0, 1);
  lcd.print(Ethernet.localIP());
  delay(5000);

  // Start up the library
  sensors.begin();
}




void loop() {
  sensors.requestTemperatures();  // Send the command to get temperatures

  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  float t1 = sensors.getTempCByIndex(0);
  if (PRINT_ON_SERIAL_BUS) {
    Serial.print(rtc.getTimeStr(2));
    Serial.print(" - ");
    Serial.println(rtc.getDateStr());
    Serial.println(rtc.getDOWStr(1));

    if (t1 != DEVICE_DISCONNECTED_C)  // if ((sky == blue) && (grass == green)) funkcja AND
    {
      Serial.println("Temperatura czujnika DS18B20 wynosi: ");
      Serial.print(t1);
      Serial.println("°C ");
    } else {
      Serial.println("Error: Could not read temperature data");
    }
  }


  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h2 = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t2 = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (PRINT_ON_SERIAL_BUS && (isnan(h2) || isnan(t2))) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  if (PRINT_ON_SERIAL_BUS) {
    Serial.println("Parametry czujnika DHT22: ");
    Serial.print(F("Wilgotnosc: "));
    Serial.print(h2);
    Serial.print(F("%  Temperatura: "));
    Serial.print(t2);
    Serial.println(F("°C "));
  }

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // If you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // Send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Access-Control-Allow-Origin: *");
          client.println("Content-Type: application/json");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          // Send the JSON data
          client.println("{");
          client.print("  \"t1\": \"");
          client.print(t1);
          client.print("\",\n");
          client.print("  \"t2\": \"");
          client.print(t2);
          client.print("\",\n");
          client.print("  \"h\": \"");
          client.print(h2);
          client.print("\",\n");
          client.print("  \"timestamp\": \"");
          client.print(rtc.getDateStr());
          client.print(" ");
          client.print(rtc.getTimeStr(2));
          client.print("\"\n");          
          // client.println("  \"t2\": " + String(t2) + ",");
          // client.println("  \"h\": " + String(h2) + ",");
          // client.println("  \"timestamp\": \"" + String(millis()) + "\"");
          client.println("}");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }

  if (digitalRead(8) == LOW) {
    lcd.clear();
    mode = 1;
  }
  if (digitalRead(9) == LOW) {
    lcd.clear();
    mode = 2;
  }
  if (digitalRead(10) == LOW) {
    lcd.clear();
    mode = 3;
  }

  if (mode == 1) {
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Temp1 C: ");
    lcd.print(t1);
    lcd.setCursor(0, 1);
    lcd.print("Temp2 C: ");
    lcd.print(t2);  //tutaj wyswietlmy temperature
  } else {          //Jeśli warunek nie został spełniony (przycisk nie jest wciśnięty)
    delay(10);      // nic nie rob
  }

  if (mode == 2) {
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Wilgotnosc % : ");
    lcd.setCursor(0, 1);
    lcd.print(h2);
  } else {      //Jeśli warunek nie został spełniony (przycisk nie jest wciśnięty)
    delay(10);  // nic nie rob
  }

  delay(100);
  if (mode == 3) {
    lcd.setCursor(0, 0);
    lcd.print(rtc.getTimeStr(2));
    lcd.setCursor(0, 1);
    lcd.print(rtc.getDateStr());
    lcd.print(" ");
    lcd.print(rtc.getDOWStr(1));
  } else {      //Jeśli warunek nie został spełniony (przycisk nie jest wciśnięty)
    delay(10);  // nic nie rob
  }
}
