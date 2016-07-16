
#ifndef __WNC_CONTROL_H_
#define __WNC_CONTROL_H_

static const unsigned WNC_TIMEOUT_MS = 5000;

// Core function that sends data to the WNC UART
extern int send_wnc_cmd(const char * s, string ** r, int ms_timeout);

// Low level command functions
extern void at_init_wnc(void);
extern void at_sockopen_wnc(const string & ipStr, const char * port );
extern void at_sockclose_wnc(void);
extern int at_dnsresolve_wnc(const char * s, string * ipStr);
extern void at_sockwrite_wnc(const char * s);
extern unsigned at_sockread_wnc(string * pS, unsigned n, unsigned retries);
extern void at_at_wnc(void);

// High level functions that attempt to correct for things going bad with the WNC
extern void software_init_mdm(void);
extern void resolve_mdm(void);
extern void sockopen_mdm(void);
extern void sockwrite_mdm(const char * s);
extern void sockread_mdm(string * sockData, int len, int retries);
extern void sockclose_mdm(void);

#endif


