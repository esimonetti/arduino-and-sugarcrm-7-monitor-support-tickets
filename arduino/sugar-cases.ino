#include <EtherCard.h>

int output_pin = 7;
int output_time_on = 500;
static byte mac_address[] = {0x42,0x42,0x42,0x42,0x42,0x42};
const char url_crm_proxy[] PROGMEM = "your-sugarcrm-proxy-url.com";
const char uri_crm_proxy[] PROGMEM = "/";

byte Ethernet::buffer[700];
static byte current_session;
static uint32_t web_timer;
static uint32_t output_timer;
int previous_response_code = 0;

void activateOutput(int milliseconds_on)
{
  if(milliseconds_on > 0)
  {
    // turning on
    output_timer = millis() + milliseconds_on;
    digitalWrite(output_pin, HIGH);
  }
  else
  {
    // turning off, if the timer has passed
    if(millis() > output_timer)
    {
      digitalWrite(output_pin, LOW);
    }
  }
}

void updateOutput(int code)
{
  // turn all LED off (2 to 6)
  for(int i = 2; i <=6; i++) digitalWrite(i, LOW);
    
  // now handle the new code
  if(code > 0)
  {
    if(code > 5) code = 5;
    
    // now incremente of one, because LED pins are 2, 3, 4, 5, 6
    code++;
    
    // turn on LED based on code
    for(int i = 2; i <= code; i++) digitalWrite(i, HIGH);
  }
}

void submitCRMRequest()
{
  Serial.print(F("Contacting CRM Proxy ("));
  Serial.print(millis());
  Serial.println(F(" milliseconds since boot)"));
  
  Stash::prepare(PSTR("GET $F HTTP/1.0" "\r\n" "Host: $F" "\r\n" "Connection: close" "\r\n" "\r\n"), uri_crm_proxy, url_crm_proxy);
  current_session = ether.tcpSend();
  Stash::cleanup();
}

void parseCRMResponse(String response)
{
  // response looks like {"open_cases":"XXX"}, parsing
  int jsonOpeningBracket = response.indexOf('{"open_cases":"');
  int jsonClosingBracket = response.indexOf('"}');
  String parsedResult = response.substring(jsonOpeningBracket + 14, jsonClosingBracket - 1);
  
  int new_response = parsedResult.toInt();
  if(previous_response_code < new_response)
  {
    activateOutput(output_time_on);
  }
  
  Serial.print(F("Open Support Cases: "));
  Serial.println(new_response);
  updateOutput(new_response);
  
  // store new value
  previous_response_code = new_response;
}

int freeRam()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void setup ()
{
  // from pin 2 to 7 are outputs
  for(int i = 2; i <=7; i++)
  {
    pinMode(i, OUTPUT);
  }
  
  Serial.begin(57600);
  
  while(ether.begin(sizeof Ethernet::buffer, mac_address) == 0) 
  {
    Serial.println(F("Failed to access Ethernet controller"));
    delay(5000);
  }
  
  ether.dhcpSetup();
  
  ether.printIp("Local IP: ", ether.myip);
  ether.printIp("Local Gateway: ", ether.gwip);
  ether.printIp("Local DNS: ", ether.dnsip);
  
  ether.dnsLookup(url_crm_proxy);
}

void loop ()
{ 
  ether.packetLoop(ether.packetReceive());
  
  const char* reply = ether.tcpReply(current_session);
  if (reply != 0)
  {
    Serial.println(F("Received response from CRM Proxy"));
    parseCRMResponse((String)reply);
  }
  
  // turn off output if required
  activateOutput(0);
  
  // execute crm request if required
  if(millis() > web_timer)
  {
    Serial.print(F("Free Ram: "));
    Serial.println(freeRam());
    
    web_timer = millis() + 15000;
    submitCRMRequest();
  }
}
