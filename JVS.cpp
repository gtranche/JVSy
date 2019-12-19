/*
 * JVS.cpp
 *
 *  Created on: 14/apr/2014
 *      Author: k4roshi
 */

#include "Arduino.h"
#include "JVS.h"
#include "constants.h"


JVS::JVS(HardwareSerial& serial) :
_Uart(serial) // Need to initialize references before body
{
	coins1 = 0;
	coins2 = 0;
	coin_pressed_at = 0;
	initialized = false;
	shift_mode = false;
	pressed_smth = false;
	old_key = 0;
}

void JVS::reset() {
	char str[] = { (char) CMD_RESET, (char) CMD_RESET_ARG };
	this->write_packet(BROADCAST, str, 2);
	delay(ASSIGN_DELAY);
	Serial.println("RESET");
}

void JVS::assign(int attempt) {
	char str[] = { (char) CMD_ASSIGN_ADDR, (char) attempt };
	this->cmd(BROADCAST, str, 2);
	Serial.println("ADDR");
}

void JVS::init(int board) {
	Serial.println("ADDR");
	char str[] = { (char) CMD_ASSIGN_ADDR, board };
	this->cmd(BROADCAST, str, 2);
	Serial.println("REQ");
	char str1[] = { (char) CMD_REQUEST_ID };
	this->cmd(board, str1, 1);
	Serial.println("CMD");
	char str2[] = { (char) CMD_COMMAND_VERSION };
	this->cmd(board, str2, 1);
	Serial.println("JVS");
	char str3[] = { (char) CMD_JVS_VERSION };
	this->cmd(board, str3, 1);
	Serial.println("CMS");
	char str4[] = { (char) CMD_COMMS_VERSION };
	this->cmd(board, str4, 1);
	Serial.println("CAP");
	char str5[] = { (char) CMD_CAPABILITIES };
	this->cmd(board, str5, 1);
	initialized = true;
}

