#ifndef GLOBAL_H
#define GLOBAL_H

#include <TimeLib.h>
#include "WifiController.h"

#define ACCESS_POINT_NAME  "WOORDKLOK"
#define ACCESS_POINT_PASSWORD  "12345678"
WifiController wifiController(ACCESS_POINT_NAME, ACCESS_POINT_PASSWORD);

ESP8266WebServer server(80);									// The Webserver
boolean firstStart = true;	// On firststart = true, NTP will try to get a valid time
strDateTime DateTime;// Global DateTime structure, will be refreshed every Second
WiFiUDP UDPNTPClient;											// NTP Client
unsigned long UnixTimestamp = 0;				// GLOBALTIME  ( Will be set by NTP)
unsigned long UnixTimestamp_adjusted = 0;
boolean Refresh = false; // For Main Loop, to refresh things like GPIO / WS2812
boolean FirstSettings = true;
boolean FirstPackage = false;
int cNTP_Update = 0;							// Counter for Updating the time via NTP
int cGet_Time_Update = 0;
Ticker tkSecond;					// Second - Timer for Updating Datetime Structure
byte Minute_Old = 100;// Helpvariable for checking, when a new Minute comes up (for Auto Turn On / Off)

struct strConfig {
	String ssid;
	String password;
	byte IP[4];
	byte Netmask[4];
	byte Gateway[4];
	boolean dhcp;
	String ntpServerName;
	long Update_Time_Via_NTP_Every;
	long timezone;
	boolean daylight;
	String DeviceName;
	boolean AutoTurnOff;
	boolean AutoTurnOn;
	byte TurnOffHour;
	byte TurnOffMinute;
	byte TurnOnHour;
	byte TurnOnMinute;
	byte LED_R;
	byte LED_G;
	byte LED_B;

	// clock settings
	boolean SoundOnOff;
	boolean TouchOnOff;
	byte Notat;
	int LMin;
	int LMax;
  int Transpd;
	int ClockMode;
	int TouchFil;
	int TouchTrL;
	int TouchTrH;
	int TouchTiS;
	int TouchTiL;
	boolean AutoStart;
	boolean Clock_NTP_Update;
	int GetTimeMinute;
} config;

/*
 **
 ** CONFIGURATION HANDLING
 **
 */
void ConfigureWifi() {
	debug_print("Configuring Wifi; SSID: " + config.ssid + "; password: " + config.password);
	if (!config.dhcp) {
		debug_printf(
			"Wifi config; IP: %d.%d.%d.%d; Gateway: %d.%d.%d.%d",
			(int)config.IP[0], (int)config.IP[1], (int)config.IP[2], (int)config.IP[3],
			(int)config.Gateway[0], (int)config.Gateway[1], (int)config.Gateway[2], (int)config.Gateway[3]
		);
		wifiController.setSTAStaticIPConfig(
			IPAddress(config.IP[0], config.IP[1], config.IP[2], config.IP[3]),
			IPAddress(config.Gateway[0], config.Gateway[1], config.Gateway[2], config.Gateway[3]),
			IPAddress(config.Netmask[0], config.Netmask[1], config.Netmask[2], config.Netmask[3])
		);
	} else {
		wifiController.setSTAStaticIPConfig(IPAddress(), IPAddress(), IPAddress());
	}
	WiFi.disconnect();
	wifiController.autoConnect(config.ssid.c_str(), config.password.c_str());
}

