#ifndef __CONFIG_ME_H_
#define __CONFIG_ME_H_

// User must set these for own context:

// This is the server's base URL name.  Example "www.google.com"
// Note that when you Fork a FLOW, it will typically assign  either
// "run-east.att.io" or "run-west.att.io", so be sure to check this.
static const char * MY_SERVER_URL       = "run-west.att.io";

// These are FLOW fields from the Endpoints tab:
#define FLOW_BASE_URL                   "/41d3470766b7b/b156b6d68ab5/c3cfb9f5e6c0be3/in/flow"
#define FLOW_INPUT_NAME                 "/progress"
#define FLOW_INPUT_END                  "/end"

// Unless you want to use a different protocol, this field should be left as is:
#define FLOW_URL_TYPE                   " HTTP/1.1\r\nHost: "

// This identifier specifies with which FLOW device you are communicating. 
// If you only have one devive there then you can just leave this as is.
// Once your FLOW device has been initialized (Virtual Device Initialize clicked),
// the Virtual Device will show up in M2X.  This is its "DEVICE SERIAL" field
#define FLOW_DEVICE_NAME                "vstarterkit001"

// This constant defines how often sensors are read and sent up to FLOW
#define SENSOR_UPDATE_INTERVAL_MS       5000; //5 seconds

// Specify here how many sensor parameters you want reported to FLOW.
// You can use only the temperature and humidity from the shield HTS221
// or you can add the reading of the FXO8700CQ motion sensor on the FRDM-K64F board
// or if you have a SiLabs PMOD plugged into the shield, you can add its proximity sensor,
// UV light, visible ambient light and infrared ambient light readings

// This is the APN name for the cellular network, you will need to change this, check the instructions included with your SIM card kit:
static const char * MY_APN_STR          = "m2m.com.attz";

//This is for normal HTTP.  If you want to use TCP to a specific port, change that here:
static const char * MY_PORT_STR         = "80";

#endif
