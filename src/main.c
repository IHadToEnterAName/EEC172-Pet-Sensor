#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_nvic.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "gpio.h"
#include "utils.h"
#include "systick.h"
#include "glcdfont.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"
#include "spi.h"
#include "i2c_if.h"
#include "i2c.h"
#include "hw_timer.h"

// Common interface includes
#include "uart_if.h"
#include "uart.h"
#include "pinmux.h"
#include "timer.h"
#include "pin.h"
#include "gpio_if.h"
#include "common.h"
#include "simplelink.h"
#include "hx711.h"



volatile bool state = false;

volatile bool expired = false;

volatile bool consecutive = false;

volatile int count = 0;

volatile char buffer_char[512];

volatile int UART_cnt = 0;





static volatile uint32_t prev_hex = 0;



static volatile uint32_t buffer = 0;

#define SYSCLKFREQ 80000000ULL

// macro to convert ticks to microseconds
#define TICKS_TO_US(ticks) \
    ((((ticks) / SYSCLKFREQ) * 1000000ULL) + \
    ((((ticks) % SYSCLKFREQ) * 1000000ULL) / SYSCLKFREQ))\

// macro to convert microseconds to ticks
#define US_TO_TICKS(us) ((SYSCLKFREQ / 1000000ULL) * (us))

// systick reload value set to 40ms period
// (PERIOD_SEC) * (SYSCLKFREQ) = PERIOD_TICKS
#define SYSTICK_RELOAD_VAL 3200000UL


//#define APPLICATION_VERSION     "1.4.0"
//#define APP_NAME                "I2C Demo"
//define UART_PRINT              Report
#define FOREVER                 1
#define CONSOLE                 UARTA0_BASE
#define FAILURE                 -1
#define SUCCESS                 0
#define RETERR_IF_TRUE(condition) {if(condition) return FAILURE;}
#define RET_IF_ERR(Func)          {int iRetVal = (Func); \
                                   if (SUCCESS != iRetVal) \
                                     return  iRetVal;}



#define MAX_URI_SIZE 128
#define URI_SIZE MAX_URI_SIZE + 1


#define APPLICATION_NAME        "SSL"
#define APPLICATION_VERSION     "1.1.1.EEC.Spring2018"
#define SERVER_NAME             "a2vqra3efgtz23-ats.iot.us-east-1.amazonaws.com"
#define GOOGLE_DST_PORT         8443

#define SL_SSL_CA_CERT "/cert/rootCA.der" //starfield class2 rootca (from firefox) // <-- this one works
#define SL_SSL_PRIVATE "/cert/private.der"
#define SL_SSL_CLIENT  "/cert/client.der"


//NEED TO UPDATE THIS FOR IT TO WORK!
#define DATE                16    /* Current Date */
#define MONTH               03     /* Month 1-12 */
#define YEAR                2024  /* Current year */
#define HOUR                00    /* Time - hours */
#define MINUTE              25    /* Time - minutes */
#define SECOND              00     /* Time - seconds */

#define POSTHEADER "POST /things/cc3200/shadow HTTP/1.1\r\n"
#define GETHEADER "GET /things/cc3200/shadow HTTP/1.1\r\n"
#define HOSTHEADER "Host: a2vqra3efgtz23-ats.iot.us-east-1.amazonaws.com\r\n"
#define CHEADER "Connection: Keep-Alive\r\n"
#define CTHEADER "Content-Type: application/json; charset=utf-8\r\n"
#define CLHEADER1 "Content-Length: "
#define CLHEADER2 "\r\n\r\n"

static char DATA1[1024] =  "{\"state\": {\r\n\"desired\" : {\r\n\"status\" : \"Hello phone, message from CC3200 via AWS IoT!\"\r\n}}}\r\n\r\n";
static char DATA2[1024] =  "{\"state\": {\r\n\"desired\" : {\r\n\"Weight\" : \"Hello phone, message from CC3200 via AWS IoT!\"\r\n}}}\r\n\r\n";

#define ZERO  0x2FD00FF
#define ONE   0x2FD807F
#define TWO   0x2FD40BF
#define THREE 0x2FDC03F
#define FOUR  0x2FD20DF
#define FIVE  0x2FDA05F
#define SIX   0x2FD609F
#define SEVEN 0x2FDE01F
#define EIGHT 0x2FD10EF
#define NINE  0x2FD906F
#define LAST  0x2FDE817
#define MUTE  0x2FD08F7

#define MASTER_MODE      1
#define SPI_IF_BIT_RATE  100000
#define TR_BUFF_SIZE     100
#define BLACK           0x0000
#define BLUE            0x001F
#define GREEN           0x07E0
#define CYAN            0x07FF
#define RED             0xF800
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

// track systick counter periods elapsed
// if it is not 0, we know the transmission ended


#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif


volatile int systick_cnt = 0;


extern void (* const g_pfnVectors[])(void);

volatile unsigned long Remote_intcount;
volatile unsigned char Remote_intflag;




// an example of how you can use structs to organize your pin settings for easier maintenance
typedef struct PinSetting {
    unsigned long port;
    unsigned int pin;
} PinSetting;

static PinSetting Remote = { .port = GPIOA3_BASE, .pin = 0x80};





