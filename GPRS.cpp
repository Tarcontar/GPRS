#include "GPRS.hpp"
#include "UART.hpp"

GPRS::GPRS(uint8_t uart, uint16_t baudRate)
{
	m_uart = UART(uart);
	m_baudRate = baudRate;
}

bool GPRS::init()
{
	if (!m_uart.begin(m_baudRate))
		return false;
	//m_uart->print("ATE0\r\n"); //disable command return
	if (!checkPowerUp())
	{
		return false;
	}
	if (!send_rec(F("AT+CFUN=1\r\n"), F("OK\r\n"))) // 1: enable full functionality
	{
		return false;
	}
	if (!checkSimStatus())	
	{
		return false;
	}
	return true;
}

bool GPRS::checkPowerUp()
{
	return send_rec(F("AT\r\n"), F("OK\r\n"));
}

bool GPRS::checkSimStatus()
{
	return send_rec(F("AT+CPIN?\r\n"), F("+CPIN: READY"));
}

bool GPRS::sendSMS(String msg, String number, String cc)
{
	if (!startSMS(number, cc))
		return false;
	m_uart << msg;
	return endSMS();
}

bool GPRS::startSMS(String number, String cc)
{
	m_uart << F("\r");
	if (!send_rec(F("AT+CMGF=1\r"), F("OK\r\n")))
		return false;
	delay(300);
	m_uart << F("AT+CMGS=\"");
	if (number.startsWith(cc))
		m_uart << number;
	else
	{
		m_uart << cc;
		number.remove(0, 1);
		m_uart << number;
	}
	if (!send_rec(F("\"\r"), F(">")))
		return false;
	delay(300);
	return true;
}

template <class T>
void GPRS::append(T t)
{
	m_uart << t;
}

bool GPRS::endSMS()
{
	m_uart << F("\r");
	delay(300);
	m_uart << (char)26;
	return wait_for_response(F("OK\r\n"));
}

bool GPRS::getSubscriberNumber(char *number)
{
	if (send_rec(F("AT+CNUM\r\n"), F("+CNUM:")))
	{
		delay(150); //wait for response
		while(m_uart.available())
		{
			char c = m_uart.read();
			if (c == '+')
			{
				int i = 0;
				number[i++] = c;
				while ((c = m_uart.read()) != '"' && m_uart.available())
				{
					number[i++] = c;
				}
				number[i] = '\0';
				break;
			}
		}
	}
	return true;
}


bool GPRS::send_rec(String cmd, String resp)
{
	m_uart << cmd;
	return wait_for_response(resp);
}

bool GPRS::wait_for_response(String resp)
{
	int len = resp.length();
	int sum = 0;
	unsigned long timerStart, prevChar;
	timerStart = millis();
	prevChar = 0;
	while (1)
	{
		if (m_uart.available())
		{
			char c = m_uart.read();
			prevChar = millis();
			if (c == resp[sum])
				sum++;
			else
				sum = 0;
			if (sum == len) break;
		}
		if ((unsigned long) (millis() - timerStart) > TIMEOUT)
			return false;
		//if (((unsigned long) (millis() - prevChar) > INTERCHAR_TIMEOUT) && (prevChar != 0))
		//	return false;
	}
	return true;
}