void WriteConfig() {
	debug_print("Writing Config");
	EEPROM.write(0, 'C');
	EEPROM.write(1, 'F');
	EEPROM.write(2, 'G');

	EEPROM.write(16, config.dhcp);
	EEPROM.write(17, config.daylight);

	EEPROMWritelong(18, config.Update_Time_Via_NTP_Every); // 4 Byte

	EEPROMWritelong(22, config.timezone);  // 4 Byte

	EEPROM.write(26, config.LED_R);
	EEPROM.write(27, config.LED_G);
	EEPROM.write(28, config.LED_B);

	EEPROM.write(32, config.IP[0]);
	EEPROM.write(33, config.IP[1]);
	EEPROM.write(34, config.IP[2]);
	EEPROM.write(35, config.IP[3]);

	EEPROM.write(36, config.Netmask[0]);
	EEPROM.write(37, config.Netmask[1]);
	EEPROM.write(38, config.Netmask[2]);
	EEPROM.write(39, config.Netmask[3]);

	EEPROM.write(40, config.Gateway[0]);
	EEPROM.write(41, config.Gateway[1]);
	EEPROM.write(42, config.Gateway[2]);
	EEPROM.write(43, config.Gateway[3]);

	WriteStringToEEPROM(64, config.ssid);
	WriteStringToEEPROM(96, config.password);
	WriteStringToEEPROM(128, config.ntpServerName);

	EEPROM.write(300, config.AutoTurnOn);
	EEPROM.write(301, config.AutoTurnOff);
	EEPROM.write(302, config.TurnOnHour);
	EEPROM.write(303, config.TurnOnMinute);
	EEPROM.write(304, config.TurnOffHour);
	EEPROM.write(305, config.TurnOffMinute);
	WriteStringToEEPROM(306, config.DeviceName);
	EEPROM.write(350, config.AutoStart);
	EEPROM.write(351, config.GetTimeMinute);
	EEPROM.write(352, config.Clock_NTP_Update);
	EEPROM.commit();
}

void WriteClockConfig() {
	debug_print("Writing Config");
	EEPROM.write(450, config.SoundOnOff);
	EEPROM.write(451, config.Notat);
	EEPROM.write(452, config.LMin);
	EEPROM.write(453, config.LMax);
	EEPROM.write(454, config.ClockMode);
	EEPROM.write(455, config.TouchOnOff);
	EEPROM.write(456, config.TouchFil);
	EEPROM.write(457, config.TouchTrL);
	EEPROM.write(458, config.TouchTrH);
	EEPROM.write(459, config.TouchTiS);
	EEPROM.write(460, config.TouchTiL);
  EEPROM.write(461,config.Transpd);
	EEPROM.commit();
}

boolean ReadConfig() {
	debug_print("Reading Configuration");
	if (EEPROM.read(0) == 'C' && EEPROM.read(1) == 'F' && EEPROM.read(2) == 'G') {
		debug_print("Configuration Found!");
		config.dhcp = EEPROM.read(16);

		config.daylight = EEPROM.read(17);

		config.Update_Time_Via_NTP_Every = EEPROMReadlong(18); // 4 Byte

		config.timezone = EEPROMReadlong(22); // 4 Byte

		config.LED_R = EEPROM.read(26);
		config.LED_G = EEPROM.read(27);
		config.LED_B = EEPROM.read(28);

		config.IP[0] = EEPROM.read(32);
		config.IP[1] = EEPROM.read(33);
		config.IP[2] = EEPROM.read(34);
		config.IP[3] = EEPROM.read(35);
		config.Netmask[0] = EEPROM.read(36);
		config.Netmask[1] = EEPROM.read(37);
		config.Netmask[2] = EEPROM.read(38);
		config.Netmask[3] = EEPROM.read(39);
		config.Gateway[0] = EEPROM.read(40);
		config.Gateway[1] = EEPROM.read(41);
		config.Gateway[2] = EEPROM.read(42);
		config.Gateway[3] = EEPROM.read(43);
		config.ssid = ReadStringFromEEPROM(64);
		config.password = ReadStringFromEEPROM(96);
		config.ntpServerName = ReadStringFromEEPROM(128);

		config.AutoTurnOn = EEPROM.read(300);
		config.AutoTurnOff = EEPROM.read(301);
		config.TurnOnHour = EEPROM.read(302);
		config.TurnOnMinute = EEPROM.read(303);
		config.TurnOffHour = EEPROM.read(304);
		config.TurnOffMinute = EEPROM.read(305);
		config.DeviceName = ReadStringFromEEPROM(306);
		config.AutoStart = EEPROM.read(350);
		config.GetTimeMinute = EEPROM.read(351);
		config.Clock_NTP_Update = EEPROM.read(352);

		return true;

	} else {
		debug_print("Configuration NOT found");
		return false;
	}
}

