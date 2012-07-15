
//////////////////////////////////////////////////////////////////////////////
// Filename:	HcpErrors.h
// Description:	This file provides error codes and associated error strings.
//				Also provides #defines previously in vip_comm.h.
// Copyright:	Varian Medical Systems
//				All Rights Reserved
//				Varian Proprietary
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
// This file contains error codes and associated error strings for
// accumulated errors that have been identified for PaxScan imager
// software to date. In this scheme error codes are defined in an
// unnamed enum and error strings in a static array.  Error codes
// previously defined at values higher than 3299 or negative have been
// re-defined with values in the range 0-128.  ViVA handles
// old values as well as new. The error string retrieval routine in
// Rev L ViVA is shown commented at the end of the file.
//////////////////////////////////////////////////////////////////////////////


#ifndef INC_PAXERRORS_H
#define INC_PAXERRORS_H

//////////////////////////////////////////////////////////////////////////////
// keep sync'd with array below
enum {
HCP_NO_ERR,					// 0x0000 VIP_COMM ERRCODE
HCP_COMM_ERR,				// 0x0001 VIP_COMM ERRCODE
HCP_STATE_ERR,				// 0x0002 VIP_COMM ERRCODE
HCP_NOT_OPEN,		//  3						// was -1
HCP_DATA_ERR,				// 0x0004 VIP_COMM ERRCODE
HCP_NOT_IMPL_ERR,	//  5						// was 0x4000
HCP_OTHER_ERR,		//  6						// was 0x8000
HCP_DUM_007,
HCP_NO_DATA_ERR,			// 0x0008 VIP_COMM ERRCODE
// error codes added for 4030CB .. OFFSET -3492
HCP_BAD_INI_FILE,	//  9						// was 3501
HCP_BAD_HL_RATIO,	// 10						// was 3502
HCP_BAD_THRESHOLD_VALUES, // 11					// was 3503
HCP_BAD_FILE_PATH,  // 12						// was 3504
HCP_FILE_OPEN_ERR,
HCP_FILE_READ_ERR,
HCP_FILE_WRITE_ERR,
HCP_NO_SRVR_ERR,			// 0x0010 VIP_COMM ERRCODE
// error codes added for 4030R .. OFFSET -3283
HCP_NUMCAL_ERR,		// 17						// was 3300
HCP_MEMALLOC_ERR,	// 18						// was 3301
HCP_NOVIDEO_ERR,	// 19						// was 3302
HCP_NOTREADY_ERR,	// 20						// was 3303
HCP_DIR_NOT_FND,	// 21						// was 3304
HCP_INIT_ERR,		// 22						// was 3305
HCP_SEL_MODE_ERR,	// 23						// was 3306
HCP_NO_FNC_ADDR_ERR,// 24						// was 3307
HCP_SEQ_SAVE_ERR,	// 25						// was 3308
HCP_VID_INIT_ERR,	// 26						// was 3309
HCP_LOGIC_ERR,		// 27						// was 3310
HCP_CALEND_TMOUT,	// 28						// was 3311
HCP_BAD_POINTER,	// 29						// was 3312
HCP_TIMEOUT,		// 30						// was 3313
HCP_DUM_031,		// 31
HCP_SETUP_ERR,		// 32	// 0x0020 VIP_COMM ERRCODE
// error codes added for 2520M .. OFFSET -3367
HCP_NO_DRIVE,		// 33						// was 3400
HCP_FUNC_ADDR,		// 34						// was 3401
HCP_ABORT_OPERATION,// 35						// was 3402
HCP_BAD_DRIVE,		// 36						// was 3403
HCP_OPEN_DEVICE_ERR,// 37						// was 3404
HCP_DEVICE_FULL,	// 38						// was 3405
HCP_WRITE_ERROR,	// 39						// was 3406
HCP_UNSUPPORTED_SIZE,//40						// was 3407
HCP_HEADER_NOT_FOUND,//41						// was 3408
HCP_READ_ERROR,		// 42						// was 3409
HCP_BAD_FILE,		// 43						// was 3410
HCP_LOGIC_ERROR,	// 44						// was 3411
HCP_TOO_FEW_IMS,	// 45						// was 3412
HCP_MULTIPLE_HDR,	// 46						// was 3413
HCP_OFST_ERR,		// 47						// was 3414
HCP_GAIN_ERR,		// 48						// was 3415
HCP_DFCT_ERR,		// 49						// was 3416
HCP_HDR_VERIFY_ERR,	// 50						// was 3417
HCP_VERIFY_INI_FILE_ERR,// 51					// was 3418
// error codes added for 4030CB .. OFFSET -3378
HCP_IMG_DIM_ERR,	// 52						// was 3430
HCP_BAD_STATE_ERR,	// 53						// was 3431
HCP_UNHANDLED_EXCEP,// 54						// was 3432
HCP_TIMEOUT_PANELREADY,// 55					// was 3433
HCP_OPERATION_CNCLD,// 56						// was 3434
HCP_TOO_FEW_MODES,	// 57
HCP_MAX_ITER,		// 58
HCP_DIAG_DATA_ERR,  // 59
HCP_DIAG_SEQ_ERR,	// 60
HCP_DIAG_STATE_ERR,	// 61
HCP_DUM_062,		// 62
HCP_DUM_063,		// 63
HCP_NO_CAL_ERR,		// 64	// 0x0040 VIP_COMM ERRCODE
HCP_NO_OFFSET_CAL_ERR,//65	// 0x0041 VIP_COMM ERRCODE
HCP_NO_GAIN_CAL_ERR,// 66   // 0x0042 VIP_COMM ERRCODE
HCP_NO_CORR_CAP_ERR,// 67	// 0x0043 VIP_COMM ERRCODE
HCP_DEV_AMBIG,		// 68
HCP_MAC_NOT_FOUND,	// 69
HCP_MULTI_REC,		// 70
HCP_RESELECT_FAIL,	// 71
HCP_RECID_CONFLICT,	// 72
HCP_CAL_ERROR,		// 73
HCP_DUM_074,		// 74
HCP_DUM_075,		// 75
HCP_DUM_076,		// 76
HCP_DUM_077,		// 77
HCP_DUM_078,		// 78
HCP_CALACQ_WARN,	// 79 This code is returned when the number of acqusition
			// frames exceeds 1 or the number of calibration frames
			// exceeds 4 if a fixed frame rate panel is detected. Note that
			// a larger number of calibration frames provides no benefit with
			// rad panels.
// error codes added for Pleora
HCP_NULL_FUNC,		// 80 NOTE: specific useage when a module
						// does not support a call - this translates
						// to HCP_NOT_IMPL_ERR when all modules do
						// not export otherwise it translates to
						// HCP_NO_ERR. For fatal NULL function
						// use HCP_FUNC_ADDR (=34)
HCP_NO_SUBMOD_ERR,	// 81
HCP_NO_MODE_SEL,	// 82
HCP_PLEORA_ERR,		// 83
HCP_FG_INIT_ERR,	// 84
HCP_GRAB_ERR,		// 85
HCP_STARTGRAB_ERR,  // 86
HCP_STOPGRAB_ERR,	// 87
HCP_STARTREC_ERR,	// 88
HCP_STOPREC_ERR,	// 89
HCP_NO_CONNECT_ERR,	// 90
HCP_DATA_TRANS_ERR,	// 91
HCP_NO_EVENT_ERR,	// 92
HCP_FRM_GRAB_ERR,	// 93
HCP_NOT_GRAB_ERR,	// 94
HCP_BAD_PARAM,		// 95
HCP_ZERO_FRM,		// 96
HCP_RES_ALLOC_ERR,	// 97
HCP_BAD_IMG_DIM,	// 98
HCP_FRM_DFLT,		// 99
HCP_REC_NOT_SUPP,	//100
HCP_REC_CNFG_ERR,	//101
HCP_SHORT_BUF_ERR,	//102
HCP_RES_ALREADY_ALLOC,//103
HCP_BAD_REQ_ERR,	//104
HCP_REC_NOT_READY,	//105
HCP_PLR_CONNECT,	//106
// error code added for 4030CB .. OFFSET -3492
HCP_INTERNAL_LOGIC_ERROR,//107					// was 3599
HCP_PLR_SERIAL,		//108
HCP_BAD_FORMAT,		//109
HCP_DRV_VERS_ERR,	//110
HCP_STRCT_ERR,		//111
HCP_DLL_VERS_ERR,	//112
HCP_FXD_RATE_ERR,	//113
HCP_DUM_114,		//114
HCP_DUM_115,		//115
HCP_DUM_116,		//116
HCP_DUM_117,		//117
HCP_DUM_118,		//118
HCP_DUM_119,		//119
HCP_DUM_120,		//120
HCP_DUM_121,		//121
HCP_DUM_122,		//122
HCP_DUM_123,		//123
HCP_DUM_124,		//124
HCP_DUM_125,		//125
HCP_DUM_126,		//126
HCP_DUM_127,		//127
HCP_NO_IMAGE_ERR,	//128	// 0x0080 VIP_COMM ERRCODE
HCP_MAX_ERR_CODE
};
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
// NOTE TO USER: Do not assume an error code is necessarily within the
// bounds of the array below. The array bounds should be verified at runtime
// by searching for the last string "UUU". See example code commented at end
// of file.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// keep sync'd with Error enum above above
static const char *HcpErrStrList[] = {
	"No Error",				//  0
	"Communication Error",	//  1
	"State Error",			//  2
	"Initialization error OR Device not open",	//  3
	"Data Error",			//  4
	"Not Implemented Error",//  5
	"Other Error",			//  6
	"!!Unknown Error",		//  7
	"No Data Error",		//  8
	"Bad Ini File",			//  9
	"Bad Gain Ratio",		// 10
	"Bad Threshold Values",	// 11
	"Bad File Path",		// 12
	"File Open Error",		// 13
	"File Read Error",		// 14
	"File Write Error",		// 15
	"No Server Error",		// 16
	"Number of Calibration Frames exceeds Allocated Buffers",	// 17
	"Memory Allocation Error",									// 18
	"Video Not Initialized",									// 19
	"Acquisition State: Not Ready",								// 20
	"Directory Not Found",										// 21
	"Error Initializing IMAGERs Directory",						// 22
	"Error Selecting Mode",										// 23
	"Function address not found in library",					// 24
	"Sequence Not Saved",										// 25
	"Error Initializing Video",									// 26
	"Logic Error",												// 27
	"Timeout in Data Thread - Possible Memory Leak",			// 28
	"Bad Pointer",			// 29
	"Timed out",			// 30
	"!!Unknown Error",		// 31
	"Setup Error",			// 32
	"Drive Not Found",		// 33
	"Function Address not found in Library",					// 34
	"", // "Abort Operation" no message should be displayed		// 35
	"Bad Drive Letter",		// 36
	"Error Opening Device", // 37
	"Not Enough Space on Device",	// 38
	"Error Writing to Device",		// 39
	"Media Size Not Supported",		// 40
	"Header not found on flash card",//41
	"Error Reading from Device",	// 42
	"File read/write error",		// 43
	"Logic Error",					// 44
	"Too few Images for Calibration",//45
	"Multiple headers found on Flash - Not Usable",				// 46
	"Offset correction not accepted",							// 47
	"Gain correction not accepted",								// 48
	"Defect correction not accepted",							// 49
	"Header Verification Problem." \
			"This could result from changing the memory card while" \
			" the link is open.Select 'Reset Link' from the" \
			" Acquisition menu.",								// 50
	"Error due to probable read-only ini file",					// 51
	"Image dimensions do not match with ini file",				// 52
	"Bad CB state selected",									// 53
	"Unhandled exception in dll",								// 54
	"Timed out waiting for ReadyForPulse",						// 55
	"Operation Cancelled",			// 56
	"Not enough modes allocated",	// 57
	"Iteration limit reached, or convergence not achievable",		// 58
	"Diagnostic data not recognized",	// 59
	"Receptor frame number out of sequence",	// 60
	"Receptor frame unexpected state",		// 61
	"!!Unknown Error",				// 62
	"!!Unknown Error",				// 63
	"No Calibration Error",			// 64
	"No Offset Calibration",		// 65
	"No Gain Calibration Error",	// 66
	"Command Processor is not Correction Capable",				// 67
	"Device ambiguity - Multiple devices found, " \
			"and unable to resolve which is required. Possibly " \
			"because MAC address not set in ini file.",			// 68
	"Unable to find the device with the specified MAC address. " \
			"Yo may need to select another or create a new " \
			"receptor directory for the device in use. (e.g. " \
			"ViVA -- Acquisition>Receptor Select>New)",			// 69
	"Multiple link open failed. Receptors must be supported " \
			"by the same RP & FG dlls - the latter of type 'FgEx'",		// 70
	"Open link failed AND unable to reselect the prior receptor --\r\n" \
	"ALL RECEPTOR LINKS HAVE BEEN CLOSED",						// 71
	"The selected device is already open. Check MAC address " \
	"or other identifier in HcpConfig.ini",						// 72
	"Calibration Error",			// 73
	"!!Unknown Error",				// 74
	"!!Unknown Error",				// 75
	"!!Unknown Error",				// 76
	"!!Unknown Error",				// 77
	"!!Unknown Error",				// 78
	"WARNING: The number of frames specified in the call has " \
	"been accepted.\r\nBUT errors could result with rad panels.",// 79
				// This code is returned when the number of acqusition
				// frames exceeds 1 or the number of calibration frames
				// exceeds 4 if a fixed frame rate panel is detected.
	"Function Not Exported",			// 80
	"Sub-Module Not Found",			// 81
	"No Mode Selected",				// 82
	"Pleora Error",					// 83
	"Frame Grabber Initialization Error",// 84
	"Frame Grabber Error",			// 85
	"Error Starting Grabber",		// 86
	"Error Stopping Grabber",		// 87
	"Error Starting Record Sequence",// 88
	"Error Stopping Record Sequence",// 89
	"Error Communicating With Frame Grabber",//90
	"Data Transmission Error",		// 91
	"No Event For Index",			// 92
	"Error Occurred During Grab",	// 93
	"Not Grabbing Error",			// 94
	"Bad Parameter",				// 95
	"Zero frames specified",		// 96
	"Resource allocation problem",	// 97
	"Bad Image Dimension",			// 98
	"Frame Rate is Overridden",		// 99
	"Receptor Not Supported",		//100
	"Problem with receptor config file",	//101
	"Inadequate buffer supplied",	//102
	"Resource already allocated",	//103
	"Unable to handle request",		//104
	"Receptor Not Ready",			//105
	"Pleora Connect Error\r\n" \
		"For driver info, search for:\r\n" \
		"'iPORT Driver Manual.pdf'." \
		"A suggested IP address for your ethernet adapter is: " \
		"192.168.2.11",				//106
	"Internal Logic Error",			//107
	"Serial transmission error - Please try again",				//108
	"Storage device is not formatted correctly",				//109
	"Driver Version Error",			//110
	"Problem with structure or StrucSize member",				//111
	"Dll Version Error",			//112
	"Fixed Frame Rate Receptor",	//113
	"!!Unknown Error",				//114
	"!!Unknown Error",				//115
	"!!Unknown Error",				//116
	"!!Unknown Error",				//117
	"!!Unknown Error",				//118
	"!!Unknown Error",				//119
	"!!Unknown Error",				//120
	"!!Unknown Error",				//121
	"!!Unknown Error",				//122
	"!!Unknown Error",				//123
	"!!Unknown Error",				//124
	"!!Unknown Error",				//125
	"!!Unknown Error",				//126
	"!!Unknown Error",				//127
	"No Image",						//128
	"UUU"}  ;   // Unknown Error String - MUST BE LAST
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// image types not defined in vip_comm.h specific to Virtual CP
#define VIP_ANALOG_OFFSET_IMAGE		8
#define VIP_PREVIEW_IMAGE           9
#define VIP_RAD_OFFSET_IMAGE       10

