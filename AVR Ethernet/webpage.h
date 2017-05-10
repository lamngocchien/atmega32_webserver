/*
 * AVR ETHERNET
 *
 * Created: 11/28/2012 9:34:10 PM
 *  Author: Lam Ngoc Chien
 */
#ifndef WEBPAGE_H
#define WEBPAGE_H
//
#include <avr/pgmspace.h>
//
//PROGMEM char Page1[] = {"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
//"<html xmlns=\"http://www.w3.org/1999/xhtml\" dir=\"ltr\" lang=\"en\"><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"><title>AVR web server</title><style type=\"text/css\">"
//".on { border:1; width: 20px; height: 20px; background: blue; text-align: center; padding: 0px; color: #fff; -moz-border-radius: 10px; -webkit-border-radius: 10px; border-radius: 10px;}"
//".off { border:1; width: 20px; height: 20px; background: white; text-align: center; padding: 0px; color: #fff; -moz-border-radius: 10px; -webkit-border-radius: 10px; border-radius: 10px;}"
//"</style></head><body bgcolor=\"#d0d0d0\" align=\"center\" style=\"padding: 75px 0px 0px 0px;\"><table bgcolor=\"#2F4F4F\" id=\"root\" border=\"1\" align=\"center\" width=\"868px\"><tbody>"
//"<tr><td colspan=\"4\" height=\"100\"><b><div align=\"center\" style=\"font-size:25pt; color:#FFFFFF\">REMOTE CONTROL AVR WEBSERVER</div></b></td></tr>"
//"<tr align=\"left\"><td height=\"25\" valign=\"top\" width=\"75%\"><b><div style=\"color:#FFFFFF\">&nbsp;Devices</div></b></td><td height=\"25\" valign=\"top\" width=\"auto\"><b><div style=\"color:#FFFFFF\">&nbsp;Sensors (<sup>o</sup>C)</div></b></td></tr>"
//"<tr><td height=\"auto\" valign=\"top\" width=\"200\"><p></p><div><form method=\"POST\" action=\"\"><font color=\"#FFFFFF\"><table width=\"95%\" border=\"1\" align=\"center\" id=\"table_device\">"
//"<tr align=\"center\"><td width=\"15%\">Names</td><td width=\"9%\">Now States</td><td width=\"11%\">Control Panel</td><td>Command</td><td width=\"15%\">Countdown (Minutes)</td><td width=\"11%\">Timer Status</td><td width=\"15%\">Auto ON (<sup>o</sup>C)</td></tr>"
//"<tr align=\"center\"><td>Device 1</td><td><div class=\"%CL1\"></div></td><td><input type=\"checkbox\" name=\"RELAY1\" value=\"ON\"%RL1></td><td><input type=\"text\" name=\"Auto1\"/></td><td>%CD1</td><td>%TS1</td><td>%TA1</td></tr>"
//"<tr align=\"center\"><td>Device 2</td><td><div class=\"%CL2\"></div></td><td><input type=\"checkbox\" name=\"RELAY2\" value=\"ON\"%RL2></td><td><input type=\"text\" name=\"Auto2\"/></td><td>%CD2</td><td>%TS2</td><td>%TA2</td></tr>"
//"<tr align=\"center\"><td>Device 3</td><td><div class=\"%CL3\"></div></td><td><input type=\"checkbox\" name=\"RELAY3\" value=\"ON\"%RL3></td><td><input type=\"text\" name=\"Auto3\"/></td><td>%CD3</td><td>%TS3</td><td>%TA3</td></tr>"
//"<tr align=\"center\"><td>Device 4</td><td><div class=\"%CL4\"></div></td><td><input type=\"checkbox\" name=\"RELAY4\" value=\"ON\"%RL4></td><td><input type=\"text\" name=\"Auto4\"/></td><td>%CD4</td><td>%TS4</td><td>%TA4</td></tr>"
//"</table></font><p align=\"center\"><input type=\"submit\" value=\"Submit\" name=\"SUB\"></p></form></div></td><td height=\"auto\" align=\"center\" width=\"auto\"><div style=\"font-size:50pt; color:#FFFFFF\">%AD</div>"
//"</td></tr><tr><td colspan=\"4\" height=\"25\" width=\"auto\"><div align=\"center\" style=\"color:#FFFFFF\">Timezone: GMT+7. Current time <span>%TI</span></div></td></tr></tbody></table></body></html>"};
//#endif //WEBPAGE_H

