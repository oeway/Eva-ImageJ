/****************************************************************************
 *
 * Filename:    ps3000aApi.h
 * Copyright:   Pico Technology Limited 2010
 * Author:      MAS
 * Description:
 *
 * This header defines the interface to driver routines for the 
 *	PicoScope3000a series of PC Oscilloscopes.
 *
 ****************************************************************************/
#ifndef __PS3000AAPI_H__
#define __PS3000AAPI_H__

#include "PicoStatus.h"

#ifdef __cplusplus
	#define PREF0 extern "C"
	#define TYPE_ENUM
#else
	#define PREF0
	#define TYPE_ENUM enum
#endif

#ifdef WIN32
	typedef unsigned __int64 u_int64_t;
	#ifdef PREF1
	  #undef PREF1
	#endif
	#ifdef PREF2
	  #undef PREF2
	#endif
	#ifdef PREF3
	  #undef PREF3
	#endif
	/*	If you are dynamically linking PS3000A.DLL into your project #define DYNLINK here
	 */
	#ifdef DYNLINK
	  #define PREF1 typedef
		#define PREF2
		#define PREF3(x) (__stdcall *x)
	#else
	  #define PREF1
		#ifdef _USRDLL
			#define PREF2 __declspec(dllexport) __stdcall
		#else
			#define PREF2 __declspec(dllimport) __stdcall
		#endif
	  #define PREF3(x) x
	#endif
#else
	/* Define a 64-bit integer type */
	#include <stdint.h>
	typedef int64_t __int64;

	#ifdef DYNLINK
		#define PREF1 typedef
		#define PREF2
		#define PREF3(x) (*x)
	#else
		#ifdef _USRDLL
			#define PREF1 __attribute__((visibility("default")))
		#else
			#define PREF1
		#endif
		#define PREF2
		#define PREF3(x) x
	#endif
	#define __stdcall
#endif

#define PS3000A_MAX_OVERSAMPLE	256

/* Depending on the adc; oversample (collect multiple readings at each time) by up to 256.
 * the results are therefore ALWAYS scaled up to 16-bits, even if
 * oversampling is not used.
 *
 * The maximum and minimum values returned are therefore as follows:
 */

#define PS3207A_MAX_ETS_CYCLES	500
#define PS3207A_MAX_INTERLEAVE	 40

#define PS3206A_MAX_ETS_CYCLES	500
#define PS3206A_MAX_INTERLEAVE	 40
#define PS3206MSO_MAX_INTERLEAVE 80

#define PS3205A_MAX_ETS_CYCLES	250
#define PS3205A_MAX_INTERLEAVE	 20
#define PS3205MSO_MAX_INTERLEAVE 40

#define PS3204A_MAX_ETS_CYCLES	125
#define PS3204A_MAX_INTERLEAVE	 10
#define PS3204MSO_MAX_INTERLEAVE 20

#define PS3000A_EXT_MAX_VALUE  32767
#define PS3000A_EXT_MIN_VALUE -32767

#define MIN_SIG_GEN_FREQ 0.0f
#define MAX_SIG_GEN_FREQ 20000000.0f

#define PS3207B_MAX_SIG_GEN_BUFFER_SIZE 32768
#define PS3206B_MAX_SIG_GEN_BUFFER_SIZE 16384
#define MAX_SIG_GEN_BUFFER_SIZE 8192
#define MIN_SIG_GEN_BUFFER_SIZE 1
#define MIN_DWELL_COUNT				3
#define MAX_SWEEPS_SHOTS				((1 << 30) - 1)

#define MAX_ANALOGUE_OFFSET_50MV_200MV	 0.250f
#define MIN_ANALOGUE_OFFSET_50MV_200MV	-0.250f
#define MAX_ANALOGUE_OFFSET_500MV_2V		 2.500f
#define MIN_ANALOGUE_OFFSET_500MV_2V		-2.500f
#define MAX_ANALOGUE_OFFSET_5V_20V			 20.f
#define MIN_ANALOGUE_OFFSET_5V_20V			-20.f