#define	HCP_FG_TARGET_FLAG			0xF3000000
#define	HCP_RP_TARGET_FLAG			0xF4000000
#define HCP_RESEND_TARGET_FLAG      0xF5000000
#define	HCP_XX_TARGET_MASK			0xFF000000
#define	HCP_NON_TARGET_MASK			0x00FFFFFF

#define VIP_CURRENT_IMG_0         0x10  | HCP_RP_TARGET_FLAG // Same as PREVIEW
#define VIP_CURRENT_IMG_1         0x11  | HCP_RP_TARGET_FLAG // Current image using 1 post acquired offset - same as VIP_CURRENT_IMAGE for a 1 offset mode
#define VIP_CURRENT_IMG_2         0x12  | HCP_RP_TARGET_FLAG // Current image using 2 post acquired offsets - same as VIP_CURRENT_IMAGE for a 2 offset mode
#define VIP_CURRENT_IMG_RAW       0x1F  | HCP_RP_TARGET_FLAG // Current image uncorrected - ignore specified corrections

#define VIP_OFFSET_IMG_0          1 // Pre (full) offset - = VIP_OFFSET_IMAGE (handled by HcpCorr as normal)
#define VIP_OFFSET_IMG_1          0x21  | HCP_RP_TARGET_FLAG // Offset image - 1st post acquired offset
#define VIP_OFFSET_IMG_2          0x22  | HCP_RP_TARGET_FLAG // Offset image - 2nd post acquired offset
#define VIP_OFFSET_IMG_AV         0x2F  | HCP_RP_TARGET_FLAG // Offset image - Average of all post acquired offsets

