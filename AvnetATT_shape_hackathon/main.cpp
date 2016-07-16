#include "mbed.h"
#include <cctype>
#include <string>
#include "SerialBuffered.h"
#include "HTS221.h"
#include "config_me.h"
#include "wnc_control.h"
#include "sensors.h"

#include "hardware.h"
I2C i2c(PTC11, PTC10);    //SDA, SCL -- define the I2C pins being used

// comment out the following line if color is not supported on the terminal
#define USE_COLOR
#ifdef USE_COLOR
 #define BLK "\033[30m"
 #define RED "\033[31m"
 #define GRN "\033[32m"
 #define YEL "\033[33m"
 #define BLU "\033[34m"
 #define MAG "\033[35m"
 #define CYN "\033[36m"
 #define WHT "\033[37m"
 #define DEF "\033[39m"
#else
 #define BLK
 #define RED
 #define GRN
 #define YEL
 #define BLU
 #define MAG
 #define CYN
 #define WHT
 #define DEF
#endif

#define MDM_DBG_OFF                             0
#define MDM_DBG_AT_CMDS                         (1 << 0)
int mdm_dbgmask = MDM_DBG_OFF;

Serial         pc(USBTX, USBRX);
SerialBuffered mdm(PTD3, PTD2, 128);
DigitalOut led_green(LED_GREEN);
DigitalOut led_red(LED_RED);
DigitalOut led_blue(LED_BLUE);

DigitalOut  mdm_uart2_rx_boot_mode_sel(PTC17);  // on powerup, 0 = boot mode, 1 = normal boot
DigitalOut  mdm_power_on(PTB9);                 // 0 = turn modem on, 1 = turn modem off (should be held high for >5 seconds to cycle modem)
DigitalOut  mdm_wakeup_in(PTC2);                // 0 = let modem sleep, 1 = keep modem awake -- Note: pulled high on shield

DigitalOut  mdm_reset(PTC12);                   // active high      

DigitalOut  shield_3v3_1v8_sig_trans_ena(PTC4); // 0 = disabled (all signals high impedence, 1 = translation active
DigitalOut  mdm_uart1_cts(PTD0);

#define TOUPPER(a) (a) //toupper(a)

const char ok_str[] = "OK";
const char error_str[] = "ERROR";

#define MDM_OK                                  0
#define MDM_ERR_TIMEOUT                         -1

#define MAX_AT_RSP_LEN                          255

ssize_t mdm_getline(char *buff, size_t size, int timeout_ms) {
    int cin = -1;
    int cin_last;
    
    if (NULL == buff || size == 0) {
        return -1;
    }

    size_t len = 0;
    Timer timer;
    timer.start();
    while ((len < (size-1)) && (timer.read_ms() < timeout_ms)) {
        if (mdm.readable()) {
            cin_last = cin;
            cin = mdm.getc();
            if (isprint(cin)) {
                buff[len++] = (char)cin;
                continue;
            } else if (('\r' == cin_last) && ('\n' == cin)) {
                break;
            }
        }
        wait_ms(1);
    }
    buff[len] = (char)NULL;
    
    return len;
}

int mdm_sendAtCmd(const char *cmd, const char **rsp_list, int timeout_ms) {
    if (cmd && strlen(cmd) > 0) {
        if (mdm_dbgmask & MDM_DBG_AT_CMDS) {
            printf(MAG "ATCMD: " DEF "--> " GRN "%s" DEF "\n", cmd);
        }
        mdm.printf("%s\r\n", cmd);
    }
    
    if (rsp_list) {
        Timer   timer;
        char    rsp[MAX_AT_RSP_LEN+1];
        int     len;
        
        timer.start();
        while (timer.read_ms() < timeout_ms) {
            len = mdm_getline(rsp, sizeof(rsp), timeout_ms - timer.read_ms());
            
            if (len < 0)
                return MDM_ERR_TIMEOUT;

            if (len == 0)
                continue;
                
            if (mdm_dbgmask & MDM_DBG_AT_CMDS) {
                printf(MAG "ATRSP: " DEF "<-- " CYN "%s" DEF "\n", rsp);
            }
        
            if (rsp_list) {
                int rsp_idx = 0;
                while (rsp_list[rsp_idx]) {
                    if (strcasecmp(rsp, rsp_list[rsp_idx]) == 0) {
                        return rsp_idx;
                    }
                    rsp_idx++;
                }
            }
        }
        return MDM_ERR_TIMEOUT;
    }
    return MDM_OK;
}

