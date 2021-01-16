#include <Arduino.h>

// Import required libraries
#include "mywifi.h"

const char* ssid = "myLoad";
const char* password =  "0912345678";

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";
const char* PARAM_INPUT_SSID = "SSID";
const char* PARAM_INPUT_Password = "Password";

uint8_t wifi_ssid_count=0;
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


String outputState(int output);
/**
  * @brief	WebServer Root Page	
  */
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>WiFi Config</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;
              text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}
    .button2 {background-color: #555555;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>YZTEK Wi-Fi Configuration</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/led?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/led?output="+element.id+"&state=0", true); }
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";
/**
	* @brief	scan SSID and Print out
	*/
void myWIFI_scan_ssid(void)
{
  //WiFi.mode(WIFI_STA);
  //WiFi.disconnect();
  delay(100);
  wifi_ssid_count = WiFi.scanNetworks();
  Serial.println("scan done");
  if (wifi_ssid_count == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(wifi_ssid_count);
    Serial.println(" networks found");

    for (int i = 0; i < wifi_ssid_count; ++i)
     {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i)));
      delay(10);
     }
  }
  Serial.println(""); 
}
/**
	* @brief	Connect to Wi-Fi and return IP
    * @retval	IPAddress 
	*/
IPAddress  myWIFI_connect(void)
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP Local IP Address
  IPAddress ip=WiFi.localIP();
  Serial.println(ip);
  return ip;
}
/**
    @brief	Replaces placeholder with  web page
*/
String root_page_placeholder(const String& var)
{
  if(var == "BUTTONPLACEHOLDER")
  {
    String hander = "";    
    hander +="<form action=\"/wificonf\">";
    hander +="<h4>SSID: <select name=\"SSID\" ></h4>";
    for(int i=0;i<wifi_ssid_count;i++)
    {
      hander +="<option >"+WiFi.SSID(i)+"</option>";
    }
    hander +="</select>";
    hander +="<h4>Password: <input type=\"text\"   name=\"Password\"></h4>";
    hander +="<input type=\"submit\" value=\"Submit\">";
    hander +="</form>";
    hander += "<h4>Output - GPIO 2</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\" " + outputState(2) + "><span class=\"slider\"></span></label>";
    hander += "<h4>Output - GPIO 4</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"4\" " + outputState(4) + "><span class=\"slider\"></span></label>";
    hander += "<h4>Output - GPIO 33</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"33\" " + outputState(33) + "><span class=\"slider\"></span></label>";
    return hander;
  }
  return String();
}

String outputState(int output)
{
  if(digitalRead(output))
  {
    return "checked";
  }
  else 
  {
    return "";
  }
}
/**
    @brief	Create asynchronous web server 
            and Setting mDNS : autoff.local
	@note   Page :  / (root)
            Page :  /led    -> control LED form page
            Page :  /get    ->
*/


void myWIFI_Webserver(void)
{
// Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/html", index_html, root_page_placeholder);
  });

  // Send a GET request to <ESP_IP>/led?output=<inputMessage1>&state=<inputMessage2>
  server.on("/led", HTTP_GET, [] (AsyncWebServerRequest *request) 
  {
    String inputMessage1;
    String inputMessage2;

    // GET input1 value on <ESP_IP>/led?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) 
    {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();

      digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());

    }
    else 
    {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";

    }
    Serial.print("GPIO: ");
    Serial.print(inputMessage1);
    Serial.print(" - Set to: ");
    Serial.println(inputMessage2);
    request->send(200, "text/plain", "OK");
  }
  );

  // Send a GET request to <ESP_IP>/wificonf?input1=<inputMessage>
  server.on("/wificonf", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String input_SSID;
    String input_Password;
    if (request->hasParam(PARAM_INPUT_SSID) && request->hasParam(PARAM_INPUT_Password)) 
    {
      input_SSID = request->getParam(PARAM_INPUT_SSID)->value();
      input_Password = request->getParam(PARAM_INPUT_Password)->value();
    }
    else 
    {
      input_SSID = "none";
      input_Password = "none";
    }
    Serial.println("SSID=:"+input_SSID);
    Serial.println("Password=:"+input_Password);

    request->send(200, "text/html", "<P>Wi-Fi Setting successful</P> <P>SSID:  "+ 
                                      input_SSID+"</P><P>Password :"+input_Password+"</P><a href=\"/\">Return to Home Page</a>");
  });

  if(!MDNS.begin("Autoff"))
  {
    Serial.println("Error Starting mDNS");
  } 
  // Start server
  server.begin();


}