// CB image types must specify HI_GAIN (state=0) or LO_GAIN (state=1)
// define image types as existing types + (256 * state)
#define HCP_STATE_HI				0
#define HCP_STATE_LO				1
#define HCP_STATE_SHIFT				8
#define HCP_OFST_HI					(HCP_STATE_HI << HCP_STATE_SHIFT)
#define HCP_OFST_LO					(HCP_STATE_LO << HCP_STATE_SHIFT)
#define HCP_IMGTYPE_MASK			0x000000FF
#define HCP_STATE_MASK				0x0000FF00

// CB image types
#define VIP_OFFSET_IMAGE_HI			(VIP_OFFSET_IMAGE + HCP_OFST_HI)
#define VIP_GAIN_IMAGE_HI			(VIP_GAIN_IMAGE + HCP_OFST_HI)
#define VIP_BASE_DEFECT_IMAGE_HI	(VIP_BASE_DEFECT_IMAGE + HCP_OFST_HI)
#define VIP_AUX_DEFECT_IMAGE_HI		(VIP_AUX_DEFECT_IMAGE + HCP_OFST_HI)

#define VIP_OFFSET_IMAGE_LO			(VIP_OFFSET_IMAGE + HCP_OFST_LO)
#define VIP_GAIN_IMAGE_LO			(VIP_GAIN_IMAGE + HCP_OFST_LO)
#define VIP_BASE_DEFECT_IMAGE_LO	(VIP_BASE_DEFECT_IMAGE + HCP_OFST_LO)
#define VIP_AUX_DEFECT_IMAGE_LO		(VIP_AUX_DEFECT_IMAGE + HCP_OFST_LO)