// Application specific status/error codes
typedef enum{
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    LAN_CONNECTION_FAILED = -0x7D0,
    INTERNET_CONNECTION_FAILED = LAN_CONNECTION_FAILED - 1,
    DEVICE_NOT_IN_STATION_MODE = INTERNET_CONNECTION_FAILED - 1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;

typedef struct
{
   /* time */
   unsigned long tm_sec;
   unsigned long tm_min;
   unsigned long tm_hour;
   /* date */
   unsigned long tm_day;
   unsigned long tm_mon;
   unsigned long tm_year;
   unsigned long tm_week_day; //not required
   unsigned long tm_year_day; //not required
   unsigned long reserved[3];
}SlDateTime;


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
volatile unsigned long  g_ulStatus = 0;//SimpleLink Status
unsigned long  g_ulPingPacketsRecv = 0; //Number of Ping Packets received
unsigned long  g_ulGatewayIP = 0; //Network Gateway IP address
unsigned char  g_ucConnectionSSID[SSID_LEN_MAX+1]; //Connection SSID
unsigned char  g_ucConnectionBSSID[BSSID_LEN_MAX]; //Connection BSSID
signed char    *g_Host = SERVER_NAME;
SlDateTime g_time;
//#if defined(ccs) || defined(gcc)
//extern void (* const g_pfnVectors[])(void);
//#endif
//#if defined(ewarm)
//extern uVectorEntry __vector_table;
//#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End: df
//*****************************************************************************


//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
static long WlanConnect();
static int set_time();
static void BoardInit(void);
static long InitializeAppVariables();
static int tls_connect();
static int connectToAccessPoint();
static int http_post(int);
static int http_get(int);
static unsigned long g_ulSamples[2];
static unsigned long g_ulFreq;
void inputInt();
void Captureinit();
void InitConsole();
const double temp = 1.0/80.0;

//Stores the pulse length
volatile uint32_t pulse=0;

//Tells the main code if the a pulse is being read at the moment
volatile static int echowait=0;

//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- Start
//*****************************************************************************


//*****************************************************************************
//
//! \brief The Function Handles WLAN Events
//!
//! \param[in]  pWlanEvent - Pointer to WLAN Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent) {
    if(!pWlanEvent) {
        return;
    }

    switch(pWlanEvent->Event) {
        case SL_WLAN_CONNECT_EVENT: {
            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);

            //
            // Information about the connected AP (like name, MAC etc) will be
            // available in 'slWlanConnectAsyncResponse_t'.
            // Applications can use it if required
            //
            //  slWlanConnectAsyncResponse_t *pEventData = NULL;
            // pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
            //

            // Copy new connection SSID and BSSID to global parameters
            memcpy(g_ucConnectionSSID,pWlanEvent->EventData.
                   STAandP2PModeWlanConnected.ssid_name,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.ssid_len);
            memcpy(g_ucConnectionBSSID,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.bssid,
                   SL_BSSID_LENGTH);

            UART_PRINT("[WLAN EVENT] STA Connected to the AP: %s , "
                       "BSSID: %x:%x:%x:%x:%x:%x\n\r",
                       g_ucConnectionSSID,g_ucConnectionBSSID[0],
                       g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                       g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                       g_ucConnectionBSSID[5]);
        }
        break;

        case SL_WLAN_DISCONNECT_EVENT: {
            slWlanConnectAsyncResponse_t*  pEventData = NULL;

            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            // If the user has initiated 'Disconnect' request,
            //'reason_code' is SL_USER_INITIATED_DISCONNECTION
            if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code) {
                UART_PRINT("[WLAN EVENT]Device disconnected from the AP: %s,"
                    "BSSID: %x:%x:%x:%x:%x:%x on application's request \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            else {
                UART_PRINT("[WLAN ERROR]Device disconnected from the AP AP: %s, "
                           "BSSID: %x:%x:%x:%x:%x:%x on an ERROR..!! \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
            memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
        }
        break;

        default: {
            UART_PRINT("[WLAN EVENT] Unexpected event [0x%x]\n\r",
                       pWlanEvent->Event);
        }
        break;
    }
}

//*****************************************************************************
//
//! \brief This function handles network events such as IP acquisition, IP
//!           leased, IP released etc.
//!
//! \param[in]  pNetAppEvent - Pointer to NetApp Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent) {
    if(!pNetAppEvent) {
        return;
    }

    switch(pNetAppEvent->Event) {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT: {
            SlIpV4AcquiredAsync_t *pEventData = NULL;

            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            //Ip Acquired Event Data
            pEventData = &pNetAppEvent->EventData.ipAcquiredV4;

            //Gateway IP address
            g_ulGatewayIP = pEventData->gateway;

            UART_PRINT("[NETAPP EVENT] IP Acquired: IP=%d.%d.%d.%d , "
                       "Gateway=%d.%d.%d.%d\n\r",
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,3),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,2),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,1),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,0),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,3),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,2),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,1),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,0));
        }
        break;

        default: {
            UART_PRINT("[NETAPP EVENT] Unexpected event [0x%x] \n\r",
                       pNetAppEvent->Event);
        }
        break;
    }
}


//*****************************************************************************
//
//! \brief This function handles HTTP server events
//!
//! \param[in]  pServerEvent - Contains the relevant event information
//! \param[in]    pServerResponse - Should be filled by the user with the
//!                                      relevant response information
//!
//! \return None
//!
//****************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent, SlHttpServerResponse_t *pHttpResponse) {
    // Unused in this application
}

//*****************************************************************************
//
//! \brief This function handles General Events
//!
//! \param[in]     pDevEvent - Pointer to General Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent) {
    if(!pDevEvent) {
        return;
    }

    //
    // Most of the general errors are not FATAL are are to be handled
    // appropriately by the application
    //
    UART_PRINT("[GENERAL EVENT] - ID=[%d] Sender=[%d]\n\n",
               pDevEvent->EventData.deviceEvent.status,
               pDevEvent->EventData.deviceEvent.sender);
}


//*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]      pSock - Pointer to Socket Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock) {
    if(!pSock) {
        return;
    }

    switch( pSock->Event ) {
        case SL_SOCKET_TX_FAILED_EVENT:
            switch( pSock->socketAsyncEvent.SockTxFailData.status) {
                case SL_ECLOSE:
                    UART_PRINT("[SOCK ERROR] - close socket (%d) operation "
                                "failed to transmit all queued packets\n\n",
                                    pSock->socketAsyncEvent.SockTxFailData.sd);
                    break;
                default:
                    UART_PRINT("[SOCK ERROR] - TX FAILED  :  socket %d , reason "
                                "(%d) \n\n",
                                pSock->socketAsyncEvent.SockTxFailData.sd, pSock->socketAsyncEvent.SockTxFailData.status);
                  break;
            }
            break;

        default:
            UART_PRINT("[SOCK EVENT] - Unexpected Event [%x0x]\n\n",pSock->Event);
          break;
    }
}