void ReadClockConfig() {
	debug_print("Reading Clock Configuration");
	config.SoundOnOff = EEPROM.read(450);
	config.Notat = EEPROM.read(451);
	config.LMin = EEPROM.read(452);
	config.LMax = EEPROM.read(453);
	config.ClockMode = EEPROM.read(454);
	config.TouchOnOff = EEPROM.read(455);
	config.TouchFil = EEPROM.read(456);
	config.TouchTrL = EEPROM.read(457);
	config.TouchTrH = EEPROM.read(458);
	config.TouchTiS = EEPROM.read(459);
	config.TouchTiL = EEPROM.read(460);
  config.Transpd = EEPROM.read(461);
	debug_print("Clock Settings Read");
}

/*
 **
 **  NTP
 **
 */

const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];
boolean NTPRefresh() {
	UDPNTPClient.begin(2390);  // Port for NTP receive

	if (WiFi.status() == WL_CONNECTED) {
		IPAddress timeServerIP;
		WiFi.hostByName(config.ntpServerName.c_str(), timeServerIP);

		// WriteLogLine("Sending NTP packet... ");
		memset(packetBuffer, 0, NTP_PACKET_SIZE);
		packetBuffer[0] = 0b11100011;   // LI, Version, Mode
		packetBuffer[1] = 0;     // Stratum, or type of clock
		packetBuffer[2] = 6;     // Polling Interval
		packetBuffer[3] = 0xEC;  // Peer Clock Precision
		packetBuffer[12] = 49;
		packetBuffer[13] = 0x4E;
		packetBuffer[14] = 49;
		packetBuffer[15] = 52;
		UDPNTPClient.beginPacket(timeServerIP, 123);
		UDPNTPClient.write(packetBuffer, NTP_PACKET_SIZE);
		UDPNTPClient.endPacket();

		delay(1000);

		int cb = UDPNTPClient.parsePacket();
		if (!cb) {
			//  WriteLogLine("NTP no packet yet");
		} else {
			// WriteLogLine("NTP packet received; length: " + (String) cb);
			UDPNTPClient.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
			unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
			unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
			unsigned long secsSince1900 = highWord << 16 | lowWord;
			const unsigned long seventyYears = 2208988800UL;
			unsigned long epoch = secsSince1900 - seventyYears;
			UnixTimestamp = epoch;
			FirstPackage = true;
			// WriteLogLine("NTP packet time is: " + (String) epoch);
			UDPNTPClient.flush();
			UDPNTPClient.stop();
			return true;
		}
	}
	UDPNTPClient.stop();
	return false;
}

void Second_Tick() {
	strDateTime tempDateTime;
	cNTP_Update++;
	cGet_Time_Update++;
	UnixTimestamp++;
	ConvertUnixTimeStamp(UnixTimestamp + (config.timezone * 360), &tempDateTime);
	if (config.daylight) // Sommerzeit beachten
		if (summertime(tempDateTime.year, tempDateTime.month, tempDateTime.day,
				tempDateTime.hour, 0)) {
			ConvertUnixTimeStamp(UnixTimestamp + (config.timezone * 360) + 3600,
					&DateTime);
		} else {
			DateTime = tempDateTime;
		}
	else {
		DateTime = tempDateTime;
	}
	Refresh = true;
}

void Update_Clock_Settings() {
	int delaytijd = 100;
	debug_print("update clock settings on startup");
	delay(delaytijd);
	Clock::setNotation(config.Notat);
	delay(delaytijd);
	Clock::setLMin(config.LMin);
	delay(delaytijd);
	Clock::setLMax(config.LMax);
	delay(delaytijd);
	Clock::enableSound(config.SoundOnOff);
  delay(delaytijd);
  Clock::Transpd(config.Transpd);
	delay(delaytijd);
	Clock::setTouch(config.TouchFil, config.TouchTrH, config.TouchTrL, config.TouchTiS, config.TouchTiL);
	delay(delaytijd);
	Clock::setTime();
	delay(delaytijd);
	Clock::setMode(config.ClockMode);
}

#endif
