/*
  xdrv_200_rcswitch.ino - RF DiO Remote transceiver using DiORemote library for Tasmota

  Copyright (C) 2022    Valentin Saugnier

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifdef USE_DIO_REMOTE
/*********************************************************************************************\
 * RF send and receive using DiOremote library https://github.com/guyou/DiOremote
\*********************************************************************************************/

#define XDRV_200             200

#define D_CMND_RFSEND "Send"
#define D_JSON_RF_DATA "Data"

const char kRfDioCommands[] PROGMEM = "RfDio|" // No prefix
        D_CMND_RFSEND;

void (* const RfDioCommands[])(void) PROGMEM = {&CmndRfDioSend};

#include <DiOremote.h>

DiOremote *myRemote;

void RfDioInit(void) {
  if (PinUsed(GPIO_RFDIOSEND)) {
    myRemote = new DiOremote(Pin(GPIO_RFDIOSEND));
  }
}

/*********************************************************************************************\
 * Commands
\*********************************************************************************************/

void CmndRfDioSend(void)
{
  if (!PinUsed(GPIO_RFDIOSEND)) { return; }

  bool error = false;

  if (XdrvMailbox.data_len) {
    unsigned long long data = 0;	// unsigned long long  => support payload >32bit

    JsonParser parser(XdrvMailbox.data);
    JsonParserObject root = parser.getRootObject();
    if (root) {
      // RFsend {"data":1234}
      char parm_uc[10];
      data = root.getULong(PSTR(D_JSON_RF_DATA), data);	// read payload data even >32bit
    } else {
      //  RFsend data
      char *p;
      uint8_t i = 0;
      for (char *str = strtok_r(XdrvMailbox.data, ", ", &p); str && i < 1; str = strtok_r(nullptr, ", ", &p)) {
        switch (i++) {
        case 0:
          data = strtoul(str, nullptr, 0);  // Allow decimal (5246996) and hexadecimal (0x501014) input
          break;
        }
      }
    }

    if (data) {
      myRemote->send(data);
      ResponseCmndDone();
    } else {
      error = true;
    }
  } else {
    error = true;
  }
  if (error) {
    Response_P(PSTR("{\"" D_CMND_RFSEND "\":\"" D_JSON_NO " " D_JSON_RF_DATA "\"}"));
  }
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xdrv200(uint32_t function)
{
  bool result = false;

  if (PinUsed(GPIO_RFDIOSEND) || PinUsed(GPIO_RFDIORECV)) {
    switch (function) {
      case FUNC_COMMAND:
        result = DecodeCommand(kRfDioCommands, RfDioCommands);
        break;
      case FUNC_INIT:
        RfDioInit();
        break;
    }
  }
  return result;
}

#endif  // USE_DIO_REMOTE