//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- End breadcrumb: s18_df
//*****************************************************************************


//*****************************************************************************
//
//! \brief This function initializes the application variables
//!
//! \param    0 on success else error code
//!
//! \return None
//!
//*****************************************************************************
static long InitializeAppVariables() {
    g_ulStatus = 0;
    g_ulGatewayIP = 0;
    g_Host = SERVER_NAME;
    memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
    memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
    return SUCCESS;
}


//*****************************************************************************
//! \brief This function puts the device in its default state. It:
//!           - Set the mode to STATION
//!           - Configures connection policy to Auto and AutoSmartConfig
//!           - Deletes all the stored profiles
//!           - Enables DHCP
//!           - Disables Scan policy
//!           - Sets Tx power to maximum
//!           - Sets power policy to normal
//!           - Unregister mDNS services
//!           - Remove all filters
//!
//! \param   none
//! \return  On success, zero is returned. On error, negative is returned
//*****************************************************************************
static long ConfigureSimpleLinkToDefaultState() {
    SlVersionFull   ver = {0};
    _WlanRxFilterOperationCommandBuff_t  RxFilterIdMask = {0};

    unsigned char ucVal = 1;
    unsigned char ucConfigOpt = 0;
    unsigned char ucConfigLen = 0;
    unsigned char ucPower = 0;

    long lRetVal = -1;
    long lMode = -1;

    lMode = sl_Start(0, 0, 0);
    ASSERT_ON_ERROR(lMode);

    // If the device is not in station-mode, try configuring it in station-mode
    if (ROLE_STA != lMode) {
        if (ROLE_AP == lMode) {
            // If the device is in AP mode, we need to wait for this event
            // before doing anything
            while(!IS_IP_ACQUIRED(g_ulStatus)) {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask();
#endif
            }
        }

        // Switch to STA role and restart
        lRetVal = sl_WlanSetMode(ROLE_STA);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Stop(0xFF);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Start(0, 0, 0);
        ASSERT_ON_ERROR(lRetVal);

        // Check if the device is in station again
        if (ROLE_STA != lRetVal) {
            // We don't want to proceed if the device is not coming up in STA-mode
            return DEVICE_NOT_IN_STATION_MODE;
        }
    }

    // Get the device's version-information
    ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    ucConfigLen = sizeof(ver);
    lRetVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt,
                                &ucConfigLen, (unsigned char *)(&ver));
    ASSERT_ON_ERROR(lRetVal);

    UART_PRINT("Host Driver Version: %s\n\r",SL_DRIVER_VERSION);
    UART_PRINT("Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n\r",
    ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3],
    ver.ChipFwAndPhyVersion.FwVersion[0],ver.ChipFwAndPhyVersion.FwVersion[1],
    ver.ChipFwAndPhyVersion.FwVersion[2],ver.ChipFwAndPhyVersion.FwVersion[3],
    ver.ChipFwAndPhyVersion.PhyVersion[0],ver.ChipFwAndPhyVersion.PhyVersion[1],
    ver.ChipFwAndPhyVersion.PhyVersion[2],ver.ChipFwAndPhyVersion.PhyVersion[3]);

    // Set connection policy to Auto + SmartConfig
    //      (Device's default connection policy)
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION,
                                SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Remove all profiles
    lRetVal = sl_WlanProfileDel(0xFF);
    ASSERT_ON_ERROR(lRetVal);



    //
    // Device in station-mode. Disconnect previous connection if any
    // The function returns 0 if 'Disconnected done', negative number if already
    // disconnected Wait for 'disconnection' event if 0 is returned, Ignore
    // other return-codes
    //
    lRetVal = sl_WlanDisconnect();
    if(0 == lRetVal) {
        // Wait
        while(IS_CONNECTED(g_ulStatus)) {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask();
#endif
        }
    }

    // Enable DHCP client
    lRetVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&ucVal);
    ASSERT_ON_ERROR(lRetVal);

    // Disable scan
    ucConfigOpt = SL_SCAN_POLICY(0);
    lRetVal = sl_WlanPolicySet(SL_POLICY_SCAN , ucConfigOpt, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Set Tx power level for station mode
    // Number between 0-15, as dB offset from max power - 0 will set max power
    ucPower = 0;
    lRetVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID,
            WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (unsigned char *)&ucPower);
    ASSERT_ON_ERROR(lRetVal);

    // Set PM policy to normal
    lRetVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Unregister mDNS services
    lRetVal = sl_NetAppMDNSUnRegisterService(0, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Remove  all 64 filters (8*8)
    memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    lRetVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                       sizeof(_WlanRxFilterOperationCommandBuff_t));
    ASSERT_ON_ERROR(lRetVal);

    lRetVal = sl_Stop(SL_STOP_TIMEOUT);
    ASSERT_ON_ERROR(lRetVal);

    InitializeAppVariables();

    return lRetVal; // Success
}


//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
//static void BoardInit(void) {
///* In case of TI-RTOS vector table is initialize by OS itself */
//#ifndef USE_TIRTOS
//  //
//  // Set vector table base
//  //
//#if defined(ccs)
//    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
//#endif
//#if defined(ewarm)
//    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
//#endif
//#endif
//    //
//    // Enable Processor
//    //
//    MAP_IntMasterEnable();
//    MAP_IntEnable(FAULT_SYSTICK);
//
//    PRCMCC3200MCUInit();
//}