#define PS3000A_MAX_LOGIC_LEVEL			32767

#define PS3000A_SHOT_SWEEP_TRIGGER_CONTINUOUS_RUN 0xFFFFFFFF

typedef enum	enPS3000ABandwidthLimiter
{
	PS3000A_BW_FULL,
	PS3000A_BW_20MHZ,
} PS3000A_BANDWIDTH_LIMITER;

typedef enum enPS3000AChannelBufferIndex
{
	PS3000A_CHANNEL_A_MAX,
	PS3000A_CHANNEL_A_MIN,
	PS3000A_CHANNEL_B_MAX,
	PS3000A_CHANNEL_B_MIN,
	PS3000A_CHANNEL_C_MAX,
	PS3000A_CHANNEL_C_MIN,
	PS3000A_CHANNEL_D_MAX,
	PS3000A_CHANNEL_D_MIN,
	PS3000A_MAX_CHANNEL_BUFFERS
} PS3000A_CHANNEL_BUFFER_INDEX;

typedef enum enPS3000AChannel
{
	PS3000A_CHANNEL_A,
	PS3000A_CHANNEL_B,
	PS3000A_CHANNEL_C,
	PS3000A_CHANNEL_D,
	PS3000A_EXTERNAL,	
	PS3000A_MAX_CHANNELS = PS3000A_EXTERNAL,
	PS3000A_TRIGGER_AUX,
	PS3000A_MAX_TRIGGER_SOURCES
}	PS3000A_CHANNEL;

typedef enum enPS3000DigitalPort
{
	PS3000A_DIGITAL_PORT0 = 0x80, // digital channel 0 - 7
	PS3000A_DIGITAL_PORT1,			 // digital channel 8 - 15
	PS3000A_DIGITAL_PORT2,			 // digital channel 16 - 23
	PS3000A_DIGITAL_PORT3,			 // digital channel 24 - 31
	PS3000A_MAX_DIGITAL_PORTS = (PS3000A_DIGITAL_PORT3 - PS3000A_DIGITAL_PORT0) + 1
}	PS3000A_DIGITAL_PORT;

typedef enum enPS3000ADigitalChannel
{
	PS3000A_DIGITAL_CHANNEL_0,
	PS3000A_DIGITAL_CHANNEL_1,
	PS3000A_DIGITAL_CHANNEL_2,
	PS3000A_DIGITAL_CHANNEL_3,
	PS3000A_DIGITAL_CHANNEL_4,
	PS3000A_DIGITAL_CHANNEL_5,
	PS3000A_DIGITAL_CHANNEL_6,
	PS3000A_DIGITAL_CHANNEL_7,
	PS3000A_DIGITAL_CHANNEL_8,
	PS3000A_DIGITAL_CHANNEL_9,
	PS3000A_DIGITAL_CHANNEL_10,
	PS3000A_DIGITAL_CHANNEL_11,
	PS3000A_DIGITAL_CHANNEL_12,
	PS3000A_DIGITAL_CHANNEL_13,
	PS3000A_DIGITAL_CHANNEL_14,
	PS3000A_DIGITAL_CHANNEL_15,
	PS3000A_DIGITAL_CHANNEL_16,
	PS3000A_DIGITAL_CHANNEL_17,
	PS3000A_DIGITAL_CHANNEL_18,
	PS3000A_DIGITAL_CHANNEL_19,
	PS3000A_DIGITAL_CHANNEL_20,
	PS3000A_DIGITAL_CHANNEL_21,
	PS3000A_DIGITAL_CHANNEL_22,
	PS3000A_DIGITAL_CHANNEL_23,
	PS3000A_DIGITAL_CHANNEL_24,
	PS3000A_DIGITAL_CHANNEL_25,
	PS3000A_DIGITAL_CHANNEL_26,
	PS3000A_DIGITAL_CHANNEL_27,
	PS3000A_DIGITAL_CHANNEL_28,
	PS3000A_DIGITAL_CHANNEL_29,
	PS3000A_DIGITAL_CHANNEL_30,
	PS3000A_DIGITAL_CHANNEL_31,
	PS3000A_MAX_DIGITAL_CHANNELS
} PS3000A_DIGITAL_CHANNEL;