#ifndef VIP_HEADER
//#define VIP_HEADER --- Don't define here. If vip_comm functions
// decalarations are needed this would prevent vip_comm.h inclusion.
// Worst case is developer gets a redefinition warning which should
// prompt to change the order of vip_comm.h and hcperrors.h if both
// are needed.

// vip_comm Return Codes
#define VIP_NO_ERR            HCP_NO_ERR
#define VIP_COMM_ERR          HCP_COMM_ERR
#define VIP_STATE_ERR         HCP_STATE_ERR
#define VIP_DATA_ERR          HCP_DATA_ERR
#define VIP_NO_DATA_ERR       HCP_NO_DATA_ERR
#define VIP_NO_SRVR_ERR       HCP_NO_SRVR_ERR
#define VIP_SETUP_ERR         HCP_SETUP_ERR
#define VIP_NO_CAL_ERR        HCP_NO_CAL_ERR
#define VIP_NO_OFFSET_CAL_ERR HCP_NO_OFFSET_CAL_ERR
#define VIP_NO_GAIN_CAL_ERR   HCP_NO_GAIN_CAL_ERR
#define VIP_NO_CORR_CAP_ERR   HCP_NO_CORR_CAP_ERR
#define VIP_NO_IMAGE_ERR      HCP_NO_IMAGE_ERR
#define VIP_NOT_IMPL_ERR      HCP_NOT_IMPL_ERR
#define VIP_OTHER_ERR         HCP_OTHER_ERR

