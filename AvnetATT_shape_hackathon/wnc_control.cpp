#include "mbed.h"
#include <cctype>
#include <string>
#include "config_me.h"
#include "SerialBuffered.h"
#include "wnc_control.h"

extern Serial pc;
extern Serial mdm;
extern string MyServerIpAddress;
extern string MySocketData;

int reinitialize_mdm(void);

enum WNC_ERR_e {
    WNC_OK =0,
    WNC_CMD_ERR = -1,
    WNC_NO_RESPONSE = -2
};

// Contains result of last call to send_wnc_cmd(..)
WNC_ERR_e WNC_MDM_ERR = WNC_OK;

// Contains the RAW WNC UART responses
static string wncStr;
static int socketOpen = 0;

void software_init_mdm(void)
{
  do
  {
    WNC_MDM_ERR = WNC_OK;
    at_init_wnc();
    if (WNC_MDM_ERR != WNC_OK)
      reinitialize_mdm();
  } while (WNC_MDM_ERR != WNC_OK); 
}

void resolve_mdm(void)
{
    do
    {
      WNC_MDM_ERR = WNC_OK;
      at_dnsresolve_wnc(MY_SERVER_URL, &MyServerIpAddress);
      if (WNC_MDM_ERR == WNC_NO_RESPONSE)
      {
        reinitialize_mdm();
        software_init_mdm();
      }
      else if (WNC_MDM_ERR == WNC_CMD_ERR)
      {
        pc.puts("Bad URL!!!!!!\r\n");
        MyServerIpAddress = "192.168.0.1";
        WNC_MDM_ERR = WNC_OK;
      }
    } while (WNC_MDM_ERR != WNC_OK);
    
    pc.printf("My Server IP: %s\r\n", MyServerIpAddress.data());
}

void sockopen_mdm(void)
{
    do
    {
      at_at_wnc();
      at_at_wnc();
      WNC_MDM_ERR = WNC_OK;
      at_sockopen_wnc(MyServerIpAddress, MY_PORT_STR);
      if (WNC_MDM_ERR == WNC_NO_RESPONSE)
      {
        reinitialize_mdm();
        software_init_mdm();
      }
      else if (WNC_MDM_ERR == WNC_CMD_ERR)
        pc.puts("Socket open fail!!!!\r\n");
      else
        socketOpen = 1;
    } while (WNC_MDM_ERR != WNC_OK);
}

void sockwrite_mdm(const char * s)
{
    if (socketOpen == 1)
    {
    do
    {
      WNC_MDM_ERR = WNC_OK;
      at_sockwrite_wnc(s);
      if (WNC_MDM_ERR == WNC_NO_RESPONSE)
      {
        reinitialize_mdm();
        software_init_mdm();
      }
      else if (WNC_MDM_ERR == WNC_CMD_ERR)
      {
        pc.puts("Socket Write fail!!!\r\n");
        // Have seen when write fails modem gets stuck in bad state, try to recover
        reinitialize_mdm();
        software_init_mdm();
      }
    } while (WNC_MDM_ERR != WNC_OK);
    }
    else
      puts("Socket is closed for write!\r\n");
}

void sockread_mdm(string * sockData, int len, int retries)
{
    if (socketOpen == 1)
    {
    do
    {
      WNC_MDM_ERR = WNC_OK;
      at_sockread_wnc(sockData, len, retries);
      if (WNC_MDM_ERR == WNC_NO_RESPONSE)
      {
        reinitialize_mdm();
        software_init_mdm();
      }
      else if (WNC_MDM_ERR == WNC_CMD_ERR)
        puts("Sock read fail!!!!\r\n");
    } while (WNC_MDM_ERR != WNC_OK);
    }
    else
      puts("Socket is closed for read\r\n");
}