typedef enum enPS3000ARange
{
	PS3000A_10MV,
	PS3000A_20MV,
	PS3000A_50MV,
	PS3000A_100MV,
	PS3000A_200MV,
	PS3000A_500MV,
	PS3000A_1V,
	PS3000A_2V,
	PS3000A_5V,
	PS3000A_10V,
	PS3000A_20V,
	PS3000A_50V,
	PS3000A_MAX_RANGES,
} PS3000A_RANGE;

typedef enum enPS3000ACoupling
{
	PS3000A_AC,
	PS3000A_DC
} PS3000A_COUPLING;

typedef enum enPS3000AChannelInfo
{
	PS3000A_CI_RANGES,
} PS3000A_CHANNEL_INFO;

typedef enum enPS3000AEtsMode
  {
  PS3000A_ETS_OFF,             // ETS disabled
  PS3000A_ETS_FAST,
	PS3000A_ETS_SLOW,
  PS3000A_ETS_MODES_MAX
  }	PS3000A_ETS_MODE;

typedef enum enPS3000ATimeUnits
  {
  PS3000A_FS,
  PS3000A_PS,
  PS3000A_NS,
  PS3000A_US,
  PS3000A_MS,
  PS3000A_S,
  PS3000A_MAX_TIME_UNITS,
  }	PS3000A_TIME_UNITS;

typedef enum enPS3000ASweepType
{
	PS3000A_UP,
	PS3000A_DOWN,
	PS3000A_UPDOWN,
	PS3000A_DOWNUP,
	PS3000A_MAX_SWEEP_TYPES
} PS3000A_SWEEP_TYPE;

typedef enum enPS3000AWaveType
{
	PS3000A_SINE,
	PS3000A_SQUARE,
	PS3000A_TRIANGLE,
	PS3000A_RAMP_UP,
	PS3000A_RAMP_DOWN,
	PS3000A_SINC,
	PS3000A_GAUSSIAN,
	PS3000A_HALF_SINE,
	PS3000A_DC_VOLTAGE,
	PS3000A_MAX_WAVE_TYPES
} PS3000A_WAVE_TYPE;

typedef enum enPS3000AExtraOperations
{
	PS3000A_ES_OFF,
	PS3000A_WHITENOISE,
	PS3000A_PRBS // Pseudo-Random Bit Stream 
} PS3000A_EXTRA_OPERATIONS;


#define PS3000A_SINE_MAX_FREQUENCY				1000000.f
#define PS3000A_SQUARE_MAX_FREQUENCY			1000000.f
#define PS3000A_TRIANGLE_MAX_FREQUENCY		1000000.f
#define PS3000A_SINC_MAX_FREQUENCY				1000000.f
#define PS3000A_RAMP_MAX_FREQUENCY				1000000.f
#define PS3000A_HALF_SINE_MAX_FREQUENCY		1000000.f
#define PS3000A_GAUSSIAN_MAX_FREQUENCY		1000000.f
#define PS3000A_PRBS_MAX_FREQUENCY				1000000.f
#define PS3000A_PRBS_MIN_FREQUENCY					 0.03f
#define PS3000A_MIN_FREQUENCY			  				 0.03f

typedef enum enPS3000ASigGenTrigType
{
	PS3000A_SIGGEN_RISING,
	PS3000A_SIGGEN_FALLING,
	PS3000A_SIGGEN_GATE_HIGH,
	PS3000A_SIGGEN_GATE_LOW
} PS3000A_SIGGEN_TRIG_TYPE;

