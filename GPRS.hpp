#pragma once
#include "Arduino.h"
#include "UART.hpp"

#define TIMEOUT 					1000		
#define INTERCHAR_TIMEOUT 			1000 

class GPRS
{
public:
    GPRS(uint8_t uart, uint16_t baudRate = 4800); 

    bool init();
	bool checkPowerUp();
	bool checkSimStatus();
    bool sendSMS(String msg, String number, String cc = "+49");
	bool getSubscriberNumber(char *number);
	
private:
	bool send_rec(String cmd, String resp);
	bool wait_for_response(String resp);
	bool send_AT();
	void send_end_mark();

private:
	uint32_t m_baudRate;
	UART m_uart;
};