int mdm_init(void) {
    // Hard reset the modem (doesn't go through
    // the signal level translator)
    mdm_reset = 0;

    // disable signal level translator (necessary
    // for the modem to boot properly).  All signals
    // except mdm_reset go through the level translator
    // and have internal pull-up/down in the module. While
    // the level translator is disabled, these pins will
    // be in the correct state.  
    shield_3v3_1v8_sig_trans_ena = 0;

    // While the level translator is disabled and ouptut pins
    // are tristated, make sure the inputs are in the same state
    // as the WNC Module pins so that when the level translator is
    // enabled, there are no differences.
    mdm_uart2_rx_boot_mode_sel = 1;   // UART2_RX should be high
    mdm_power_on = 0;                 // powr_on should be low
    mdm_wakeup_in = 1;                // wake-up should be high
    mdm_uart1_cts = 0;                // indicate that it is ok to send

   // Now, wait for the WNC Module to perform its initial boot correctly
    wait(1.0);
  
    // The WNC module initializes comms at 115200 8N1 so set it up
    mdm.baud(115200);
    
    //Now, enable the level translator, the input pins should now be the
    //same as how the M14A module is driving them with internal pull ups/downs.
    //When enabled, there will be no changes in these 4 pins...
    shield_3v3_1v8_sig_trans_ena = 1;

    // Now, give the modem 60 secons to start responding by
    // sending simple 'AT' commands to modem once per second.
    Timer timer;
    timer.start();
    while (timer.read() < 60) {
        const char * rsp_lst[] = { ok_str, error_str, NULL };
        int rc = mdm_sendAtCmd("AT", rsp_lst, 500);
        if (rc == 0)
            return true; //timer.read();
        wait_ms(1000 - (timer.read_ms() % 1000));
        pc.printf("\r%d",timer.read_ms()/1000);
    }
    return false;       
}

int mdm_sendAtCmdRsp(const char *cmd, const char **rsp_list, int timeout_ms, string * rsp, int * len) {
    static char cmd_buf[3200];  // Need enough room for the WNC sockreads (over 3000 chars)
    size_t n = strlen(cmd);
    if (cmd && n > 0) {
        if (mdm_dbgmask & MDM_DBG_AT_CMDS) {
            printf(MAG "ATCMD: " DEF "--> " GRN "%s" DEF "\n", cmd);
        }
        while (n--) {
            mdm.putc(*cmd++);
            wait_ms(1);
        };
        mdm.putc('\r');
        wait_ms(1);
        mdm.putc('\n');
        wait_ms(1);
    }

    if (rsp_list) {
        rsp->erase(); // Clean up from prior cmd response
        *len = 0;
        Timer   timer;
        timer.start();
        while (timer.read_ms() < timeout_ms) {
            int lenCmd = mdm_getline(cmd_buf, sizeof(cmd_buf), timeout_ms - timer.read_ms());

            if (lenCmd == 0)
                continue;

            if (lenCmd < 0)
                return MDM_ERR_TIMEOUT;
            else {
                *len += lenCmd;
                *rsp += cmd_buf;
            }

            if (mdm_dbgmask & MDM_DBG_AT_CMDS) {
                printf(MAG "ATRSP: " DEF "<-- " CYN "%s" DEF "\n", cmd_buf);
            }

            int rsp_idx = 0;
            while (rsp_list[rsp_idx]) {
                if (strcasecmp(cmd_buf, rsp_list[rsp_idx]) == 0) {
                    return rsp_idx;
                }
                rsp_idx++;
            }
        }
        return MDM_ERR_TIMEOUT;
    }
    pc.printf("D %s",rsp);
    return MDM_OK;
}

void reinitialize_mdm(void)
{
    // Initialize the modem
    printf(GRN "Modem RE-initializing..." DEF "\r\n");
    if (!mdm_init()) {
        printf(RED "\n\rModem RE-initialization failed!" DEF "\n");
    }
    printf("\r\n");
}
// These are built on the fly
string MyServerIpAddress;
string MySocketData;

// These are to be built on the fly
string my_temp;
string my_humidity;

#define CTOF(x)  ((x)*1.8+32)