typedef enum enPS3000ASigGenTrigSource
{
	PS3000A_SIGGEN_NONE,
	PS3000A_SIGGEN_SCOPE_TRIG,
	PS3000A_SIGGEN_AUX_IN,
	PS3000A_SIGGEN_EXT_IN,
	PS3000A_SIGGEN_SOFT_TRIG
} PS3000A_SIGGEN_TRIG_SOURCE;

typedef enum enPS3000AIndexMode
{
	PS3000A_SINGLE,
	PS3000A_DUAL,
	PS3000A_QUAD,
	PS3000A_MAX_INDEX_MODES
} PS3000A_INDEX_MODE;

typedef enum enPS3000A_ThresholdMode
{
	PS3000A_LEVEL,
	PS3000A_WINDOW
} PS3000A_THRESHOLD_MODE;

typedef enum enPS3000AThresholdDirection
{
	PS3000A_ABOVE, //using upper threshold
	PS3000A_BELOW, 
	PS3000A_RISING, // using upper threshold
	PS3000A_FALLING, // using upper threshold
	PS3000A_RISING_OR_FALLING, // using both threshold
	PS3000A_ABOVE_LOWER, // using lower threshold
	PS3000A_BELOW_LOWER, // using lower threshold
	PS3000A_RISING_LOWER,			 // using upper threshold
	PS3000A_FALLING_LOWER,		 // using upper threshold

	// Windowing using both thresholds
	PS3000A_INSIDE = PS3000A_ABOVE, 
	PS3000A_OUTSIDE = PS3000A_BELOW,
	PS3000A_ENTER = PS3000A_RISING, 
	PS3000A_EXIT = PS3000A_FALLING, 
	PS3000A_ENTER_OR_EXIT = PS3000A_RISING_OR_FALLING,
	PS3000A_POSITIVE_RUNT = 9,
  PS3000A_NEGATIVE_RUNT,

	// no trigger set
  PS3000A_NONE = PS3000A_RISING 
} PS3000A_THRESHOLD_DIRECTION;

typedef enum enPS3000ADigitalDirection
{
	PS3000A_DIGITAL_DONT_CARE,
	PS3000A_DIGITAL_DIRECTION_LOW,	
	PS3000A_DIGITAL_DIRECTION_HIGH,
	PS3000A_DIGITAL_DIRECTION_RISING,
	PS3000A_DIGITAL_DIRECTION_FALLING,
	PS3000A_DIGITAL_DIRECTION_RISING_OR_FALLING,
	PS3000A_DIGITAL_MAX_DIRECTION
} PS3000A_DIGITAL_DIRECTION;

typedef enum enPS3000ATriggerState
{
  PS3000A_CONDITION_DONT_CARE,
  PS3000A_CONDITION_TRUE,
  PS3000A_CONDITION_FALSE,
	PS3000A_CONDITION_MAX
} PS3000A_TRIGGER_STATE;

#pragma pack(1)
typedef struct tPS3000ATriggerConditions
{
  PS3000A_TRIGGER_STATE channelA;
  PS3000A_TRIGGER_STATE channelB;
  PS3000A_TRIGGER_STATE channelC;
  PS3000A_TRIGGER_STATE channelD;
  PS3000A_TRIGGER_STATE external;
  PS3000A_TRIGGER_STATE aux;
	PS3000A_TRIGGER_STATE pulseWidthQualifier;
} PS3000A_TRIGGER_CONDITIONS;
#pragma pack()

#pragma pack(1)
typedef struct tPS3000ATriggerConditionsV2
{
  PS3000A_TRIGGER_STATE channelA;
  PS3000A_TRIGGER_STATE channelB;
  PS3000A_TRIGGER_STATE channelC;
  PS3000A_TRIGGER_STATE channelD;
  PS3000A_TRIGGER_STATE external;
  PS3000A_TRIGGER_STATE aux;
	PS3000A_TRIGGER_STATE pulseWidthQualifier;
	PS3000A_TRIGGER_STATE digital;
} PS3000A_TRIGGER_CONDITIONS_V2;
#pragma pack()

