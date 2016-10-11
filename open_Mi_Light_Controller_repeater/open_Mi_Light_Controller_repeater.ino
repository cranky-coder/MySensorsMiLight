#define MY_DEBUG 
#define MY_RADIO_NRF24
#define MY_NODE_ID 100
#define mi_command_repeat 30   //# of times to resend the command to bulb

#define MY_REPEATER_FEATURE
#include <MyConfig.h>
#include <MySensors.h>

#include <SPI.h>
#define CE_PIN 9     //CE and CSN pin for nrf24 radio
#define CSN_PIN 10
#include <nRF_24L01.h>
#include <avr/power.h>
#include <printf.h>

#include "PL1167_nRF24.h"
#include "MiLightRadio.h"






RF_24 mi_radio(CE_PIN, CSN_PIN);
PL1167_nRF24 prf(mi_radio);
MiLightRadio mlr(prf);
                                       // B0    F2    EA    6D    B0    02    f0
static uint8_t outgoingPacket_on[7] = { 0xB0, 0xF2, 0xEA, 0x6D, 0x91, 0x03, 0x00};  //the first three #s are remote ID, replace with yours
static uint8_t outgoingPacket_off[7] = { 0xB0, 0xF2, 0xEA, 0x04, 0x91, 0x04, 0x00};
uint8_t outgoingPacket[7];

#define sizeofincomingCommand 15 //command comes in text form of 7 uint8_t, with an additional NULL at end

char *incomingCommand;    //array for incoming command

 
boolean mi_radio_state = false;
void presentation()  
{  
  //Send the sensor node sketch version information to the gateway
  sendSketchInfo("Milight controller", "1.0");
  delay(500);
}

void setup()
{  
  
   //clock_prescale_set(clock_div_2);     //for a barebone atmega328p, this  will make the CPU running at 4MHz.
  //Serial.begin(115200);
  // The third argument enables repeater mode.

  //Send the sensor node sketch version information to the gateway
  present(0, S_LIGHT);
  delay(500);
  present(1, S_CUSTOM);

delay(500);

}




void receive(const MyMessage &message) {
  // The command will be transmitted in V_VAR1 type, in the following 7 byte format: ID1 ID2 ID3 COLOR BRIGHTNESS COMMAND.
  // see https://hackaday.io/project/5888-reverse-engineering-the-milight-on-air-protocol
  Serial.println(message.type);
  if (message.type == V_STATUS)
  {
    //Serial.println("triggered V_STATUS");
    mlr.begin();

    // Change light status
    Serial.println(message.getBool());
    if ( message.getBool())
    {
      Serial.println("here");
      // Serial.println("triggered ON");
      sendCommand (outgoingPacket_on, sizeof(outgoingPacket_on));
    }


    else {
      //  Serial.println("triggered off");
      sendCommand(outgoingPacket_off, sizeof(outgoingPacket_off));
    }
    _begin();
  }


  if (message.type == V_VAR1)
  {
    Serial.println("triggered V_VAR1");
    
    incomingCommand = (char *)message.getCustom();
    Serial.println((char *)message.getCustom());
     mlr.begin();
     Serial.println("#############");
    for (int i=0; i<sizeof(outgoingPacket); i++)
    { outgoingPacket[i] = 16*char_to_uint8_t(incomingCommand[i*2]) + char_to_uint8_t(incomingCommand[i*2+1]);
      Serial.println(outgoingPacket[i]);
      }
    Serial.println("#############");
    sendCommand(outgoingPacket, sizeof(outgoingPacket));
    _begin();
  }

//gw.begin(incomingMessage, NODE_ID, true);




}

void sendCommand(uint8_t mi_Command[7], int sizeofcommand)
{ for (int i = 0; i < mi_command_repeat; i++)
  { mlr.write(mi_Command, sizeofcommand);
  }
}

uint8_t char_to_uint8_t(char c) 
{
	uint8_t i;
	if (c <= '9')
		i = c - '0';
	else if (c >= 'a')
		i = c - 'a' + 10;
	else
		i = c - 'A' + 10;
	return i;
}