void sockclose_mdm(void)
{
    do
    {
      WNC_MDM_ERR = WNC_OK;
      at_sockclose_wnc();
      // Assume close happened even if it went bad
      // going bad will result in a re-init anyways and if close
      // fails we're pretty much in bad state and not much can do
      socketOpen = 0;
      if (WNC_MDM_ERR == WNC_NO_RESPONSE)
      {
        reinitialize_mdm();
        software_init_mdm();
      }
      else if (WNC_MDM_ERR == WNC_CMD_ERR)
        puts("Sock close fail!!!\r\n");
    } while (WNC_MDM_ERR != WNC_OK);
}

/**                                                                                                                                                          
 * C++ version 0.4 char* style "itoa":                                                                                                                       
 * Written by Lukás Chmela                                                                                                                                   
 * Released under GPLv3.                                                                                                                                     
*/
 
char* itoa(int value, char* result, int base)                                                                                                          
{                                                                                                                                                        
    // check that the base if valid                                                                                                                      
    if ( base < 2 || base > 36 ) {                                                                                                                       
        *result = '\0';                                                                                                                                  
        return result;                                                                                                                                   
    }                                                                                                                                                    
 
    char* ptr = result, *ptr1 = result, tmp_char;                                                                                                        
    int tmp_value;                                                                                                                                       
 
    do {                                                                                                                                                 
        tmp_value = value;                                                                                                                               
        value /= base;                                                                                                                                   
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + (tmp_value - value * base)];                             
    } while ( value );                                                                                                                                   
 
    // Apply negative sign                                                                                                                               
    if ( tmp_value < 0 )                                                                                                                                 
    *ptr++ = '-';                                                                                                                                    
    *ptr-- = '\0';                                                                                                                                       
 
    while ( ptr1 < ptr ) {                                                                                                                               
    tmp_char = *ptr;                                                                                                                                 
    *ptr-- = *ptr1;                                                                                                                                  
    *ptr1++ = tmp_char;                                                                                                                              
    }                                                                                                                                                    
 
    return result;                                                                                                                                       
}

extern int mdm_sendAtCmdRsp(const char *cmd, const char **rsp_list, int timeout_ms, string * rsp, int * len);

// Sets a global with failure or success, assumes 1 thread all the time
int send_wnc_cmd(const char * s, string ** r, int ms_timeout)
{
  static const char * rsp_lst[] = { "OK", "ERROR", NULL };
  int len;
  
  pc.printf("Send: %s\r\n",s);
  int res = mdm_sendAtCmdRsp(s, rsp_lst, ms_timeout, &wncStr, &len);
  *r = &wncStr;   // Return a pointer to the static string
      
  if (res >= 0)
  {
      pc.puts("[");
      pc.puts(wncStr.data());
      pc.puts("]\n\r");
      if (res > 0)
      {
          if (WNC_MDM_ERR != WNC_NO_RESPONSE)
            WNC_MDM_ERR = WNC_CMD_ERR;
          return -1;
      }
      else
          return 0;
  }
  else
  {
      WNC_MDM_ERR = WNC_NO_RESPONSE;
      pc.puts("No response from WNC!\n\r");
      return -2;
  }
}

void at_at_wnc(void)
{
    string * pRespStr;
    send_wnc_cmd("AT", &pRespStr, WNC_TIMEOUT_MS); // Heartbeat?
}

void at_init_wnc(void)
{
  string * pRespStr;
  send_wnc_cmd("AT", &pRespStr, WNC_TIMEOUT_MS);             // Heartbeat?
  send_wnc_cmd("ATE1", &pRespStr, WNC_TIMEOUT_MS);           // Echo ON
  string cmd_str("AT%PDNSET=1,");
  cmd_str += MY_APN_STR;
  cmd_str += ",IP";
  send_wnc_cmd(cmd_str.data(), &pRespStr, 2*WNC_TIMEOUT_MS); // Set APN, cmd seems to take a little longer sometimes
  send_wnc_cmd("AT@INTERNET=1", &pRespStr, WNC_TIMEOUT_MS);  // Internet services enabled
  send_wnc_cmd("AT@SOCKDIAL=1", &pRespStr, WNC_TIMEOUT_MS);
}