// IO links.
#define VIP_NO_LINK       -1
#define VIP_ETHERNET_LINK 0
#define VIP_SERIAL_LINK   1
#define VIP_COM1 0
#define VIP_COM2 1
#define VIP_COM3 2
#define VIP_COM4 3

#define VIP_DISPL_TST_PTTRN_STRTUP 0
#define VIP_STANDALONE_STRTUP      1
#define VIP_ACTIVE_ACQ_STRTUP      2

// System Version Number types
#define VIP_MOTHERBOARD_VER    0
#define VIP_SYS_SW_VER         1
#define VIP_GLOBAL_CTRL_VER    2
#define VIP_GLOBAL_CTRL_FW_VER 3
#define VIP_RECEPTOR_VER       4
#define VIP_RECEPTOR_FW_VER    5
#define VIP_IPS_VER            6
#define VIP_VIDEO_OUT_VER      7
#define VIP_VIDEO_OUT_FW_VER   8
#define	VIP_COMMAND_PROC_VER   9
#define	VIP_RECEPTOR_CONFIG_VER		10

// Image Types
// for CB image types see below
#define VIP_CURRENT_IMAGE			0
#define VIP_OFFSET_IMAGE			1
#define VIP_GAIN_IMAGE				2
#define VIP_BASE_DEFECT_IMAGE		3
#define VIP_AUX_DEFECT_IMAGE		4
#define VIP_TEST_IMAGE				5
#define VIP_RECEPTOR_TEST_IMAGE		6
#define VIP_RECEPTOR_TEST_IMAGE_OFF 7

	// Software Handshaking Constants