//****************************************************************************
//
//! \brief Connecting to a WLAN Accesspoint
//!
//!  This function connects to the required AP (SSID_NAME) with Security
//!  parameters specified in te form of macros at the top of this file
//!
//! \param  None
//!
//! \return  0 on success else error code
//!
//! \warning    If the WLAN connection fails or we don't aquire an IP
//!            address, It will be stuck in this function forever.
//
//****************************************************************************
static long WlanConnect() {
    SlSecParams_t secParams = {0};
    long lRetVal = 0;

    secParams.Key = SECURITY_KEY;
    secParams.KeyLen = strlen(SECURITY_KEY);
    secParams.Type = SECURITY_TYPE;

    UART_PRINT("Attempting connection to access point: ");
    UART_PRINT(SSID_NAME);
    UART_PRINT("... ...");
    lRetVal = sl_WlanConnect(SSID_NAME, strlen(SSID_NAME), 0, &secParams, 0);
    ASSERT_ON_ERROR(lRetVal);

    UART_PRINT(" Connected!!!\n\r");

    // Wait for WLAN Event
    while((!IS_CONNECTED(g_ulStatus)) || (!IS_IP_ACQUIRED(g_ulStatus))) {
        // Toggle LEDs to Indicate Connection Progress
        _SlNonOsMainLoopTask();
        GPIO_IF_LedOff(MCU_IP_ALLOC_IND);
        MAP_UtilsDelay(800000);
        _SlNonOsMainLoopTask();
        GPIO_IF_LedOn(MCU_IP_ALLOC_IND);
        MAP_UtilsDelay(800000);
    }

    return SUCCESS;

}




long printErrConvenience(char * msg, long retVal) {
    UART_PRINT(msg);
    GPIO_IF_LedOn(MCU_RED_LED_GPIO);
    return retVal;
}


//*****************************************************************************
//
//! This function updates the date and time of CC3200.
//!
//! \param None
//!
//! \return
//!     0 for success, negative otherwise
//!
//*****************************************************************************

static int set_time() {
    long retVal;

    g_time.tm_day = DATE;
    g_time.tm_mon = MONTH;
    g_time.tm_year = YEAR;
    g_time.tm_sec = HOUR;
    g_time.tm_hour = MINUTE;
    g_time.tm_min = SECOND;



    retVal = sl_DevSet(SL_DEVICE_GENERAL_CONFIGURATION,
                          SL_DEVICE_GENERAL_CONFIGURATION_DATE_TIME,
                          sizeof(SlDateTime),(unsigned char *)(&g_time));

    ASSERT_ON_ERROR(retVal);
    return SUCCESS;
}

//*****************************************************************************
//
//! This function demonstrates how certificate can be used with SSL.
//! The procedure includes the following steps:
//! 1) connect to an open AP
//! 2) get the server name via a DNS request
//! 3) define all socket options and point to the CA certificate
//! 4) connect to the server via TCP
//!
//! \param None
//!
//! \return  0 on success else error code
//! \return  LED1 is turned solid in case of success
//!    LED2 is turned solid in case of failure
//!
//*******************************************

