#ifndef __PS3000ACONN_HHH___
#define  __PS3000ACONN_HHH___


#include <stdio.h>

/* Headers for Windows */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 
#include "windows.h"
#include <conio.h>
#include "ps3000aApi.h"
#else
#include <sys/types.h>
#include <string.h>

#include <libps3000a-1.0/ps3000aApi.h>
#include "linux_utils.h"
#endif


typedef enum
{
	ANALOGUE,
	DIGITAL,
	AGGREGATED,
	MIXED
}MODE;


typedef struct
{
	short DCcoupled;
	short range;
	short enabled;
}CHANNEL_SETTINGS;

typedef enum
{
	MODEL_NONE = 0,
	MODEL_PS3204A	= 0xA204,
	MODEL_PS3204B	= 0xB204,
	MODEL_PS3205A	= 0xA205,
	MODEL_PS3205B	= 0xB205,
	MODEL_PS3206A	= 0xA206,
	MODEL_PS3206B	= 0xB206,
	MODEL_PS3207A	= 0xA207,
	MODEL_PS3207B	= 0xB207,
	MODEL_PS3404A	= 0xA404,
	MODEL_PS3404B	= 0xB404,
	MODEL_PS3405A	= 0xA405,
	MODEL_PS3405B	= 0xB405,
	MODEL_PS3406A	= 0xA406,
	MODEL_PS3406B	= 0xB406,
	MODEL_PS3204MSO = 0xD204,
	MODEL_PS3205MSO = 0xD205,
	MODEL_PS3206MSO = 0xD206,
} MODEL_TYPE;

typedef enum
{
	SIGGEN_NONE = 0,
	SIGGEN_FUNCTGEN = 1,
	SIGGEN_AWG = 2
} SIGGEN_TYPE;

typedef struct tTriggerDirections
{
	PS3000A_THRESHOLD_DIRECTION channelA;
	PS3000A_THRESHOLD_DIRECTION channelB;
	PS3000A_THRESHOLD_DIRECTION channelC;
	PS3000A_THRESHOLD_DIRECTION channelD;
	PS3000A_THRESHOLD_DIRECTION ext;
	PS3000A_THRESHOLD_DIRECTION aux;
}TRIGGER_DIRECTIONS;

typedef struct tPwq
{
	PS3000A_PWQ_CONDITIONS_V2 * conditions;
	short nConditions;
	PS3000A_THRESHOLD_DIRECTION direction;
	unsigned long lower;
	unsigned long upper;
	PS3000A_PULSE_WIDTH_TYPE type;
}PWQ;

typedef struct
{
	short handle;
	MODEL_TYPE				model;
	PS3000A_RANGE			firstRange;
	PS3000A_RANGE			lastRange;
	short					channelCount;
	short					maxValue;
	short					sigGen;
	short					ETS;
	short					AWGFileSize;
	CHANNEL_SETTINGS		channelSettings [PS3000A_MAX_CHANNELS];
	short					digitalPorts;
}UNIT;

unsigned long timebase = 8;
short       oversample = 1;
BOOL      scaleVoltages = TRUE;

unsigned short inputRanges [PS3000A_MAX_RANGES] = {	10, 
	20,
	50,
	100,
	200,
	500,
	1000,
	2000,
	5000,
	10000,
	20000,
	50000};

BOOL     			g_ready = FALSE;
long long 		g_times [PS3000A_MAX_CHANNELS];
short     		g_timeUnit;
long      		g_sampleCount;
unsigned long	g_startIndex;
short			g_autoStopped;
short			g_trig = 0;
unsigned long	g_trigAt = 0;
char BlockFile[20]  = "block.txt";
char StreamFile[20] = "stream4.txt";

typedef struct tBufferInfo
{
	UNIT * unit;
	MODE mode;
	short **driverBuffers;
	short **appBuffers;
	short **driverDigBuffers;
	short **appDigBuffers;

} BUFFER_INFO;


#endif
