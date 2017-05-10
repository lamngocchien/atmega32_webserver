#ifndef WEBPAGE_H
#define WEBPAGE_H
//
#include <avr/pgmspace.h>
//
PROGMEM char Page1[] = {"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
"<html xmlns=\"http://www.w3.org/1999/xhtml\" dir=\"ltr\" lang=\"en\">"
"<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">"
"<title>AVR web server</title>"
"<meta http-equiv=\"refresh\" content=\"30\">" //Auto Reset Page After 25 Seconds
"<style type=\"text/css\"> .on { border:1; width: 20px; height: 20px; background: blue; text-align: center; padding: 0px; color: #fff; -moz-border-radius: 10px; -webkit-border-radius: 10px; border-radius: 10px;}"
".off { border:1; width: 20px; height: 20px; background: white; text-align: center; padding: 0px; color: #fff; -moz-border-radius: 10px; -webkit-border-radius: 10px; border-radius: 10px; border: 1px solid #a4a4a4;}</style>"
"</head>"
"<body bgcolor=\"#d0d0d0\" align=\"center\" style=\"padding: 15px 0px 0px 0px;\">"
"<table bgcolor=\"#2F4F4F\" id=\"root\" border=\"1\" align=\"center\"  width=\"818px\""
"<tbody>"
"<tr><td colspan=\"4\" height=\"125\"><b><div align=\"center\" style=\"font-size:24pt; color:#FFFFFF\">REMOTE CONTROL AVR WEBSERVER</div></b></td></tr>"
"<tr align=\"left\"><td bgcolor=\"#d0d0d0\" colspan=\"4\" height=\"20\" valign=\"top\" width=\"694\"><b><div>&nbsp;Remote control webserver with ATmega32</div></b></td></tr>"
"<tr align=\"left\"><td height=\"20\" valign=\"top\" width=\"75%\"><b><div style=\"color:#FFFFFF\">&nbsp;Devices</div></b></td><td height=\"20\" valign=\"top\" width=\"auto\"><b><div style=\"color:#FFFFFF\">&nbsp;Sensors</div></b></td></tr>"
"<tr><td height=\"auto\" valign=\"top\" width=\"200\"><p></p>"
"<div><form method=\"POST\" action=\"\">"
"<strong><table bgcolor=\"#d0d0d0\" width=\"95%\" border=\"1\" align=\"center\" id=\"table_device\">"
"<tr align=\"center\"><td width=\"15%\">Names</td><td width=\"9%\">Now States</td><td width=\"11%\">Control Panel</td><td>Command</td><td width=\"15%\">Countdown (Minutes)</td><td width=\"11%\">Timer Status</td><td width=\"15%\">Auto ON (<sup>o</sup>C)</td></tr>"
"<tr align=\"center\"><td>Device 1</td><td><div class=\"%CL1\"></td><td><input type=\"checkbox\" name=\"RELAY1\" value=\"ON\"%RL1></div></td><td><input type=\"text\" name=\"Auto1\"/></td><td>%CD1</td><td>%TS1</td><td>%TA1</td></tr>"
"<tr align=\"center\"><td>Device 2</td><td><div class=\"%CL2\"></td><td><input type=\"checkbox\" name=\"RELAY2\" value=\"ON\"%RL2></div></td><td><input type=\"text\" name=\"Auto2\"/></td><td>%CD2</td><td>%TS2</td><td>%TA2</td></tr>"
"<tr align=\"center\"><td>Device 3</td><td><div class=\"%CL3\"></td><td><input type=\"checkbox\" name=\"RELAY3\" value=\"ON\"%RL3></div></td><td><input type=\"text\" name=\"Auto3\"/></td><td>%CD3</td><td>%TS3</td><td>%TA3</td></tr>"
"<tr align=\"center\"><td>Device 4</td><td><div class=\"%CL4\"></td><td><input type=\"checkbox\" name=\"RELAY4\" value=\"ON\"%RL4></div></td><td><input type=\"text\" name=\"Auto4\"/></td><td>%CD4</td><td>%TS4</td><td>%TA4</td></tr>"
"</strong></table>"
"<p align=\"center\"><input type=\"submit\" value=\"Submit\" name=\"SUB\"></p>"
"</form></div></td>"
"<td height=\"auto\" align=\"center\" valign=\"center\" width=\"auto\"><div style=\"font-size:55pt; color:#FFFFFF \">%AD<sup>o</sup>C</div></td></tr>"
"<tr align=\"left\"><td colspan=\"4\" height=\"auto\" valign=\"top\" width=\"auto\"><b><div style=\"color:#FFFFFF\">&nbsp;Manuals</div></b></td></tr>"
"<tr align=\"left\"><td bgcolor=\"#d0d0d0\"colspan=\"4\" height=\"auto\" valign=\"top\" width=\"auto\"><div>&nbsp;Command :&nbsp; <b>&lt;States&gt;&lt;Minutes&gt;</b> and <b>&lt;Temperature Value&gt;&lt;States&gt;</b>.<br/>&nbsp;Ex1: OFF15 means device turn off after 15 minutes.<br/>&nbsp;Ex2: 35ON means if temperature higher 35 Celsius, device is on and reverse.</div></td></tr>"
"<tr><td colspan=\"4\" height=\"auto\" width=\"auto\"><div align=\"center\" style=\"color:#FFFFFF\">Designed by Lam Ngoc Chien<div>Timezone: GMT+7. Current time <span>%TI</span></div></div></td></tr>"
"</tbody></table></body></html>"
};
#endif //WEBPAGE_H
