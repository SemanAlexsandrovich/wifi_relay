#include <Arduino.h>
#include <ESP8266WiFi.h>

WiFiServer server(80);

enum CMD_e{
  CMD_SCAN = 0,
  CMD_SSID = 1,
  CMD_PSWD = 2,
  CMD_NO_EXIST = 3,
};

enum CONNECT_status_e{
  CONNECT_OK = 0,
  CONNECT_FAILED= 1,
};

// Auxiliar variables to store the current output state
String first_relay_state = "off";
String second_relay_state = "off";
String third_relay_state = "off";
String fourth_relay_state = "off";

// Assign output variables to GPIO pins
const int first_relay = 5;
const int second_relay = 4;
const int third_relay = 0;
const int fourth_relay = 2;

// Variable to store the HTTP request
String header;

void scan_wifi(void){
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.print("Scan start ... ");
  int n = WiFi.scanNetworks();
  Serial.print(n);
  Serial.println(" network(s) found");
  for (int i = 0; i < n; i++)
  {
    Serial.println(WiFi.SSID(i));
  }
  Serial.println();
}

int parse_cmd(char *str){
  const char scan_cmd[] = "scan";
  const char ssid_cmd[] = "ssid:";
  const char pswd_cmd[] = "pswd:";
  int number_of_matches = 0;
  for (int smd_sym_i = 0; smd_sym_i < 4; smd_sym_i++){
    if(*(str + smd_sym_i) == *(scan_cmd + smd_sym_i)) {
      number_of_matches++;
    }
  }
  if (number_of_matches == 4) {
    return CMD_SCAN;
  }
  number_of_matches = 0;
  for (int smd_sym_i = 0; smd_sym_i < 5; smd_sym_i++){
    if(*(str + smd_sym_i) == *(ssid_cmd + smd_sym_i)) {
      number_of_matches++;
    }
  }
  if (number_of_matches == 5) {
    return CMD_SSID;
  }
  number_of_matches = 0;
  for (int smd_sym_i = 0; smd_sym_i < 5; smd_sym_i++){
    if(*(str + smd_sym_i) == *(pswd_cmd + smd_sym_i)) {
      number_of_matches++;
    }
  }
  if (number_of_matches == 5) {
    return CMD_PSWD;
  }

  return CMD_NO_EXIST;
}

int connect_attempt(char * ssid, char * pswd){
 WiFi.begin(ssid, pswd);
  int attempt_num = 0;
 while (WiFi.status() != WL_CONNECTED) {
 delay(1000);
 Serial.print("try to connect:");
 Serial.print(ssid);
 Serial.print(" with  password:");
 Serial.println(pswd);
  attempt_num++;
  if (attempt_num > 10) {
    return CONNECT_FAILED;
  }
 }
  return CONNECT_OK;
}

void connection_wait_loop(void) {
  char ssid[50] = "MGTS_GPON_1617";
  char password[50] = "2GNJ47HT";
  while (1) {
    int connect_status = connect_attempt(ssid, password);
    if (connect_status == CONNECT_OK){
      Serial.println("WiFi connected");
      break;
    } else {
      Serial.println("Connection failed.");
      Serial.println("scan wifi net with command \"scan\"");
      Serial.println("Change ssid with command \"ssid:[wifi name]\"");
      Serial.println("Change pswd with command \"pswd:[new password]\"");
      Serial.println("print any over massage to try again");
      bool exit_fl = false;
      while(1){
        if (exit_fl == true){
          break;
        }
        if (Serial.available() > 0) {
          String str = Serial.readString();
          char str_in_char[50] = "";
          str.toCharArray(str_in_char, 50);
          int cmd = parse_cmd(str_in_char);
          int j = 0;
          switch(cmd){
            case CMD_SSID:
              for(int i = 5; i < (int)(sizeof(str_in_char)/sizeof(char)); i++) {
                ssid[j] = str[i];
                j++;
              }
              Serial.print("new ssid:");
              Serial.println(ssid);
              break;
            case CMD_PSWD:
              for(int i = 5; i < (int)(sizeof(str_in_char)/sizeof(char)); i++) {
                password[j] = str[i];
                j++;
              }
              Serial.print("new pswd:");
              Serial.println(password);
              break;
            case CMD_SCAN: 
              scan_wifi();
              break;
            default:
              exit_fl = true;
              break;
          }
        }
      }
    }
  }
}