#pragma pack(1)
typedef struct tPS3000APwqConditions
{
  PS3000A_TRIGGER_STATE channelA;
  PS3000A_TRIGGER_STATE channelB;
  PS3000A_TRIGGER_STATE channelC;
  PS3000A_TRIGGER_STATE channelD;
  PS3000A_TRIGGER_STATE external;
  PS3000A_TRIGGER_STATE aux;
} PS3000A_PWQ_CONDITIONS;
#pragma pack()

#pragma pack(1)
typedef struct tPS3000APwqConditionsV2
{
  PS3000A_TRIGGER_STATE channelA;
  PS3000A_TRIGGER_STATE channelB;
  PS3000A_TRIGGER_STATE channelC;
  PS3000A_TRIGGER_STATE channelD;
  PS3000A_TRIGGER_STATE external;
  PS3000A_TRIGGER_STATE aux;
	PS3000A_TRIGGER_STATE digital;
} PS3000A_PWQ_CONDITIONS_V2;
#pragma pack()

#pragma pack(1)
typedef struct tPS3000ADigitalChannelDirections
{
	PS3000A_DIGITAL_CHANNEL channel;
	PS3000A_DIGITAL_DIRECTION direction;
} PS3000A_DIGITAL_CHANNEL_DIRECTIONS;
#pragma pack()

#pragma pack(1)
typedef struct tPS3000ATriggerChannelProperties
{
  short										thresholdUpper;
	unsigned short					thresholdUpperHysteresis; 
  short										thresholdLower;
	unsigned short					thresholdLowerHysteresis;
	PS3000A_CHANNEL					channel;
  PS3000A_THRESHOLD_MODE	thresholdMode;
} PS3000A_TRIGGER_CHANNEL_PROPERTIES;
#pragma pack()
	
typedef enum enPS3000ARatioMode
{
	PS3000A_RATIO_MODE_NONE,
	PS3000A_RATIO_MODE_AGGREGATE = 1,	
	PS3000A_RATIO_MODE_DECIMATE = 2,
	PS3000A_RATIO_MODE_AVERAGE = 4,
} PS3000A_RATIO_MODE;

typedef enum enPS3000APulseWidthType
{
	PS3000A_PW_TYPE_NONE,
  PS3000A_PW_TYPE_LESS_THAN,
	PS3000A_PW_TYPE_GREATER_THAN,
	PS3000A_PW_TYPE_IN_RANGE,
	PS3000A_PW_TYPE_OUT_OF_RANGE
} PS3000A_PULSE_WIDTH_TYPE;

typedef enum enPS3000AHoldOffType
{
	PS3000A_TIME,
	PS3000A_EVENT,
	PS3000A_MAX_HOLDOFF_TYPE
} PS3000A_HOLDOFF_TYPE;

typedef void (__stdcall *ps3000aBlockReady)
	(
		short											handle,
		PICO_STATUS								status,
		void										*	pParameter
	); 

typedef void (__stdcall *ps3000aStreamingReady)
	(
		short    									handle,
		long     									noOfSamples,
		unsigned long							startIndex,
		short    									overflow,
		unsigned long							triggerAt,
		short    									triggered,
		short    									autoStop,
		void										*	pParameter
	); 