PROGMEM char Page1[] = {"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
"<html xmlns=\"http://www.w3.org/1999/xhtml\" dir=\"ltr\" lang=\"en\"><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"><title>AVR web server</title><style type=\"text/css\">"
".on { border:1; width: 20px; height: 20px; background: blue; text-align: center; padding: 0px; color: #fff; -moz-border-radius: 10px; -webkit-border-radius: 10px; border-radius: 10px;}.button{width: 100%;}"
".off { border:1; width: 20px; height: 20px; background: white; text-align: center; padding: 0px; color: #fff; -moz-border-radius: 10px; -webkit-border-radius: 10px; border-radius: 10px;} .button2{width: 80px;}"
"</style></head><body bgcolor=\"#d0d0d0\" align=\"center\" style=\"padding: 75px 0px 0px 0px;\"><table bgcolor=\"#2F4F4F\" id=\"root\" border=\"1\" align=\"center\" width=\"868px\"><tbody>"
"<tr><td colspan=\"4\" height=\"100\"><b><div align=\"center\" style=\"font-size:25pt; color:#FFFFFF\">REMOTE CONTROL AVR WEBSERVER</div></b></td></tr>"
"<tr align=\"left\"><td height=\"25\" valign=\"top\" width=\"75%\"><b><div style=\"color:#FFFFFF\">&nbsp;Devices</div></b></td><td height=\"25\" valign=\"top\" width=\"auto\"><b><div style=\"color:#FFFFFF\">&nbsp;Sensors (<sup>o</sup>C)</div></b></td></tr>"
"<tr><td height=\"auto\" valign=\"top\" width=\"200\"><p></p><div><form method=\"POST\" action=\"\"><font color=\"#FFFFFF\"><table width=\"95%\" border=\"1\" align=\"center\" id=\"table_device\">"
"<tr align=\"center\"><td width=\"15%\">Names</td><td width=\"9%\">Now States</td><td width=\"11%\">Control Panel</td><td>Command</td><td width=\"15%\">Countdown (Minutes)</td><td width=\"11%\">Timer Status</td><td width=\"15%\">Auto ON (<sup>o</sup>C)</td></tr>"
"<tr align=\"center\"><td>Device 1</td><td><div class=\"%CL1\"></div></td><td><input class=\"button\" type=\"submit\" value=\"%RS1\" name=\"RL1\"></td><td><input type=\"text\" name=\"Auto1\"/></td><td>%CD1</td><td>%TS1</td><td>%TA1</td></tr>"
"<tr align=\"center\"><td>Device 2</td><td><div class=\"%CL2\"></div></td><td><input class=\"button\" type=\"submit\" value=\"%RS2\" name=\"RL2\"></td><td><input type=\"text\" name=\"Auto2\"/></td><td>%CD2</td><td>%TS2</td><td>%TA2</td></tr>"
"<tr align=\"center\"><td>Device 3</td><td><div class=\"%CL3\"></div></td><td><input class=\"button\" type=\"submit\" value=\"%RS3\" name=\"RL3\"></td><td><input type=\"text\" name=\"Auto3\"/></td><td>%CD3</td><td>%TS3</td><td>%TA3</td></tr>"
"<tr align=\"center\"><td>Device 4</td><td><div class=\"%CL4\"></div></td><td><input class=\"button\" type=\"submit\" value=\"%RS4\" name=\"RL4\"></td><td><input type=\"text\" name=\"Auto4\"/></td><td>%CD4</td><td>%TS4</td><td>%TA4</td></tr>"
"</table></font><p align=\"center\"><input class=\"button2\" type=\"submit\" value=\"Submit\" name=\"SUB\"></p></form></div></td><td height=\"auto\" align=\"center\" width=\"auto\"><div style=\"font-size:50pt; color:#FFFFFF\">%AD</div>"
"</td></tr><tr><td colspan=\"4\" height=\"25\" width=\"auto\"><div align=\"center\" style=\"color:#FFFFFF\">Timezone: GMT+7. Current time <span>%TI</span></div></td></tr></tbody></table></body></html>"};
#endif //WEBPAGE_H