void gpio_setup(void){
 pinMode(first_relay, OUTPUT);
 digitalWrite(first_relay, HIGH);
 pinMode(second_relay, OUTPUT);
 digitalWrite(second_relay, HIGH);
 pinMode(third_relay, OUTPUT);
 digitalWrite(third_relay, HIGH);
 pinMode(fourth_relay, OUTPUT);
 digitalWrite(fourth_relay, HIGH);
}


void server_run(void){
  WiFiClient client = server.accept();
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /first_relay/on") >= 0) {
              Serial.println("first relay on");
              first_relay_state = "on";
              digitalWrite(first_relay, LOW);
            } else if (header.indexOf("GET /first_relay/off") >= 0) {
              Serial.println("first relay off");
              first_relay_state = "off";
              digitalWrite(first_relay, HIGH);
            } else if (header.indexOf("GET /second_relay/on") >= 0) {
              Serial.println("second relay on");
              second_relay_state = "on";
              digitalWrite(second_relay, LOW);
            } else if (header.indexOf("GET /second_relay/off") >= 0) {
              Serial.println("second relay off");
              second_relay_state = "off";
              digitalWrite(second_relay, HIGH);
            } else if (header.indexOf("GET /third_relay/on") >= 0) {
              Serial.println("third relay on");
              third_relay_state = "on";
              digitalWrite(third_relay, LOW);
            } else if (header.indexOf("GET /third_relay/off") >= 0) {
              Serial.println("third relay off");
              third_relay_state = "off";
              digitalWrite(third_relay, HIGH);
            } else if (header.indexOf("GET /fourth_relay/on") >= 0) {
              Serial.println("fourth relay on");
              fourth_relay_state = "on";
              digitalWrite(fourth_relay, LOW);
            } else if (header.indexOf("GET /fourth_relay/off") >= 0) {
              Serial.println("fourth relay off");
              fourth_relay_state = "off";
              digitalWrite(fourth_relay, HIGH);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Relay Web Server</h1>");
            client.println("<p>first relay state: " + first_relay_state + "</p>");  
            if (first_relay_state=="off") {
              client.println("<p><a href=\"/first_relay/on\"><button class=\"button button2\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/first_relay/off\"><button class=\"button\">ON</button></a></p>");
            } 
            
            client.println("<p>second relay state: " + second_relay_state + "</p>");    
            if (second_relay_state=="off") {
              client.println("<p><a href=\"/second_relay/on\"><button class=\"button button2\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/second_relay/off\"><button class=\"button\">ON</button></a></p>");
            }

            client.println("<p>third relay state: " + third_relay_state + "</p>");    
            if (third_relay_state=="off") {
              client.println("<p><a href=\"/third_relay/on\"><button class=\"button button2\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/third_relay/off\"><button class=\"button\">ON</button></a></p>");
            }

            client.println("<p>fourth relay state: " + fourth_relay_state + "</p>");    
            if (fourth_relay_state=="off") {
              client.println("<p><a href=\"/fourth_relay/on\"><button class=\"button button2\">OFF</button></a></p>");   
            } else {
              client.println("<p><a href=\"/fourth_relay/off\"><button class=\"button\">ON</button></a></p>");
            }

            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    client.flush();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}


void setup() {
  Serial.begin(115200);
  delay(10);
  gpio_setup();
  connection_wait_loop();

  server.begin();
  Serial.print("to device control join to: ");
  Serial.println(WiFi.localIP());
}

void loop(){
  server_run();
}