typedef void (__stdcall *ps3000aDataReady)
	(
		short    									handle,
		PICO_STATUS								status,
		unsigned long     				noOfSamples,
		short    									overflow,
		void										*	pParameter
	); 

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aOpenUnit) 
  (
	  short											* handle,
		char											* serial
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aOpenUnitAsync)
  (
	  short											* status,
		char											* serial
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aOpenUnitProgress)
	(
	  short 										* handle,
	  short 										* progressPercent,
	  short 										* complete
	); 

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetUnitInfo)
 	(
	  short     								  handle, 
	  char      								* string,
	  short     								  stringLength,
	  short     								* requiredSize,
	  PICO_INFO 								  info
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aFlashLed)
	(
	  short 											handle,
		short 											start
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aCloseUnit)
	(
	  short												handle
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aMemorySegments)
	(
	  short												handle,
		unsigned short							nSegments,
		long											* nMaxSamples
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetChannel)
 	(
	  short												handle,
		PS3000A_CHANNEL							channel,
	  short												enabled,
	  PS3000A_COUPLING						type, 
		PS3000A_RANGE								range,
		float												analogOffset
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetDigitalPort)
 	(
	  short												handle,
		PS3000A_DIGITAL_PORT				port,
	  short												enabled,
		short									      logicLevel
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetBandwidthFilter)
 	(
	  short												handle,
	  PS3000A_CHANNEL							channel,
	  PS3000A_BANDWIDTH_LIMITER		bandwidth
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetNoOfCaptures) 
	(
	short handle,
	unsigned short nCaptures
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetTimebase)
	(
	   short											handle,
	   unsigned long							timebase,
	   long												noSamples,
	   long											* timeIntervalNanoseconds,
	   short											oversample,
		 long											* maxSamples,
		 unsigned short							segmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetTimebase2)
	(
	   short											handle,
	   unsigned long							timebase,
	   long												noSamples,
	   float										* timeIntervalNanoseconds,
	   short											oversample,
		 long											* maxSamples,
		 unsigned short							segmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetSigGenArbitrary)
	(
	 	short												handle,
	 	long												offsetVoltage,
	 	unsigned long								pkToPk,
	 	unsigned long								startDeltaPhase,
	 	unsigned long								stopDeltaPhase,
	 	unsigned long								deltaPhaseIncrement, 
	 	unsigned long								dwellCount,
	 	short											*	arbitraryWaveform, 
	 	long												arbitraryWaveformSize,
		PS3000A_SWEEP_TYPE					sweepType,
		PS3000A_EXTRA_OPERATIONS		operation,
		PS3000A_INDEX_MODE					indexMode,
		unsigned long								shots,
		unsigned long								sweeps,
		PS3000A_SIGGEN_TRIG_TYPE		triggerType,
		PS3000A_SIGGEN_TRIG_SOURCE	triggerSource,
		short												extInThreshold
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3(ps3000aSetSigGenBuiltIn)
	(
		short												handle,
		long												offsetVoltage,
		unsigned long								pkToPk,
		short												waveType,
		float												startFrequency,
		float												stopFrequency,
		float												increment,
		float												dwellTime,
		PS3000A_SWEEP_TYPE					sweepType,
		PS3000A_EXTRA_OPERATIONS		operation,
		unsigned long								shots,
		unsigned long								sweeps,
		PS3000A_SIGGEN_TRIG_TYPE		triggerType,
		PS3000A_SIGGEN_TRIG_SOURCE	triggerSource,
		short												extInThreshold
		);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSigGenSoftwareControl)
	(
		short												handle,
		short												state
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetEts)
  (
		short												handle,
		PS3000A_ETS_MODE						mode,
		short												etsCycles,
		short												etsInterleave,
		long											* sampleTimePicoseconds
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetSimpleTrigger)
	(
		short handle,
		short enable,
		PS3000A_CHANNEL source,
		short threshold,
		PS3000A_THRESHOLD_DIRECTION direction,
		unsigned long delay,
		short autoTrigger_ms
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetTriggerDigitalPortProperties)
(
	short handle,
	PS3000A_DIGITAL_CHANNEL_DIRECTIONS * directions,
	short															   nDirections
);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetTriggerChannelProperties)
	(
		short																	handle,
		PS3000A_TRIGGER_CHANNEL_PROPERTIES	*	channelProperties,
		short																	nChannelProperties,
		short																	auxOutputEnable,
		long																	autoTriggerMilliseconds
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetTriggerChannelConditions)
	(
		short													handle,
		PS3000A_TRIGGER_CONDITIONS	*	conditions,
		short													nConditions
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetTriggerChannelConditionsV2)
	(
		short														handle,
		PS3000A_TRIGGER_CONDITIONS_V2	*	conditions,
		short														nConditions
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetTriggerChannelDirections)
	(
		short													handle,
		PS3000A_THRESHOLD_DIRECTION		channelA,
		PS3000A_THRESHOLD_DIRECTION		channelB,
		PS3000A_THRESHOLD_DIRECTION		channelC,
		PS3000A_THRESHOLD_DIRECTION		channelD,
		PS3000A_THRESHOLD_DIRECTION		ext,
		PS3000A_THRESHOLD_DIRECTION		aux
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetTriggerDelay)
	(
		short									handle,
		unsigned long					delay
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetPulseWidthQualifier)
	(
		short													handle,
		PS3000A_PWQ_CONDITIONS			*	conditions,
		short													nConditions,
		PS3000A_THRESHOLD_DIRECTION		direction,
		unsigned long									lower,
		unsigned long									upper,
		PS3000A_PULSE_WIDTH_TYPE			type
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetPulseWidthQualifierV2)
	(
		short													handle,
		PS3000A_PWQ_CONDITIONS_V2		*	conditions,
		short													nConditions,
		PS3000A_THRESHOLD_DIRECTION		direction,
		unsigned long									lower,
		unsigned long									upper,
		PS3000A_PULSE_WIDTH_TYPE			type
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aIsTriggerOrPulseWidthQualifierEnabled)
	(
		short 								handle,
		short 							* triggerEnabled,
		short 							* pulseWidthQualifierEnabled
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetTriggerTimeOffset)
	(
		short									handle,
		unsigned long 			* timeUpper,
		unsigned long 			* timeLower,
		PS3000A_TIME_UNITS	*	timeUnits,
		unsigned short				segmentIndex	
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetTriggerTimeOffset64)
	(
		short									handle,
		__int64							* time,
		PS3000A_TIME_UNITS	*	timeUnits,
		unsigned short				segmentIndex	
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetValuesTriggerTimeOffsetBulk)
	(
	  short									handle,
		unsigned long				*	timesUpper,
		unsigned long				* timesLower,
		PS3000A_TIME_UNITS	*	timeUnits,
		unsigned short				fromSegmentIndex,
		unsigned short				toSegmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetValuesTriggerTimeOffsetBulk64)
	(
	  short									handle,
		__int64							*	times,
		PS3000A_TIME_UNITS	*	timeUnits,
		unsigned short				fromSegmentIndex,
		unsigned short				toSegmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetNoOfCaptures)
	(
	  short								handle,
		unsigned long			*	nCaptures
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetNoOfProcessedCaptures)
	(
	  short								handle,
		unsigned long			*	nProcessedCaptures
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetDataBuffer)
(
   short								 handle,
	 PS3000A_CHANNEL 			 channelOrPort,
	 short							*  buffer,
   long									 bufferLth,
	 unsigned short				 segmentIndex,
	 PS3000A_RATIO_MODE		 mode
);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetDataBuffers)
(
   short								 handle,
	 PS3000A_CHANNEL 			 channelOrPort,
	 short							 * bufferMax,
	 short							 * bufferMin,
   long									 bufferLth,
	 unsigned short				 segmentIndex,
	 PS3000A_RATIO_MODE		 mode
);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetEtsTimeBuffer)
(
   short									handle,
	 __int64 *							buffer,
	 long										bufferLth
);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aSetEtsTimeBuffers)
(
   short									handle,
	 unsigned long				* timeUpper,
	 unsigned long				* timeLower,
	 long										bufferLth
);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aIsReady)
	(
		short handle,
		short * ready
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aRunBlock)
	(
		short									handle,
		long									noOfPreTriggerSamples,
		long									noOfPostTriggerSamples,
		unsigned long					timebase,
		short									oversample,
		long								* timeIndisposedMs,
		unsigned short				segmentIndex,
		ps3000aBlockReady			lpReady,
		void								* pParameter
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aRunStreaming)
  (
	  short									handle,
		unsigned long				* sampleInterval,	
		PS3000A_TIME_UNITS		sampleIntervalTimeUnits,
	  unsigned long					maxPreTriggerSamples,
	  unsigned long					maxPostPreTriggerSamples,
		short									autoStop,
		unsigned long					downSampleRatio,
		PS3000A_RATIO_MODE		downSampleRatioMode,
    unsigned long					overviewBufferSize
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetStreamingLatestValues)
  (
    short									handle, 
    ps3000aStreamingReady	lpPs3000aReady,
		void								* pParameter
  );  

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aNoOfStreamingValues)
	(
	  short								handle,
		unsigned long			*	noOfValues
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetMaxDownSampleRatio)
	(
	  short								handle,
		unsigned long 			noOfUnaggreatedSamples,
		unsigned long 		* maxDownSampleRatio,
		PS3000A_RATIO_MODE	downSampleRatioMode,
		unsigned short			segmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetValues)
	(
	  short								handle,
		unsigned long 			startIndex,
	  unsigned long			*	noOfSamples,
	  unsigned long				downSampleRatio,
		PS3000A_RATIO_MODE	downSampleRatioMode,
		unsigned short			segmentIndex,
		short							* overflow
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetValuesBulk)
	(
	  short								handle,
		unsigned long			*	noOfSamples,
		unsigned short			fromSegmentIndex,
		unsigned short			toSegmentIndex,
	  unsigned long				downSampleRatio,
		PS3000A_RATIO_MODE 	downSampleRatioMode,
		short							* overflow
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetValuesAsync)
	(
	  short								handle,
		unsigned long				startIndex,
	  unsigned long				noOfSamples,
	  unsigned long				downSampleRatio,
		short								downSampleRatioMode,
		unsigned short			segmentIndex,
	  void							*	lpDataReady,
		void							*	pParameter
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetValuesOverlapped)
	(
	  short								handle,
	  unsigned long 			startIndex,
	  unsigned long			*	noOfSamples,
	  unsigned long				downSampleRatio,
	  PS3000A_RATIO_MODE	downSampleRatioMode,
		unsigned short      segmentIndex,
	  short				*       overflow
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetValuesOverlappedBulk)
	(
	  short								handle,
		unsigned long				startIndex,
  	unsigned long 	 	*	noOfSamples,
	  unsigned long				downSampleRatio,
	  PS3000A_RATIO_MODE	downSampleRatioMode,
	  unsigned short			fromSegmentIndex,
	  unsigned short			toSegmentIndex,
	  short							*	overflow
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aStop)
	(
	  short handle
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aHoldOff)
	(
	  short								handle,	
		u_int64_t						holdoff,
		PS3000A_HOLDOFF_TYPE	type
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetChannelInformation) 
	(
		short handle, 
		PS3000A_CHANNEL_INFO info, 
		int probe, 
		int * ranges,
		int * length,
		int channels
  );

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aEnumerateUnits)
	(
	short * count,
	char * serials,
	short * serialLth
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aPingUnit)
	(
	short handle
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aMaximumValue)
	(
	short		handle,
	short * value
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aMinimumValue)
	(
	short		handle,
	short * value
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetAnalogueOffset)
	(
	short handle, 
	PS3000A_RANGE range,
	PS3000A_COUPLING	coupling,
	float * maximumVoltage,
	float * minimumVoltage
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aGetMaxSegments)
	(
	short		handle,
	unsigned short * maxSegments
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aChangePowerSource)
	(
	short		handle,
  PICO_STATUS powerState
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps3000aCurrentPowerSource)
	(
	short		handle
	);

#endif