void at_sockopen_wnc(const string & ipStr, const char * port )
{
  string * pRespStr;
  send_wnc_cmd("AT@SOCKCREAT=1", &pRespStr, WNC_TIMEOUT_MS);
  string cmd_str("AT@SOCKCONN=1,\"");
  cmd_str += ipStr;
  cmd_str += "\",";
  cmd_str += port;
  send_wnc_cmd(cmd_str.data(), &pRespStr, WNC_TIMEOUT_MS);
}

void at_sockclose_wnc(void)
{
  string * pRespStr;
  send_wnc_cmd("AT@SOCKCLOSE=1", &pRespStr, WNC_TIMEOUT_MS);
}

int at_dnsresolve_wnc(const char * s, string * ipStr)
{
  string * pRespStr;
  string str(s);
  str = "AT@DNSRESVDON=\"" + str;
  str += "\"\r\n";
  if (send_wnc_cmd(str.data(), &pRespStr, WNC_TIMEOUT_MS) == 0)
  {
    size_t pos_start = pRespStr->find(":\"") + 2;
    if (pos_start !=  string::npos)
    {
      size_t pos_end = pRespStr->rfind("\"") - 1;
      if (pos_end != string::npos)
      {
        if (pos_end > pos_start)
        {
          // Make a copy for use later (the source string is re-used)
          *ipStr = pRespStr->substr(pos_start, pos_end - pos_start + 1);
          return 1;
        }
        else
          pc.puts("URL Resolve fail, substr Err\r\n");
      }
      else
        pc.puts("URL Resolve fail, no 2nd quote\r\n");
    }
    else
      pc.puts("URL Resolve fail, no quotes\r\n");
  }
  else
    pc.puts("URL Resolve fail, WNC cmd fail\r\n");
  
  return -1;
}

void at_sockwrite_wnc(const char * s)
{
  string * pRespStr;
  char num2str[6];
  size_t sLen = strlen(s);
  if (sLen <= 99999)
  {
    string cmd_str("AT@SOCKWRITE=1,");
    itoa(sLen, num2str, 10);
    cmd_str += num2str;
    cmd_str += ",\"";
    while(*s != '\0')
    {
      itoa((int)*s++, num2str, 16);
      // Always 2-digit ascii hex:
      if (strlen(num2str) == 1)
      {
        num2str[2] = '\0';
        num2str[1] = num2str[0];
        num2str[0] = '0';
      }
      cmd_str += num2str;
    }
    cmd_str += "\"";
    send_wnc_cmd(cmd_str.data(), &pRespStr, WNC_TIMEOUT_MS);
  }
  else
    pc.puts("sockwrite Err, string to long\r\n");
}

unsigned at_sockread_wnc(string * pS, unsigned n, unsigned retries = 0)
{
  unsigned i;
  string * pRespStr;
  string cmd_str("AT@SOCKREAD=1,");
  if (n <= 1500)
  {
    char num2str[6];
    
    itoa(n, num2str, 10);
    cmd_str += num2str;
    retries += 1;
    while (retries--)
    {
      // Assuming someone is sending then calling this to receive response, invoke
      // a pause to give the response some time to come back and then also
      // between each retry.
      wait_ms(10);
      
      send_wnc_cmd(cmd_str.data(), &pRespStr, WNC_TIMEOUT_MS);
      size_t pos_start = pRespStr->find("\"")  + 1;
      size_t pos_end   = pRespStr->rfind("\"") - 1;
      i = pos_end - pos_start + 1;
      if (i > 0)
      {
        retries = 0;  // If any data found stop retrying
        string byte;
        pS->erase();
        while (pos_start < pos_end)
        {
          byte = pRespStr->substr(pos_start, 2);
          *pS += (char)strtol(byte.data(), NULL, 16);
          pos_start += 2;
        }
        return i;
      }
    }
  }
  else
    pc.puts("sockread Err, to many to read\r\n");
  
  return 0;
}