#define VIP_SW_PREPARE           0
#define VIP_SW_VALID_XRAYS       1
#define VIP_SW_RADIATION_WARNING 2
#define VIP_SW_RESET             3
// Use to enable and disabling frame summing while acquisition is in progress
#define VIP_SW_FRAME_SUMMING	 4

// Rad Scaling Type Constants
#define VIP_RAD_SCALE_NONE 0
#define VIP_RAD_SCALE_UP   1
#define VIP_RAD_SCALE_DOWN 2
#define VIP_RAD_SCALE_BOTH 3

// Gain Scaling Type Constants
#define VIP_GAIN_SCALE_NONE		0
#define VIP_GAIN_SCALE_EXPAND   1


// Acquisition Constants
#define VIP_ACQ_TYPE_CONTINUOUS   0
#define VIP_ACQ_TYPE_ACCUMULATION 1
// CB Data Type Constants
#define VIP_CB_NORMAL				0x0000
#define VIP_CB_DUAL_READ			0x0100
#define VIP_CB_DYNAMIC_GAIN			0x0200
// The bitwise OR: Acquisition Constants | CB Data Type Constants
// is returned by vip_get_mode_info as *acq_type


// Mode Acquisition Type Constants.
#define VIP_INVALID_ACQ_MODE_TYPE  -1
#define VIP_VALID_XRAYS_N_FRAMES   0
#define VIP_VALID_XRAYS_ALL_FRAMES 1
#define VIP_AUTO_SENSE_N_FRAMES    2
#define VIP_AUTO_SENSE_ALL_FRAMES  3
// Mode Acquisition CB Constants
#define VIP_CB_AUX_MODE_FLAG		500

#define VIP_CB_BASE_MODE			0	// DYNAMIC GAIN normal switching
#define VIP_CB_AUX_MODE_1			1	// DYNAMIC GAIN forced lo gain

// CB auxiliary states are selected by a call
// to vip_set_mode_acq_type with acq_type=VIP_CB_AUX_MODE_FLAG
// then the number of the auxiliary mode is sent in the num_frames field
//
// vip_set_mode_acq_type should return an error if the mode specifies
// other than a DYNAMIC GAIN mode currently and VIP_CB_AUX_MODE_FLAG is set

#define VIP_ACQ_MASK				0x00FF
#define VIP_CB_MASK					0x0F00
#define VIP_CB_SHIFT				8