void JVS::switches(int board) {
	char str[] = { CMD_READ_DIGITAL, 0x02, 0x02, CMD_READ_COINS, 0x02, CMD_READ_ANALOG, 0x04};
	//char str[ ] = { 0x20, 0x02, 0x02, 0x21, 0x02, 0x22, 0x08};
	this->write_packet(board, str, sizeof str);
	char incomingByte;
	while (!_Uart.available()) {
	}
	while (_Uart.read() != 0xE0) {
	} // wait for sync
	while (_Uart.read() != 0) {
	} // only if it's for me
	while (!_Uart.available()) {
	} // wait for length
	int length = _Uart.read();
	int counter = 0;
  int X_player1 = 512;
  int Y_player1 = 512;
  int X_player2 = 512;
  int Y_player2 = 512; 
	int coin1 = 0;
  int coin2 = 0;
	int key = 0;
	bool old_shift = shift_mode;

	if	(DEBUG_MODE) {
		Serial.print("swthc: E0 0 ");
		Serial.print(length, HEX);
	}
	while (counter < length) {
		while (!_Uart.available()) {
		}
		incomingByte = _Uart.read();
		if (DEBUG_MODE){
			Serial.print(" ");
			Serial.print(incomingByte, HEX);
		}
		switch (counter) {
		// fourth byte (first three bytes are sync and
		case 3:
			// p1 b1
			shift_mode = bitRead(incomingByte, 7);

			if (shift_mode != old_shift) {
				if (shift_mode == 0 && !pressed_smth)
					//never pressed anything, wants start
					key = KEY_1;
				else
					pressed_smth = false;
			}
			if (shift_mode) {
//				Serial.println("shiftmode on");
				if(bitRead(incomingByte, 1))
					key = KEY_5;
				if(bitRead(incomingByte, 2))
					key = KEY_TAB;
				if(bitRead(incomingByte, 3))
					key = KEY_ENTER;
				if(bitRead(incomingByte, 4))
					key = KEY_P;

				if (key)
					pressed_smth = true;
			} else {
				Joystick.button(1,bitRead(incomingByte, 1));
				Joystick.button(2,bitRead(incomingByte, 0));
				if bitRead(incomingByte, 2)
										X_player1 += 511;
				if bitRead(incomingByte, 3)
										X_player1 -= 512;
				//Joystick.X(X_player1);
				if bitRead(incomingByte, 4)
										Y_player1 += 511;
				if bitRead(incomingByte, 5)
										Y_player1 -= 512;
				//Joystick.Y(Y_player1);
				Joystick.button(7,bitRead(incomingByte, 6));
			}
			break;
		case 4:
			// p1 b2
			if (shift_mode) {

			} else {
				Joystick.button(9,bitRead(incomingByte, 2));
				Joystick.button(10,bitRead(incomingByte, 3));
				Joystick.button(11,bitRead(incomingByte, 4));
				Joystick.button(12,bitRead(incomingByte, 5));
				Joystick.button(13,bitRead(incomingByte, 6));
				Joystick.button(14,bitRead(incomingByte, 7));
			}
			break;
		case 5:
			// p2 b1
			if (shift_mode) {
				if(bitRead(incomingByte, 7))
					key = KEY_ESC;
				if (key)
					pressed_smth = true;
			} else {
				Joystick2.button(1,bitRead(incomingByte, 0));
				Joystick2.button(2,bitRead(incomingByte, 1));
				if bitRead(incomingByte, 2)
										X_player2 += 511;
				if bitRead(incomingByte, 3)
										X_player2 -= 512;
				//Joystick2.X(X_player2);
				if bitRead(incomingByte, 4)
										Y_player2 += 511;
				if bitRead(incomingByte, 5)
										Y_player2 -= 512;
				//Joystick2.Y(Y_player2);
				Joystick2.button(7,bitRead(incomingByte, 6));
				if bitRead(incomingByte, 7)
					Keyboard.press(KEY_2);
				else
					Keyboard.release(KEY_2);
			}
			break;
		case 6:
			// p2 b2
			if (shift_mode) {

			} else {
				Joystick2.button(25,bitRead(incomingByte, 2));
				Joystick2.button(26,bitRead(incomingByte, 3));
				Joystick2.button(27,bitRead(incomingByte, 4));
				Joystick2.button(28,bitRead(incomingByte, 5));
				Joystick2.button(29,bitRead(incomingByte, 6));
				Joystick2.button(30,bitRead(incomingByte, 7));
			}
			break;
		case 8:
			// coins 1 status
			break;
		case 9:
			// coins1
			if (incomingByte > coins1) {
				// added coin
				coin1 = 1;
				coins1 = incomingByte;
			}
			if (coin1){
				coin_pressed_at = millis();
				Keyboard.press(KEY_5);
			} else if (coin_pressed_at > 0){
				if (millis() - coin_pressed_at > 50){
					coin_pressed_at = 0;
					Keyboard.release(KEY_5);
				}
			}
			break;
		case 10:
			// coins2 status
			break;
		case 11:
			// coins2
      if (incomingByte > coins2) {
        // added coin
        coin2 = 1;
        coins2 = incomingByte;
      }
      if (coin2){
        coin_pressed_at = millis();
        Keyboard.press(KEY_6);
      } else if (coin_pressed_at > 0){
        if (millis() - coin_pressed_at > 50){
          coin_pressed_at = 0;
          Keyboard.release(KEY_6);
        }
      }
      break;
    case 13:
      // Analog X P1 
      //Serial.print(incomingByte,DEC);
      if(incomingByte < 0) {
        X_player1 = 1024 + (incomingByte * 4);
      } else {
        X_player1 = 4 * incomingByte;
      }
      Joystick.X(X_player1);
      break;
    case 15:
    // Analog Y P1 
      if(incomingByte < 0) {
        Y_player1 = 1024 + (incomingByte * 4);
      } else {
        Y_player1 = 4 * incomingByte;
      }
      Joystick.Y(Y_player1);
      break;
    case 17:
      // Analog X P2
      //Serial.print(incomingByte,DEC);
      if(incomingByte < 0) {
        X_player2 = 1024 + (incomingByte * 4);
      } else {
        X_player2 = 4 * incomingByte;
      }
      Joystick2.X(X_player2);
      break;
    case 19:
      // Analog Y P2
      if(incomingByte < 0) {
        Y_player2 = 1024 + (incomingByte * 4);
      } else {
        Y_player2 = 4 * incomingByte;
      }
      Joystick2.Y(Y_player2);
      break;
    }  
		counter++;
    Joystick.send_now();
    Joystick2.send_now();
	}

	if (key){
		if (key != old_key){
			old_key = key;
		//	Serial.print("pressed key: ");
		//	Serial.println(key, HEX);
			Keyboard.set_key1(key);
		}
	} else {
		if (old_key){
	//	Serial.print("old_key: ");
	//	Serial.println(old_key, HEX);
		Keyboard.set_key1(0);
		old_key = 0;
		}
	}
	Keyboard.send_now();
	delay(SWCH_DELAY);

	//	if (coins1 > 0){
	//		char str1[ ] = {CMD_DECREASE_COIN};
	//		this->cmd(board, str1, 1);
	//	}
	if (DEBUG_MODE)
		Serial.println();
}

int* JVS::cmd(char destination, char data[], int size) {
	this->write_packet(destination, data, size);
	char incomingByte;
	while (!_Uart.available()) {
	}
	while (_Uart.read() != 0xE0) {
	} // wait for sync
	while (_Uart.read() != 0) {
	} // only if it's for me
	while (!_Uart.available()) {
	} // wait for length
	int length = _Uart.read();
	Serial.print("Received: E0 0 ");
	Serial.print(length, HEX);
	int counter = 0;
	int * res = (int *) malloc(length * sizeof(int));
	while (counter < length) {
		while (!_Uart.available()) {
		}
		incomingByte = _Uart.read();
		res[counter] = incomingByte;
		// actually do something with incomingByte
		Serial.print(" ");
		Serial.print(res[counter], HEX);
		counter++;
	}
	Serial.println();
	delay(CMD_DELAY);
	return res;
}

void JVS::write_packet(char destination, char data[], int size) {
	_Uart.write(SYNC);
	_Uart.write(destination);
	_Uart.write(size + 1);
	char sum = destination + size + 1;
	for (int i = 0; i < size; i++) {
		if (data[i] == SYNC || data[i] == ESCAPE) {
			_Uart.write(ESCAPE);
			_Uart.write(data[i] - 1);
		} else {
			_Uart.write(data[i]);
		}
		sum = (sum + data[i]) % 256;
	}
	_Uart.write(sum);
	_Uart.flush();
}
//Button debugs
//Serial.print("1P1: ");
//for (int i = 0; i<=7; i++)
//	Serial.print(bitRead(incomingByte, i));
//Serial.println();