//********************************************************************************************************************************************
//* Create string with sensor readings that can be sent to flow as an HTTP get
//********************************************************************************************************************************************
K64F_Sensors_t  SENSOR_DATA =
{
    .Temperature        = "0",
    .Humidity           = "0",
    .AccelX             = "0",
    .AccelY             = "0",
    .AccelZ             = "0",
    .MagnetometerX      = "0",
    .MagnetometerY      = "0",
    .MagnetometerZ      = "0",
    .AmbientLightVis    = "0",
    .AmbientLightIr     = "0",
    .UVindex            = "0",
    .Proximity          = "0",
    .Temperature_Si7020 = "0",
    .Humidity_Si7020    = "0"
};

void GenerateModemString(char * modem_string)
{
    sprintf(modem_string, "POST %s%s?serial=%s&temp=%s&humidity=%s %s%s\r\n\r\n", FLOW_BASE_URL, FLOW_INPUT_NAME, FLOW_DEVICE_NAME, FLOW_URL_TYPE, MY_SERVER_URL);
} //GenerateModemString        
            
            
//Periodic timer
Ticker OneMsTicker;
volatile bool bTimerExpiredFlag = false;
int OneMsTicks = 0;
int iTimer1Interval_ms = 1000;
//********************************************************************************************************************************************
//* Periodic 1ms timer tick
//********************************************************************************************************************************************
void OneMsFunction() 
{
    OneMsTicks++;
    if ((OneMsTicks % iTimer1Interval_ms) == 0)
    {
        bTimerExpiredFlag = true;
    }            
} //OneMsFunction()

//********************************************************************************************************************************************
//* Set the RGB LED's Color
//* LED Color 0=Off to 7=White.  3 bits represent BGR (bit0=Red, bit1=Green, bit2=Blue) 
//********************************************************************************************************************************************
void SetLedColor(unsigned char ucColor)
{
    //Note that when an LED is on, you write a 0 to it:
    led_red = !(ucColor & 0x1); //bit 0
    led_green = !(ucColor & 0x2); //bit 1
    led_blue = !(ucColor & 0x4); //bit 2
} //SetLedColor()

//********************************************************************************************************************************************
//* Process JSON response messages
//********************************************************************************************************************************************
bool extract_JSON(char* search_field, char* found_string)
{
    char* beginquote;
    char* endquote;
    beginquote = strchr(search_field, '{'); //start of JSON
    endquote = strchr(search_field, '}'); //end of JSON
    if (beginquote != 0)
    {
        uint16_t ifoundlen;
        if (endquote != 0)
        {
            ifoundlen = (uint16_t) (endquote - beginquote) + 1;
            strncpy(found_string, beginquote, ifoundlen );
            found_string[ifoundlen] = 0; //null terminate
            return true;
        }
        else
        {
            endquote = strchr(search_field, '\0'); //end of string...  sometimes the end bracket is missing
            ifoundlen = (uint16_t) (endquote - beginquote) + 1;
            strncpy(found_string, beginquote, ifoundlen );
            found_string[ifoundlen] = 0; //null terminate
            return false;
        }
    }
    else
    {
        return false;
    }
} //extract_JSON
// check for progress
bool parse_progress_JSON(char* json_string)
{
    char* beginquote;
    char token[] = "\"progress\":\"";
    beginquote = strstr(json_string, token );
    if ((beginquote != 0))
    {
        char cLedColor = beginquote[strlen(token)];
        printf(GRN "LED Found : %c" DEF "\r\n", cLedColor);
        switch(cLedColor)
        {
            case 'O':
            { //Off
                SetLedColor(0);
                break;
            }
            case 'R':
            { //Red
                SetLedColor(1);
                break;
            }
            case 'G':
            { //Green
                SetLedColor(2);
                break;
            }
            case 'Y':
            { //Yellow
                SetLedColor(3);
                break;
            }
            case 'B':
            { //Blue
                SetLedColor(4);
                break;
            }
            case 'M':
            { //Magenta
                SetLedColor(5);
                break;
            }
            case 'T':
            { //Turquoise
                SetLedColor(6);
                break;
            }
            case 'W':
            { //White
                SetLedColor(7);
                break;
            }
            default:
            {
                break;
            }
        } //switch(cLedColor)
        return true;
    }
    else
    {
        return false;
    }
} //parse_JSON