// Gray Level Mapping Constants
#define VIP_WL_MAPPING_LINEAR     0
#define VIP_WL_MAPPING_NORM_ATAN  1
#define VIP_WL_MAPPING_CUSTOM     2

// Opcode.
#define VIP_AUTO_GAIN_CTRL     0x12
#define VIP_AUTO_GAIN_CTRL_ON  1
#define VIP_AUTO_GAIN_CTRL_OFF 0

#define VIP_VERT_REVERSAL_OP   0x1d
#define VIP_VERT_REVERSAL_ON   1
#define VIP_VERT_REVERSAL_OFF  0

#define VIP_HORZ_REVERSAL_OP   0x1e
#define VIP_HORZ_REVERSAL_ON   1
#define VIP_HORZ_REVERSAL_OFF  0

// System Modes
#define VIP_SYSTEM_MODE_NORMAL		0
#define VIP_SYSTEM_MODE_TEST		1
#define	VIP_SYSTEM_MODE_FLASH_TEST	2

// Register Types
#define GLOBAL_CONTROL_REGISTER	0

// Register Names
#define EXPOSE_OK_REGISTER	72	// 0x48 which is where the register is mapped

#ifdef INC_FRAMERATE_VALS
double AllowedFrameRates[] = { 1.0, 1.5, 2.0, 3.0, 3.75, 5.0,
									6.0, 7.5, 10.0, 15.0, 30.0, -1.0 };
            // use the '-1.0' as a terminator ***** add new values ahead of '-1.0'
#endif // INC_FRAMERATE_VALS

#define SYS_SOFTWARE_FILE_STR	"SystemSoftwareFile"
#define RCPT_CONFIG_FILE_STR	"ConfigDataFile"
#define BASE_DEFECT_IMAGE_STR	"BaseDefectImage"
#define AUX_DEFECT_IMAGE_STR	"AuxDefectImage"
#define GAIN_CAL_IMAGE_STR		"GainCalImage"
#define OFFSET_CAL_IMAGE_STR	"OffsetCalImage"
#define SYS_CONFIG_FILE_STR		"SystemDataFile"
#define GC_FW_FILE_STR			"GcFirmware"
#define RCPT_FW_FILE_STR		"RcptFirmware"
#define VIDEO_OUT_FILE_STR		"VideoOutFirmware"
#define DETAILED_MODE_DESCR		"DetailedModeDesc"

enum VistCapableList
{
	VIP_VISTA_SUPPORT_UNKNOWN = 0,
	VIP_VISTA_UNSUPPORTED,
	VIP_VISTA_SUPPORTED
};

union RegisterUnion
{
	UCHAR  value8Bit;
	USHORT value16Bit;
	ULONG  value32Bit;
};

#endif // VIP_HEADER
#endif // INC_PAXERRORS_H


// finds the error string for error code
// standard VIP codes are extended
/*
#define MAX_HCP_ERROR_CNT 65536
void GetCPError(int errCode, CString* errMsg)
{
	static int hcpErrCnt=0;
	if(!hcpErrCnt)
	{
		for(int i=0; i<MAX_HCP_ERROR_CNT; i++)
		{
			if(!strncmp(HcpErrStrList[i], "UUU", MIN_STR)) break;
		}
		if(i < MAX_HCP_ERROR_CNT)
		{
			hcpErrCnt = i;
		}
		if(hcpErrCnt <= 0)
		{
			if(errMsg) *errMsg += "Error at Compile Time";
			return;
		}
	}

	if(errCode < 0 || errCode >= HCP_MAX_ERR_CODE)
	{
		if(errCode == -1) errCode = 3;
		else if(errCode == 0x4000) errCode = 5;
		else if(errCode == 0x8000) errCode = 6;
		else if(errCode > 3500 && errCode < 3600) errCode -= 3492;
		else if(errCode > 3299 && errCode < 3314) errCode -= 3283;
		else if(errCode > 3399 && errCode < 3419) errCode -= 3367;
		else if(errCode > 3429 && errCode < 3435) errCode -= 3378;
	}

	if(errCode >= 0 && errCode < hcpErrCnt)
	{
		if(errMsg) *errMsg += HcpErrStrList[errCode];
	}
	else
	{
		if(errMsg) *errMsg += "Unknown Error";
	}
}

*/