static int http_get(int iTLSSockID){
    char acSendBuff[512];
    char acRecvbuff[1460];
    char cCLLength[200];
    char buffer[1000];
    char* pcBufHeaders;
    int lRetVal = 0;

    strcpy(buffer, DATA1);

    pcBufHeaders = acSendBuff;
    strcpy(pcBufHeaders, GETHEADER);
    pcBufHeaders += strlen(GETHEADER);
    strcpy(pcBufHeaders, HOSTHEADER);
    pcBufHeaders += strlen(HOSTHEADER);
    strcpy(pcBufHeaders, CHEADER);
    pcBufHeaders += strlen(CHEADER);
    strcpy(pcBufHeaders, "\r\n\r\n");



    int testDataLength = strlen(pcBufHeaders);

    UART_PRINT(acSendBuff);

    //
    // Send the packet to the server */
    //
    lRetVal = sl_Send(iTLSSockID, acSendBuff, strlen(acSendBuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("POST failed. Error Number: %i\n\r",lRetVal);
        sl_Close(iTLSSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        return lRetVal;
    }
    lRetVal = sl_Recv(iTLSSockID, &acRecvbuff[0], sizeof(acRecvbuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("Received failed. Error Number: %i\n\r",lRetVal);
        //sl_Close(iSSLSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
           return lRetVal;
    }
    else {
        acRecvbuff[lRetVal+1] = '\0';
        UART_PRINT(acRecvbuff);
        UART_PRINT("\n\r\n\r");
    }

    return 0;
}




static int http_post(int iTLSSockID){
    char acSendBuff[512];
    char acRecvbuff[1460];
    char cCLLength[200];
    char* pcBufHeaders;
    int lRetVal = 0;

    pcBufHeaders = acSendBuff;
    strcpy(pcBufHeaders, POSTHEADER);
    pcBufHeaders += strlen(POSTHEADER);
    strcpy(pcBufHeaders, HOSTHEADER);
    pcBufHeaders += strlen(HOSTHEADER);
    strcpy(pcBufHeaders, CHEADER);
    pcBufHeaders += strlen(CHEADER);
    strcpy(pcBufHeaders, "\r\n\r\n");

    int dataLength = strlen(DATA1);

    strcpy(pcBufHeaders, CTHEADER);
    pcBufHeaders += strlen(CTHEADER);
    strcpy(pcBufHeaders, CLHEADER1);

    pcBufHeaders += strlen(CLHEADER1);
    sprintf(cCLLength, "%d", dataLength);

    strcpy(pcBufHeaders, cCLLength);
    pcBufHeaders += strlen(cCLLength);
    strcpy(pcBufHeaders, CLHEADER2);
    pcBufHeaders += strlen(CLHEADER2);

    strcpy(pcBufHeaders, DATA1);
    pcBufHeaders += strlen(DATA1);

    int testDataLength = strlen(pcBufHeaders);

    UART_PRINT(acSendBuff);


    //
    // Send the packet to the server */
    //
    lRetVal = sl_Send(iTLSSockID, acSendBuff, strlen(acSendBuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("POST failed. Error Number: %i\n\r",lRetVal);
        sl_Close(iTLSSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        return lRetVal;
    }
    lRetVal = sl_Recv(iTLSSockID, &acRecvbuff[0], sizeof(acRecvbuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("Received failed. Error Number: %i\n\r",lRetVal);
        //sl_Close(iSSLSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
           return lRetVal;
    }
    else {
        acRecvbuff[lRetVal+1] = '\0';
        UART_PRINT(acRecvbuff);
        UART_PRINT("\n\r\n\r");
    }

    return 0;
}

int connectToAccessPoint() {
    long lRetVal = -1;
    GPIO_IF_LedConfigure(LED1|LED3);

    GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);

    lRetVal = InitializeAppVariables();
    ASSERT_ON_ERROR(lRetVal);

    //
    // Following function configure the device to default state by cleaning
    // the persistent settings stored in NVMEM (viz. connection profiles &
    // policies, power policy etc)
    //
    // Applications may choose to skip this step if the developer is sure
    // that the device is in its default state at start of applicaton
    //
    // Note that all profiles and persistent settings that were done on the
    // device will be lost
    //
    lRetVal = ConfigureSimpleLinkToDefaultState();
    if(lRetVal < 0) {
      if (DEVICE_NOT_IN_STATION_MODE == lRetVal)
          UART_PRINT("Failed to configure the device in its default state \n\r");

      return lRetVal;
    }

    UART_PRINT("Device is configured in default state \n\r");

    CLR_STATUS_BIT_ALL(g_ulStatus);

    ///
    // Assumption is that the device is configured in station mode already
    // and it is in its default state
    //
    UART_PRINT("Opening sl_start\n\r");
    lRetVal = sl_Start(0, 0, 0);
    if (lRetVal < 0 || ROLE_STA != lRetVal) {
        UART_PRINT("Failed to start the device \n\r");
        return lRetVal;
    }

    UART_PRINT("Device started as STATION \n\r");

    //
    //Connecting to WLAN AP
    //
    lRetVal = WlanConnect();
    if(lRetVal < 0) {
        UART_PRINT("Failed to establish connection w/ an AP \n\r");
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        return lRetVal;
    }

    UART_PRINT("Connection established w/ AP and IP is aquired \n\r");
    return 0;
}

static int tls_connect() {
    SlSockAddrIn_t    Addr;
    int    iAddrSize;
    unsigned char    ucMethod = SL_SO_SEC_METHOD_TLSV1_2;
    unsigned int uiIP;
    //    unsigned int uiCipher = SL_SEC_MASK_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA;
    unsigned int uiCipher = SL_SEC_MASK_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA;// SL_SEC_MASK_SSL_RSA_WITH_RC4_128_SHA
// SL_SEC_MASK_SSL_RSA_WITH_RC4_128_MD5
// SL_SEC_MASK_TLS_RSA_WITH_AES_256_CBC_SHA
// SL_SEC_MASK_TLS_DHE_RSA_WITH_AES_256_CBC_SHA
// SL_SEC_MASK_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA
// SL_SEC_MASK_TLS_ECDHE_RSA_WITH_RC4_128_SHA
// SL_SEC_MASK_TLS_RSA_WITH_AES_128_CBC_SHA256
// SL_SEC_MASK_TLS_RSA_WITH_AES_256_CBC_SHA256
// SL_SEC_MASK_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256
// SL_SEC_MASK_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 // does not work (-340, handshake fails)
    long lRetVal = -1;
    int iSockID;

    lRetVal = sl_NetAppDnsGetHostByName(g_Host, strlen((const char *)g_Host),
                                    (unsigned long*)&uiIP, SL_AF_INET);

    if(lRetVal < 0) {
        return printErrConvenience("Device couldn't retrieve the host name \n\r", lRetVal);
    }

    Addr.sin_family = SL_AF_INET;
    Addr.sin_port = sl_Htons(GOOGLE_DST_PORT);
    Addr.sin_addr.s_addr = sl_Htonl(uiIP);
    iAddrSize = sizeof(SlSockAddrIn_t);
    //
    // opens a secure socket
    //
    iSockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, SL_SEC_SOCKET);
    if( iSockID < 0 ) {
        return printErrConvenience("Device unable to create secure socket \n\r", lRetVal);
    }

    //
    // configure the socket as TLS1.2
    //
    lRetVal = sl_SetSockOpt(iSockID, SL_SOL_SOCKET, SL_SO_SECMETHOD, &ucMethod,\
                               sizeof(ucMethod));
    if(lRetVal < 0) {
        return printErrConvenience("Device couldn't set socket options \n\r", lRetVal);
    }
    //
    //configure the socket as ECDHE RSA WITH AES256 CBC SHA
    //
    lRetVal = sl_SetSockOpt(iSockID, SL_SOL_SOCKET, SL_SO_SECURE_MASK, &uiCipher,\
                           sizeof(uiCipher));
    if(lRetVal < 0) {
        return printErrConvenience("Device couldn't set socket options \n\r", lRetVal);
    }



/////////////////////////////////
// START: COMMENT THIS OUT IF DISABLING SERVER VERIFICATION
    //
    //configure the socket with CA certificate - for server verification
    //
    lRetVal = sl_SetSockOpt(iSockID, SL_SOL_SOCKET, \
                           SL_SO_SECURE_FILES_CA_FILE_NAME, \
                           SL_SSL_CA_CERT, \
                           strlen(SL_SSL_CA_CERT));

    if(lRetVal < 0) {
        return printErrConvenience("Device couldn't set socket options \n\r", lRetVal);
    }
// END: COMMENT THIS OUT IF DISABLING SERVER VERIFICATION
/////////////////////////////////


    //configure the socket with Client Certificate - for server verification
    //
    lRetVal = sl_SetSockOpt(iSockID, SL_SOL_SOCKET, \
                SL_SO_SECURE_FILES_CERTIFICATE_FILE_NAME, \
                                    SL_SSL_CLIENT, \
                           strlen(SL_SSL_CLIENT));

    if(lRetVal < 0) {
        return printErrConvenience("Device couldn't set socket options \n\r", lRetVal);
    }

    //configure the socket with Private Key - for server verification
    //
    lRetVal = sl_SetSockOpt(iSockID, SL_SOL_SOCKET, \
            SL_SO_SECURE_FILES_PRIVATE_KEY_FILE_NAME, \
            SL_SSL_PRIVATE, \
                           strlen(SL_SSL_PRIVATE));

    if(lRetVal < 0) {
        return printErrConvenience("Device couldn't set socket options \n\r", lRetVal);
    }


    /* connect to the peer device - Google server */
    lRetVal = sl_Connect(iSockID, ( SlSockAddr_t *)&Addr, iAddrSize);

    if(lRetVal >= 0) {
        UART_PRINT("Device has connected to the website:");
        UART_PRINT(SERVER_NAME);
        UART_PRINT("\n\r");
    }
    else if(lRetVal == SL_ESECSNOVERIFY) {
        UART_PRINT("Device has connected to the website (UNVERIFIED):");
        UART_PRINT(SERVER_NAME);
        UART_PRINT("\n\r");
    }
    else if(lRetVal < 0) {
        UART_PRINT("Device couldn't connect to server:");
        UART_PRINT(SERVER_NAME);
        UART_PRINT("\n\r");
        return printErrConvenience("Device couldn't connect to server \n\r", lRetVal);
    }

    GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
    return iSockID;
}


static void BoardInit(void);


//static void GPIOA3IntHandler(void) {    // Remote handler
//    unsigned long ulStatus;
//
//    ulStatus = MAP_GPIOIntStatus (Remote.port, true);
//    MAP_GPIOIntClear(Remote.port, ulStatus);       // clear interrupts on GPIOA2
//    Remote_intcount++;
//    Remote_intflag=1;
//}






static inline void SysTickReset(void) {


    HWREG(NVIC_ST_CURRENT) = 1;

    // clear the global count variable
    systick_cnt = 0;
}



static void SysTickHandler(void) {

    systick_cnt++;
}


static void handler(void) {
    //state = 0;
    unsigned long ulStatus;
    ulStatus = MAP_GPIOIntStatus (Remote.port, true);
    MAP_GPIOIntClear(Remote.port, ulStatus);       // clear interrupts on GPIOA2
    Remote_intflag = 1;





}

static void
BoardInit(void) {
    /* In case of TI-RTOS vector table is initialize by OS itself */
    #ifndef USE_TIRTOS
        //
        // Set vector table base
        //
    #if defined(ccs)
        MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
    #endif
    #if defined(ewarm)
        MAP_IntVTableBaseSet((unsigned long)&__vector_table);
    #endif
    #endif
        //
        // Enable Processor
        //
        MAP_IntMasterEnable();
        MAP_IntEnable(FAULT_SYSTICK);

        PRCMCC3200MCUInit();
}

static void SysTickInit(void) {

    // configure the reset value for the systick countdown register
    MAP_SysTickPeriodSet(SYSTICK_RELOAD_VAL);

    // register interrupts on the systick module
    MAP_SysTickIntRegister(SysTickHandler);

    // enable interrupts on systick
    // (trigger SysTickHandler when countdown reaches 0)
    MAP_SysTickIntEnable();

    // enable the systick module itself
    MAP_SysTickEnable();
}

void MasterMain()
{

    MAP_SPIReset(GSPI_BASE);

    //
    // Configure SPI interface
    //
   MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVEHIGH |
                     SPI_WL_8));

    //
    // Enable SPI for communication
    //
   MAP_SPIEnable(GSPI_BASE);
}



int main() {
    unsigned long ulStatus;

    BoardInit();

    PinMuxConfig();

    PinConfigSet(PIN_62,PIN_STRENGTH_2MA|PIN_STRENGTH_4MA,PIN_TYPE_STD_PD);


    InitTerm();

    //I2C_IF_Open(I2C_MASTER_MODE_FST);



            //
            // Clearing the Terminal.
            //
    ClearTerm();
    UART_PRINT("My terminal works!\n\r");



            //
            // Display the Banner
            //
                //
            // Reset the peripheral
            //
    MAP_PRCMPeripheralReset(PRCM_GSPI);

        //#if MASTER_MODE

    MasterMain();
    Adafruit_Init();
    I2C_IF_Open(I2C_MASTER_MODE_FST);



      TimerConfigure(TIMERA2_BASE, TIMER_CFG_PERIODIC_UP);

      TimerEnable(TIMERA2_BASE,TIMER_A);

      GPIOIntEnable(GPIOA2_BASE,0x2);


      GPIOIntTypeSet(GPIOA2_BASE, 0x2,GPIO_BOTH_EDGES);

      TimerIntRegister(TIMERA2_BASE,TIMER_A,inputInt);

      GPIOIntRegister(GPIOA2_BASE,inputInt);






    //
    // Register the interrupt handlers
    //



    // clear global variables

    //Remote_intcount=0;
    Remote_intflag=0;

    // Enable SW2 and SW3 interrupts
   long weight = HX711_Tare(10);
        Report("Load cell tare: %d \n\r", weight);


    fillScreen(BLACK);
    unsigned char character = 0;
    int cnt = 0;
    long lRetVal1 = -1;






    Message("\t\t****************************************************\n\r");
    Message("\t\t\tRemote registering\n\r");
    Message("\t\t ****************************************************\n\r");
    Message("\n\n\n\r");

            int counter = 0;
            uint64_t delta;
            uint64_t delta_us;

            bool check = false;
            bool check2 = false;

            int x = 0;
            int y = 0;




                        int i = 0;

                        bool check3 = false;
                        bool check4 = true;
                        bool check5 =true;
                        bool check6 = false;
                        bool check7 = true;
//                        fillScreen(BLACK);
//                        UARTIntEnable( UARTA1_BASE, UART_INT_RX) ;
//                        UARTIntRegister(UARTA1_BASE, CheckMessage);
                        UART_PRINT("My terminal works!\n\r");

                           //Connect the CC3200 to the local access point
                        float ans = 10000000000;
                        bool change = false;
                        int weightdb = 100000000000;
                           unsigned char ucDevAddr, ucStopBit, ucLen;
                            unsigned char ucRegOffset[3] = {0x3, 0x5};
                            unsigned char aucDataBuf[6];


                          while(1) {
                              I2C_IF_Write(0x18,&ucRegOffset[0],1,0);  //48,01
                              //
                              //            //
                              //            // Read the specified length of data
                              //            //
                                      I2C_IF_Read(0x18, &aucDataBuf[0], 1);
                              //
                              //
                                    I2C_IF_Write(0x18,&ucRegOffset[1],1,0);  //
                              //
                              //                    //
                              //                    // Read the specified length of data
                              //                    //
                                     I2C_IF_Read(0x18, &aucDataBuf[1], 1);


                                     if ((aucDataBuf[0] > 20 && aucDataBuf[0] < 235) || (aucDataBuf[1] > 20 &&
                                            aucDataBuf[1] < 235)) {
                                         char    message[] = "{\"state\": {\r\n\"desired\" : {\r\n\"state\" : \"%s\"\r\n}}}\r\n\r\n";

                                         char testing[] = "Your Machine has Been Flipped";
                                         sprintf(DATA1, message, testing);
                                         Report(" checker %s\n", DATA1);
                                         long lRetVal = -1;
                                         lRetVal = connectToAccessPoint();
                                         //Set time so that encryption can be used
                                         lRetVal = set_time();
                                         if(lRetVal < 0) {
                                             UART_PRINT("Unable to set time in the device");
                                             LOOP_FOREVER();
                                         }
                                         //Connect to the website with TLS encryption
                                         lRetVal = tls_connect();
                                         if(lRetVal < 0) {
                                             ERR_PRINT(lRetVal);
                                         }
                                         http_post(lRetVal);

                                     }

                              int weightd =  getGram(15);
                              Report("%ld\n",weightd);
                              if (weightd < 25 && !check) {
                              char    message[] = "{\"state\": {\r\n\"desired\" : {\r\n\"state\" : \"%s\"\r\n}}}\r\n\r\n";

                              char testing[] = "Please refill your Bowl, Not enough food detected detected";
                              sprintf(DATA1, message, testing);
                              Report(" checker %s\n", DATA1);
                              long lRetVal = -1;
                              lRetVal = connectToAccessPoint();
                              //Set time so that encryption can be used
                              lRetVal = set_time();
                              if(lRetVal < 0) {
                                  UART_PRINT("Unable to set time in the device");
                                  LOOP_FOREVER();
                              }
                              //Connect to the website with TLS encryption
                              lRetVal = tls_connect();
                              if(lRetVal < 0) {
                                  ERR_PRINT(lRetVal);
                              }
                              http_post(lRetVal);
                              check = true;
                              }
                              if (weightd > 1500) {
                                  weightd = 0;
                              }
                              if (weightdb < 25 && weightd <25) {
                                  change = false;
                              } else if (weightd < 35 && (weightdb >= 35 || weightdb < 25)) {
                                  change = true;

                              } else if (35 <= weightd < 50 && (35 >= weightdb || weightdb >= 50)) {
                                  change = true;

                              } else if (50 <= weightd < 70 && (50 >=  weightdb || weightdb >= 70)) {
                                  change = true;

                              } else if (weightd >= 70 &&  weightdb < 70) {
                                  change = true;

                              } else {
                                  change = false;
                              }
//                              if (weightdb ==0) {
//                                  change = false;
//                              }
                              if (change) {
                                  fillRect(width()/2 - 32,height()/2 -6, width(),height(),  BLACK);
                              }
                              if (weightd < 25) {
                                  //fillRect(width()/2 - 32,height()/2 -6, width(),height(),  BLACK);
                                  drawChar(width()/2 - 10,height()/2 - 6,'0', WHITE, BLACK,4);
                                  drawChar(width()/2 + 10,height()/2 - 6,'%', WHITE, BLACK,4);

                              } else if (weightd < 35) {
                                 // fillRect(width()/2 - 32,height()/2 -6, width(),height(),  BLACK);
                                  char letter[] = "25";
                                  drawChar(width()/2 - 32 ,height()/2 - 6,letter[0], WHITE, BLACK,4);
                                  drawChar(width()/2 - 12,height()/2 - 6,letter[1], WHITE, BLACK,4);
                                  drawChar(width()/2 + 12,height()/2 -6,'%', WHITE, BLACK,4);
                                  check = false;
                                                    } else if (weightd < 50) {
                                 // fillRect(width()/2 - 32,height()/2 -6, width(),height(),  BLACK);
                                  char letter[] = "50";
                                  drawChar(width()/2 - 32 ,height()/2 - 6,letter[0], WHITE, BLACK,4);
                                  drawChar(width()/2 - 12,height()/2 - 6,letter[1], WHITE, BLACK,4);
                                  drawChar(width()/2 + 12,height()/2 -6,'%', WHITE, BLACK,4);
                                  check = false;
                              } else if (weightd < 70) {
                                 // fillRect(width()/2 - 32,height()/2 -6, width(),height(),  BLACK);
                                  char letter[] = "75";
                                  drawChar(width()/2 - 32 ,height()/2 - 6,letter[0], WHITE, BLACK,4);
                                  drawChar(width()/2 - 12,height()/2 - 6,letter[1], WHITE, BLACK,4);
                                  drawChar(width()/2 + 12,height()/2 -6,'%', WHITE, BLACK,4);
                                  check = false;
                              } else {
                                 // fillRect(width()/2 - 32,height()/2 -6, width(),height(),  BLACK);
                                  char letter[] = "100";
                                  drawChar(width()/2 - 32 ,height()/2 - 6,letter[0], WHITE, BLACK,4);
                                  drawChar(width()/2 - 12,height()/2 - 6,letter[1], WHITE, BLACK,4);
                                  drawChar(width()/2 + 12,height()/2 -6,letter[2], WHITE, BLACK,4);
                                  drawChar(width()/2 + 32,height()/2 -6,'%', WHITE, BLACK,4);
                                  check = false;
                              }
                              weightdb = weightd;
                              change = false;

                              cnt++;
                              if(echowait != 1){
                                                                      //Does the required pulse of 10uS
                                  MAP_GPIOPinWrite(GPIOA1_BASE, 0x2,0x2);
                                  UtilsDelay(266);
                                  MAP_GPIOPinWrite(GPIOA1_BASE, 0x2, 0x00);
                                                                      /*
                                 This makes the code wait for a reading to finish
                                                                        You can omit this part if you want the code to be non-blocking but
                                                                        reading is only ready when echowait=0.
                                                                      */
                                  while(echowait != 0);
                                  float percentage =0.0;
                                  int ad;
                                  float total =40.0;

                                  //Converts the counter value to cm. TempValueC = (uint32_t)(147.5 - ((75.0*3.3 *(float)ADCValues[0])) / 4096.0);
                                  //Report("distance = %dcm \n" , pulse);
                                  pulse =(uint32_t)(temp * pulse);
                                  pulse = pulse / 58;


                                  //Percentage calculatin

                                  percentage =(float)(pulse/total);
                                  percentage= percentage*100;

                                  ad=(int)percentage;

                                  //Prints out the distance measured.
                                  Report("distance = %2dcm \n\r" , pulse);
                                  Report("distance = %2dcm \n\r" , ad);

                                  Report("Percentage = %f -/- \n\r" , percentage);
                                  ans = percentage;

                                  if (ans  < 100 && ans != 0) {
                                      counter ++;
                                  } else {
                                      if (counter > 2 && check2) {

                                      char message[] = "{\"state\": {\r\n\"desired\" : {\r\n\"state\" : \"%s\"\r\n}}}\r\n\r\n";

                                      char testing[] = "Your cat has stopped eating";
                                      sprintf(DATA1, message, testing);
                                      Report(" checker %s\n", DATA1);
                                      long lRetVal = -1;
                                      lRetVal = connectToAccessPoint();
                                      //Set time so that encryption can be used
                                      lRetVal = set_time();
                                      if(lRetVal < 0) {
                                          UART_PRINT("Unable to set time in the device");
                                          LOOP_FOREVER();
                                      }
                                      //Connect to the website with TLS encryption
                                      lRetVal = tls_connect();
                                      if(lRetVal < 0) {
                                          ERR_PRINT(lRetVal);
                                      }
                                      http_post(lRetVal);
                                      check2 = false;

                                  }


                                      counter = 0;

                                  }

                              }
                              //wait about 10ms until the next reading.
                              //UtilsDelay(400000);
                              UtilsDelay(8000000);


                              if (counter >  2) {
                                  //post
                                  cnt = 0;
                                  if (!check2) {
                                  char message[] = "{\"state\": {\r\n\"desired\" : {\r\n\"state\" : \"%s\"\r\n}}}\r\n\r\n";

                                  char testing[] = "Your cat is now eating";
                                  sprintf(DATA1, message, testing);
                                  Report(" checker %s\n", DATA1);
                                  long lRetVal = -1;
                                  lRetVal = connectToAccessPoint();
                                  //Set time so that encryption can be used
                                  lRetVal = set_time();
                                  if(lRetVal < 0) {
                                      UART_PRINT("Unable to set time in the device");
                                      LOOP_FOREVER();
                                  }
                                  //Connect to the website with TLS encryption
                                  lRetVal = tls_connect();
                                  if(lRetVal < 0) {
                                      ERR_PRINT(lRetVal);
                                  }
                                  http_post(lRetVal);
                                  }
                                  check2 = true;


                              }


                              if (cnt % 30 == 0 && !check2) {
                                  char message[] = "{\"state\": {\r\n\"desired\" : {\r\n\"state\" : \"%s\"\r\n}}}\r\n\r\n";

                                  char testing[] = "Your pet hasnt eaten in a long time";
                                  sprintf(DATA1, message, testing);
                                  Report(" checker %s\n", DATA1);
                                  long lRetVal = -1;
                                  lRetVal = connectToAccessPoint();
                                  //Set time so that encryption can be used
                                  lRetVal = set_time();
                                  if(lRetVal < 0) {
                                      UART_PRINT("Unable to set time in the device");
                                      LOOP_FOREVER();
                                  }
                                  //Connect to the website with TLS encryption
                                  lRetVal = tls_connect();
                                  if(lRetVal < 0) {
                                      ERR_PRINT(lRetVal);
                                  }
                                  http_post(lRetVal);
                              }






                  }

              }

void inputInt(){
         //Clear interrupt flag. Since we only enabled on this is enough
           GPIOIntClear(GPIOA2_BASE,0x2);

         /*
           If it's a rising edge then set he timer to 0
           It's in periodic mode so it was in some random value
         */
         if (GPIOPinRead(GPIOA2_BASE,0x2) == 2){
           HWREG(TIMERA2_BASE + TIMER_O_TAV ) = 0; //Loads value 0 into the timer.

            // TimerLoadSet(TIMERA2_BASE,TIMER_A,0xFFFFFFFF);
            long ad = TimerLoadGet(TIMERA2_BASE,TIMER_A);
           //Report("load = %dcm \n\r" , ad);
           TimerEnable(TIMERA2_BASE,TIMER_A);
           echowait=1;
         }
         /*
           If it's a falling edge that was detected, then get the value of the counter
         */
         else{
           pulse = TimerValueGet(TIMERA2_BASE,TIMER_A);
           long af = GPIOPinRead(GPIOA2_BASE,0x2);
           //Report("pin = %dcm \n\r" , af);
          // Report("distance = %dcm \n\r" , pulse);//record value
           TimerDisable(TIMERA2_BASE,TIMER_A);
           echowait=0;
         }
       }