// parse end json
bool parse_end_JSON(char* json_string)
{
    char* beginquote;
    char token[] = "\"progress\":\"";
    beginquote = strstr(json_string, token );
    if ((beginquote != 0))
    {
        char cLedColor = beginquote[strlen(token)];
        printf(GRN "LED Found : %c" DEF "\r\n", cLedColor);
        switch(cLedColor)
        {
            case 'O':
            { //Off
                SetLedColor(0);
                break;
            }
            case 'R':
            { //Red
                SetLedColor(1);
                break;
            }
            case 'G':
            { //Green
                SetLedColor(2);
                break;
            }
            case 'Y':
            { //Yellow
                SetLedColor(3);
                break;
            }
            case 'B':
            { //Blue
                SetLedColor(4);
                break;
            }
            case 'M':
            { //Magenta
                SetLedColor(5);
                break;
            }
            case 'T':
            { //Turquoise
                SetLedColor(6);
                break;
            }
            case 'W':
            { //White
                SetLedColor(7);
                break;
            }
            default:
            {
                break;
            }
        } //switch(cLedColor)
        return true;
    }
    else
    {
        return false;
    }
} //parse_JSON

int main() {
    int i;
    HTS221 hts221;
    pc.baud(115200);
    
    void hts221_init(void);

    // Set LED to RED until init finishes
    SetLedColor(0x1);

    pc.printf(BLU "Hello World from AT&T Shape!\r\n\n\r");
    pc.printf(GRN "Initialize the HTS221\n\r");

    i = hts221.begin();  
    if( i ) 
        pc.printf(BLU "HTS221 Detected! (0x%02X)\n\r",i);
    else
        pc.printf(RED "HTS221 NOT DETECTED!!\n\r");

    printf("Temp  is: %0.2f F \n\r",CTOF(hts221.readTemperature()));
    printf("Humid is: %02d %%\n\r",hts221.readHumidity());
    
    sensors_init();
    read_sensors();

    // Initialize the modem
    printf(GRN "Modem initializing... will take up to 60 seconds" DEF "\r\n");
    do {
        i=mdm_init();
        if (!i) {
            pc.printf(RED "Modem initialization failed!" DEF "\n");
        }
    } while (!i);
    
    //Software init
    software_init_mdm();
 
    // Resolve URL to IP address to connect to
    resolve_mdm();

    //Create a 1ms timer tick function:
    OneMsTicker.attach(OneMsFunction, 0.001f) ;

    iTimer1Interval_ms = SENSOR_UPDATE_INTERVAL_MS;

    // Open the socket (connect to the server)
    sockopen_mdm();

    // Set LED BLUE for partial init
    SetLedColor(0x4);

    // Send and receive data perpetually
    while(1) {
        static unsigned ledOnce = 0;
        if  (bTimerExpiredFlag)
        {
            bTimerExpiredFlag = false;
            sprintf(SENSOR_DATA.Temperature, "%0.2f", CTOF(hts221.readTemperature()));
            sprintf(SENSOR_DATA.Humidity, "%02d", hts221.readHumidity());
            read_sensors(); //read available external sensors from a PMOD and the on-board motion sensor
            char modem_string[512];
            GenerateModemString(&modem_string[0]);
            printf(BLU "Sending to modem : %s" DEF "\n", modem_string); 
            sockwrite_mdm(modem_string);
            sockread_mdm(&MySocketData, 1024, 20);
            
            // If any non-zero response from server, make it GREEN one-time
            //  then the actual FLOW responses will set the color.
            if ((!ledOnce) && (MySocketData.length() > 0))
            {
                ledOnce = 1;
                SetLedColor(0x2);
            }
            
            printf(BLU "Read back : %s" DEF "\n", &MySocketData[0]);
            char * myJsonResponse;
            if (extract_JSON(&MySocketData[0], &myJsonResponse[0]))
            {
                printf(GRN "JSON : %s" DEF "\n", &myJsonResponse[0]);
                parse_JSON(&myJsonResponse[0]);
            }
            else
            {
                printf(RED "JSON : %s" DEF "\n", &myJsonResponse[0]); //most likely an incomplete JSON string
                parse_JSON(&myJsonResponse[0]); //This is risky, as the string may be corrupted
            }
        } //bTimerExpiredFlag
    } //forever loop
}
