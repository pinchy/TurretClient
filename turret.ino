/*
  Turret Controller
  @author: Sebastien Eckersley-Maslin
  @date: Jan 2016
  
  Turret controller script. Connects to the turret webserver and downloads 
  any pending move/aim commeands.  If it finds one, it controls the turret
  to move to the correct location and to execute the command.
  
  Requires the Turret library
  
 */
#include <SPI.h>
#include <Ethernet.h>
#include <Turret.h>

#define NUM_RESPONSES_BEFORE_CUTOFF 10  // max 256

Turret turret;

byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01 };
IPAddress ip(10, 1, 1, 99);
//IPAddress server(10, 1, 1, 14);  // who we're connecting to.  TODO UPDATE
char server[] = "turret.bluechilli.com";

EthernetClient client;
unsigned short invalidResponseCount = 0;
const unsigned long requestInterval = 5000;
unsigned long lastAttemptTime = 0;            // last time you connected to the server, in milliseconds

String resp;

void setup()
{
  resp.reserve(256);
  Serial.begin(9600);
  while (!Serial)  { ; }

  // attempt a DHCP connection:
  Serial.println("Attempting to get an IP address using DHCP:");
  if (!Ethernet.begin(mac))
  {
    Serial.println("failed to get an IP address using DHCP, trying manually");
    Ethernet.begin(mac, ip);
  }

  Serial.print("My address:");
  Serial.println(Ethernet.localIP());
  connectToServer();
}


// Main program loop
// Check if client is connected, get the response and parse the response
// if it's not connected, check how long for and reconnect if over request interval
void loop()
{
  if (client.connected())
  {
    resp = getResponse();
    parseResponse();

    if (invalidResponseCount == NUM_RESPONSES_BEFORE_CUTOFF)
    {
      turret.motors(OFF);
    }
  }
  else if (millis() - lastAttemptTime > requestInterval)
  {
    Serial.println(turret.toString());
    connectToServer();
  }
}


void connectToServer()
{
  Serial.println("connecting to server...");
  if (client.connect(server, 80))
  {
    Serial.println("making HTTP request...");
    client.println("GET /turret/check HTTP/1.0");
    client.println("HOST: turret.bluechilli.com");
    client.println();
  }

  lastAttemptTime = millis();
}


// Get the response from the server
// Response we want is the first (and only) all numeric line as
// this represents the flat encoded numeric data
String getResponse()
{
  String currentLine;
  currentLine.reserve(256);

  while (client.available())
  {
    char inChar = client.read();
    currentLine += inChar;

    if (inChar == '\n')
    {
      if (currentLine.toInt())
      {
        Serial.println(currentLine);
        client.stop();
        return currentLine;
      }
      currentLine = "";
    }
  }
  client.stop();
}

/* Parse the response from the server
[position] length description
[0]      1 fire or hold
[1-5]    5 azimuth, zero padded left
[6-10]   5 elevation, zero padded left
[11]     1 use door code
[12-14]  3 door code, zero padded left

|fire (1) or nothing (0)
||azimuth
||    |elevation
||    |    |use door code
||    |    ||door code
||    |    ||
012345123450123
*/
bool parseResponse()
{
  if (!resp.toInt() || resp.toInt() == 0)
  {
    Serial.println("No valid command pending");
    resp = "";

    if (invalidResponseCount < NUM_RESPONSES_BEFORE_CUTOFF)
      invalidResponseCount ++;

    return false;
  }

  String a = resp.substring(1, 6);
  String e = resp.substring(6, 11);
  String c = resp.substring(12, 15);
  bool fire = (resp.charAt(0) == '1');
  bool door = (resp.charAt(11) == '1');

  if (fire)
  {
    Serial.println("Fire");
    turret.aimAndFire(a.toInt(), e.toInt());
    splash();
  }
  else
  {
    Serial.println("Aim only");
    turret.aim(a.toInt(), e.toInt());
  }

  Serial.print("Target A: ");
  Serial.print(turret.target.azimuth);

  Serial.print(" E: ");
  Serial.println(turret.target.elevation);

  resp = "";
  return true;
}

void splash(void)
{
    Serial.println("connecting to server...");
    if (client.connect(server, 80))
    {
      Serial.println("making HTTP request to Splash...");
      client.println("GET /turret/splash HTTP/1.0");
      client.println("HOST: turret.bluechilli.com");
      client.println();
      client.stop();
    }

    lastAttemptTime = millis();
}
