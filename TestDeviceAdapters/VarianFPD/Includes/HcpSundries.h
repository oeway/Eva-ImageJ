

//////////////////////////////////////////////////////////////////////////////
// Filename:	hcpsundries.h
// Description:	This file provides structures and related enums used by the
//				interface.
// Copyright:	Varian Medical Systems
//				All Rights Reserved
//				Varian Proprietary
//////////////////////////////////////////////////////////////////////////////

#ifndef VIP_INC_SUNDRYSTRUCTS_H
#define VIP_INC_SUNDRYSTRUCTS_H

#ifndef MAX_STR
#define MAX_STR		256
#endif

#ifndef MIN_STR
#define MIN_STR		32
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// **>> vip_cal_control <<**
// The following is used in specialized user controlled calibrations.
// In these cases a calType is sent first as described above by
// vip_gain_cal_prepare. A series of calls to vip_cal_control then
// results in acquisitions of the specified type. Each call to vip_cal_control
// specifes a CtrlType from the list below. The calibration must be
// terminated by a call to vip_cal_end.
enum {
HCP_CTRLTYPE_MIN = 46,
HCP_CTRLTYPE_OFST,
HCP_CTRLTYPE_GAIN,
HCP_CTRLTYPE_GAIN_HI_G,
HCP_CTRLTYPE_GAIN_LO_G,
HCP_CTRLTYPE_EXT0,
HCP_CTRLTYPE_EXT1,
HCP_CTRLTYPE_EXT2,
HCP_CTRLTYPE_EXT3,
HCP_CTRLTYPE_EXTCAL,
HCP_CTRLTYPE_GR_DUAL,
HCP_CTRLTYPE_MAX
};

// The settings specified by the following option determine how data are
// acquired and transmitted (sent) to the Corrections module.
// The AccMode setting determines whether any previous data should be ignored
// or the current acquisition added to them. Also determines whether the
// data are sent to storage or not.
enum {
HCP_ACCMODE_RESET_AND_SEND,	// resets any previous acquired data to zero
								// and sends the new data for storage
HCP_ACCMODE_RESET_AND_HOLD,	// resets any previous data and holds the new data
								// without sending for storage
HCP_ACCMODE_SUM_AND_HOLD,	// sums to any previous data and holds the new data
								// without sending for storage
HCP_ACCMODE_END_AND_SEND,	// no further acquisition is done; sends
								// existing data for storage
};


struct SCalCtrl
{
	int				StructSize;		// Initialize to sizeof(SCalCtrl)
	int				CtrlType;		// specifies acquisition type requested
	int				AccMode;		// specifies how frames are used
									// relative to any previous acquisition
	int				NumFrames;		// number of frames to acquire
									// zero default implies number calibration
									// frames or acquisition frames if rad gain
	float			GainRatio;		// returned when CtrlType = HCP_CTRLTYPE_EXTCAL
	int				Reserved1;
	int				Reserved2;
	int				Reserved3;
};

//////////////////////////////////////////////////////////////////////////////
// **>> vip_check_link <<**
#define		HCP_CHKLNK_LONG	3
#define		HCP_CHKLNK_SHRT	4
struct SCheckLink
{
	int				StructSize;	// Initialize to sizeof(SOpenReceptorLink)
	int				ImgMedianVal;	// result of check_link (input ignored) - image Median
	float			ImgStdDev;		// result of check_link (input ignored) - image StdDev
	int				ImgMedLoLim;	// FOR NORMAL OPERATION LEAVE AT ZERO DEFAULT.
									// lo limit of acceptable median - zero
									// implies 100 default
	int				ImgMedHiLim;	// FOR NORMAL OPERATION LEAVE AT ZERO DEFAULT.
									// hi limit of acceptable median - zero
									// implies value derived from receptor
									// configuration
	float			ImgMedSDRatioLim;// FOR NORMAL OPERATION LEAVE AT ZERO DEFAULT.
									// Acceptable ratio Median / StdDev -
									// zero implies that it must not be below 2
	int				NumImgAcq;		// FOR NORMAL OPERATION LEAVE AT ZERO DEFAULT.
									// MUST be 0 or 1 when using ChkLnkType below.
									// number of images to acquire - zero is
									// interpreted as 1 -- BUT this should NOT
									// be set to a value greater than 1 for rad
									// panels as it will override the special efficient
									// operation provided and contolled through
									// ChkLnkType below
	int				ChkLnkType;		// FOR NORMAL OPERATION LEAVE AT ZERO DEFAULT.
									// NOTE: ignored if NumImgAcq > 1.
									// May be used to control check link type
									// using one of the values #defined above
									// HCP_CHKLNK_SHRT -- gets 1 frame only
									// HCP_CHKLNK_LONG -- gets 2 frames AND
									// updates the fixed frame rate field in
									// HcpConfig.ini if applicable.
									//
									// Default operation uses HCP_CHKLNK_LONG
									// for first call for each receptor index after
									// application launch. HCP_CHKLNK_SHRT is used
									// subsequently.
	int				Reserved3;
	int				Reserved2;
	int				Reserved1;
};

//////////////////////////////////////////////////////////////////////////////
// **>> vip_close_link <<**
#define		CLSLNK_POWERDOWN		0x0100
			// Use to signal a power down requirement when using I/O box.
			// Previously this was done with -ve value for the recNum
			// parameter. The -ve value will still be recognized but will be
			// interpreted as close all with power shutdown. Selectively
			// closing with power shutdown can be done by bitwise OR of this
			// constant with recNum.
#define		CLSLNK_RELMEM			0x0200
			// Use to signal that buffer memory should be deleted.
#define		CLSLNK_CLOSEALL			0x0400
			// Use to signal that all links should be closed.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// **>> vip_correct_image <<**
struct SCorrectImage
{
	int				StructSize;		// Initialize to sizeof(SCorrectImage)
	WORD*			BufIn;
	int				BufInX;
	int				BufInY;
	WORD*			BufOut;
	// Note that if the image manipulation to rotate 90 degrees is in use (see
	// vip_set_correction_settings), then BufOutX and BufOutY should
	// be set accordingly to accept the rotated image.
	int				BufOutX;
	int				BufOutY;
	int				CorrType;
	int				Reserved1;
};

//////////////////////////////////////////////////////////////////////////////
// **>> vip_gain_cal_prepare <<**
// The following are used in calls to vip_gain_cal_prepare(mode, calType).
// calType is defaulted to zero for normal gain calibrations where there is
// an automated sequence as in real CP use. For more specialized calibrations
// like those in dual gain modes the calType is specified. The calibration
// module then expects calibrations to be controlled by one or more calls
// to vip_cal_control followed by vip_cal_end.
enum {
HCP_CALTYPE_MIN = 23,	// do not use
HCP_CALTYPE_GAIN,		// gain cal that may involved additional steps
							// in dual gain modes
HCP_CALTYPE_EXTGAIN,	// extended gain cal - DYNAMIC_GAIN modes only
HCP_CALTYPE_GAINRATIO,	// gain ratio calibration - DUAL_READ modes only
HCP_CALTYPE_MAX			// do not use
};

//////////////////////////////////////////////////////////////////////////////
// **>> vip_get_analog_offset info <<**
struct SAnalogOffsetInfo
{
	int				StructSize;	// Initialize to sizeof(SAnalogOffsetInfo)
	int				AsicNum;
	unsigned long	AnalogOfstElapsdTime;
	int*			AsicOffsets;
};

//////////////////////////////////////////////////////////////////////////////
// **>> vip_get_analog_offset params <<**
// **>> vip_set_analog_offset params <<**
struct SAnalogOffsetParams
{
	int				StructSize;	// Initialize to sizeof(SAnalogOffsetParams)
	int				TargetValue;
	int				Tolerance;
	int				MedianPercent;
	float			FracIterDelta;
	int				NumIterations;
};

//////////////////////////////////////////////////////////////////////////////
// **>> vip_get_cal_info <<**
// statndard struct used in the vip_get_cal_info call
struct SCalInfo
{
	int				StructSize;		// Initialize to sizeof(SCalInfo)
	float			OfstMedian;
	float			OfstStdDev;
	float			GainMedian;
	float			GainStdDev;
	float			GainScaling;
	unsigned long	Time;
	int				GainState;		// Set when using dual read or dynamic gain
										// modes. Use one of the values defined
										// below GNSTATE_DYN_HI or
										// GNSTATE_FLG_LO
	int				Reserved1;
	int				Reserved2;
	int				Reserved3;
	int				Reserved4;
};


// special struct used in the vip_get_cal_info call for DGS modes to retrieve
// extended gain calibration statistics
#define EXGN_ID			0xCB
#define GNSTATE_DYN_HI	0
#define GNSTATE_FLG_LO	1
// use this struct in place of SCalInfo - to retrieve cal info for extended
// gain calibrations with dynamic gain modes - as follows:
// SCalInfoExGn calInfoEx;
// memset(&calInfoEx, 0, sizeof(SCalInfoExGn));
// calInfoEx.StructSize = sizeof(SCalInfoExGn);
// calInfoEx.VersionID = EXGN_ID;
// calInfoEx.GainState = GNSTATE_DYN_HI;	// or GNSTATE_FLG_LO
// calInfoEx.XrayLevel = HCP_CTRLTYPE_EXT0;	// any of HCP_CTRLTYPE_EXTn where n = 0,1,2,3

			// the XrayLevel parameter uses values from those used in
			// the vip_cal_control call as defined above
			// the combinations of GainState and XrayLevel provide for 8
			// different sets of statistics to be returned

// SCalInfo* calInfo = (SCalInfo*)(&calInfoEx);
// .. make call using calInfo
struct SCalInfoExGn
{
	int				StructSize;	// Initialize to sizeof(SCalInfoExGn)
	int				CallerID;	// Set to EXGN_ID to identify struct type
	int				GainState;	// Set to GNSTATE_DYN_HI or GNSTATE_FLG_LO
	int				XrayLevel;	// Set to HCP_CTRLTYPE_EXTn where n = 0,1,2,3
	int				CbMedian;		// Value returned
	float			CbStdDev;		// Value returned
	int				ImgCnt;			// Value returned
	int				BadFlagCnt;		// Value returned
	unsigned long	Time;			// Value returned
	float			GainRatio;		// Value returned
	int				Reserved1;
	int				Reserved2;
	int				Reserved3;
	int				Reserved4;
};
// NOTE: the GainRatio is not returned (remains as zero) during the
// calibration. It is updated only if HCP_CTRLTYPE_EXT1 is included
// in the acquisitions. The current value is returned for all allowed
// GainState and XrayLevel settings at other times.

//////////////////////////////////////////////////////////////////////////////
// **>> vip_get_cal_limits <<**
// **>> vip_set_cal_limits <<**
struct SCalLimits
{
	int				StructSize;		// Initialize to sizeof(SCalLimits)
	int				OfstLimitLo;
	int				OfstLimitHi;
	int				GainLimitHi;
};

//////////////////////////////////////////////////////////////////////////////
// **>> vip_get_correction_settings <<**
// **>> vip_set_correction_settings <<**
struct SCorrections
{
	int				StructSize;	// Initialize to sizeof(SCorrections)
	BOOL			Ofst;
	BOOL			Gain;
	BOOL			Dfct;
	BOOL			Line;
	// the next two settings are mode specific and are returned only for
	// CB mode types for the currently selected mode -- NOT YET IMPLEMENTED
	int				PixDataFormat; // ignored by vip_set_correction_settings
	float			GainRatio; // ignored by vip_set_correction_settings

					// Requests for gain, offset and defect images
					// always return the unmanipulated image.
	int				Rotate90; // Returned image is rotated 90 degrees
	int				FlipX;		// Returned image is flipped in X direction
									// (mirrored about vertical axis)
	int				FlipY;  // Returned image is flipped in Y direction

	int				Reserved1;
};

//////////////////////////////////////////////////////////////////////////////
// **>> vip_get_gain_scaling_info <<**
// **>> vip_set_gain_scaling_info <<**
struct SGainScaling
{
	int				StructSize;	// Initialize to sizeof(SGainScaling)
	int				GainType;
	int				MaxLinearValue;
	int				PxSatValue;
	int				PxRepValue;
};

//////////////////////////////////////////////////////////////////////////////
// **>> vip_set_hw_config <<**
// the following are specialized controls used in the vip_set_hw_config call
enum {
HCP_FRMINTRPT_FG_PULS0,
HCP_FRMINTRPT_FG_FVAL,
HCP_FRMINTRPT_FG_MAX,
};

enum {
HCP_GENSYNC_AC_ADU200,
HCP_GENSYNC_AC_NONE,
HCP_GENSYNC_AC_MAX,
};

struct SHwConfig
{
	int				StructSize;		// Initialize to sizeof(SHwConfig)
	int				FrmIntrptSrc;	// This defines frame interrupt source
									// for the FG module - PULS0 is the pulse
									// generator used to generate internal
									// frame starts. FVAL is FrameValid.
	int				GenSyncType;	// This defines how AC handles Generator Sync.
									// ADU200 uses a USB board to interface to
									// generator. NONE implies no AC control
									// over generator. All signal_frame_starts
									// are treated as valid frames.
	int				TimeParam1;
	int*			OffsetPtr;		// used by analog offset calibration
									// otherwise must be NULL
	int				OffsetLen;
	int 			ModeNum;
};

//////////////////////////////////////////////////////////////////////////////
// **>> vip_get_mode_info <<**
struct SModeInfo
{
	int				StructSize;	// Initialize to sizeof(SModeInfo)
	int				ModeNum;
	int				AcqType;	// VIP_ACQ_TYPE_ACCUMULATION etc; VCP_CB - CHECK ALL INSTANCES
	float			FrameRate;
	float			AnalogGain;
	int				LinesPerFrame;
	int				ColsPerFrame;
	int				LinesPerPixel;
	int				ColsPerPixel;
	char			ModeDescription[MAX_STR];
	char			DirReadyModeDescription[MAX_STR];
	int				DcdsEnable;
	float			MxAllowedFrameRate;
	BOOL			UserSync;
	int 			AcqFrmCount;
	int 			CalFrmCount;
	int				GainRoiUpperLeftX;
	int				GainRoiUpperLeftY;
	int				GainRoiLowerRightX;
	int				GainRoiLowerRightY;
	int				UncorrectablePixelRepValue;
	int				OffsetCalShift;
	int				MaxDefectRange;
	int				LeanBufferStatus; // in rad mode we may choose to allocate
										// fewer buffers and operate a sum
										// as we go mode when calibrating
	int				Reserved3;
	int				Reserved2;
	int				Reserved1;
	void*			ExtInfoPtr;
	int				ExtInfoLen;
};

//////////////////////////////////////////////////////////////////////////////
// **>> vip_get_system_info <<**
struct SSysInfo
{
	int				StructSize;	// Initialize to sizeof(SSysInfo)
	int				NumModes;
	int				DfltModeNum;
	int				MxLinesPerFrame;
	int				MxColsPerFrame;
	int				MxPixelValue;
	BOOL			HasVideo;
	char			SysDescription[MAX_STR];
	int				StartUpConfig;
	int				NumAsics;
	int				ReceptorType;
	int				BorderPixels;
	int				MxImageValue; // normally the same as MxPixelValue
	void*			DeviceInfoPtr; // pointer to a SDeviceInfoN struct -
 									// must be allocated by caller
	int				Reserved3;
	int				Reserved2;
	int				Reserved1;
};

#define STR_TYPE1	1
struct SDeviceInfo1
{
	int				StructSize;	// Initialize to sizeof(SDeviceInfo1)
	int				StructType; // User sets to the N value in SDeviceInfoN (here 1)
	int				ReceptorIndex; // User specifies the receptor index for which
										// device info is requested
	char			ReceptorConfigPath[MAX_STR]; // Returns the receptor config
										// path as determined when the link opens

	char			MacAddress[MIN_STR]; // Returns the MAC address of the device
	char			IpAddress[MIN_STR];  // Returns the IP address of the device
												// (if applicable)
	int				Reserved4;
	int				Reserved3;
	int				Reserved2;
	int				Reserved1;
};

//////////////////////////////////////////////////////////////////////////////
// **>> vip_get_sys_mode <<**
// **>> vip_set_sys_mode <<**
struct SSysMode
{
	int				StructSize;	// Initialize to sizeof(SSysMode)
	int				SystemMode; // This is the system mode specified in the
									// receptor configuration - not currently
									// used by VCP
	int				CurrentReceptorIndex; // The currently selected
											// receptor index (zero-based)
	int				CurrentMode; // The currently selected mode (zero-based)
	int				NumReceptorsOpen; // The number of receptors currently open
										// (return only -
										// ignored by vip_set_sys_mode)
	int				Reserved4;
	int				Reserved3;
	int				Reserved2;
	int				Reserved1;
};

//////////////////////////////////////////////////////////////////////////////
// **>> vip_open_receptor_link <<**
struct SOpenReceptorLink
{
	int				StructSize;	// Initialize to sizeof(SOpenReceptorLink)
	void*			VcpDatPtr;	// Internal Virtual CP use only - must be NULL
	char			RecDirPath[MAX_STR];
	int				TestMode;	// Normally zero - enables frame stamp which
								// overwrites some pixels
	int				DebugMode;	// May be used to turn on debug verbosity
	int				RcptNum;
	int				MaxRcptCount;
	int				MaxModesPerRcpt;
	void*			FgTargetPtr;// Normally zero. Custom use only by FG module
	int				BufferLen;	// Normally zero. Custom use only. Use in
								// conjunction with FgTargetPtr
	int				SubModeBinX;// Normally zero. Custom use only. Specifies
									// submode binning or sample rate in X.
	int				SubModeBinY;// Normally zero. Custom use only. Specifies
									// submode binning or sample rate in Y.
	int				TimeoutBoostSec;// If permitted boosts the timeout
										// used by frame grabber -- max 1000
	int				TimeoutBoostMsVcpCall;	// Allows the caller to increase
										// the time allowed to wait for the
										// call mutex - may prevent HCP_STATE_ERR
	BOOL			CallMutexOverride;	// Overrides the VCP entry point call
										// mutex. This should only set if the
										// VCP is always called from the same thread.
	int				Reserved4;
	int				Reserved3;
	int				Reserved2;
	int				Reserved1;
};

//////////////////////////////////////////////////////////////////////////////
// **>> vip_query_error_info <<**
struct SQueryErrorInfo
{
	int				StructSize;	// Initialize to sizeof(SQueryErrorInfo)
	int				ErrorCode;
	char			ErrMsg[MAX_STR];
	char			ActionStr[MAX_STR];
};

//////////////////////////////////////////////////////////////////////////////
// **>> vip_query_prog_info <<**
// these values are used in the vip_query_prog_info call to specify the
// member of the union
enum {
HCP_U_QPI,
HCP_U_QPIX,
HCP_U_QPIFW,
// Codes for reading image diagnostic data
HCP_U_QPIRCPT,   // Receptor ID
HCP_U_QPIFRAME,  // Frame ID
HCP_U_QPITEMPS,  // Temperature measurement data
HCP_U_QPIVOLTS,  // Voltage measurement data
HCP_U_QPIDIAGDATA,  // Raw diagnostic pixel data
HCP_U_QPI_MAX
};

#define HCP_U_QPI_MASK            (0x7FFF)
// By default, diagnostic data requests always apply to most recent exposed frame
// To request most current frame, OR in the following symbol
#define HCP_U_QPI_CRNT_DIAG_DATA  (0x8000)

// Codes for qpi.Cancellation
#define NO_CANCEL              (0)
#define CANCEL_SUCCEEDED       (1)
#define CANCEL_FAILED          (2)
#define CANCEL_TIMEOUT_PREP    (3)

struct SQueryProgInfo // uType = HCP_U_QPI
{
	int				StructSize;	// Initialize to sizeof(SQueryProgInfo)
	int				NumFrames;
	BOOL			Complete;
	int				NumPulses;
	BOOL			ReadyForPulse;
	int				ProgLimit; // In some cases a limit is provided e.g.
								// for progress bar upper bound
	char			StatusStr[MIN_STR]; // Status message when available
	int				Cancellation;
	int				Prepare;
	int				Reserved2;
	int				Reserved1;
};

// custom use only
struct SQueryProgInfoEx // uType = HCP_U_QPIX
{
	int				StructSize;	// Initialize to sizeof(SQueryProgInfoEx)
	int				NumFrames;
	BOOL			Complete;
	int				NumPulses;
	BOOL			ReadyForPulse;
	void*			FgTargetPtr;// Normally zero. Custom use only by FG module
	int				BufferLen;	// Normally zero. Custom use only. Use in
								// conjunction with FgTargetPtr
	int				Reserved4;
	int				Reserved3;
	int				Reserved2;
	int				Reserved1;
};

struct SQueryProgInfoFw // uType = HCP_U_QPIFW
{
	int				StructSize;	// Initialize to sizeof(SQueryProgInfoFw)
	int				ProgressCurrent;
	int				ProgressLimit;
	BOOL			Complete;
};

struct SQueryProgInfoRcpt // uType = HCP_U_QPIRCPT
{
	int				StructSize;	// Initialize to sizeof(SQueryProgInfoRc)
	int				PanelType;
	int				FwVersion;
	WORD            BoardSNbr[3]; // 48 bits
	WORD            ReservedWd;   // Restore word alignment
	int             ReservedRcpt2;
	int             ReservedRcpt1;
};

// Codes for FrameType in SQueryProgInfoFrame
enum RAD_FRAME_TYPE
{
	RAD_FRAME_IDLE,
	RAD_FRAME_OFFSET,
	RAD_FRAME_EXPOSED,
	RAD_FRAME_DISABLED,
	RAD_FRAME_OFSDIS,
	RAD_FRAME_OTHER,
	RAD_FRAME_CANCEL,
	RAD_FRAME_TIMEOUT_PREP
};

struct SQueryProgInfoFrame // uType = HCP_U_QPIFRAME
{
	int				StructSize;	// Initialize to sizeof(SQueryProgInfoFr)
	int             RcptFrameId;
	int				Exposed;
	unsigned long   AcqTime;
	int             FrameType;
	int             Cancellation;
	int             Prepare;
	int             ReservedFr1;
};

struct SQueryProgInfoTemps // uType = HCP_U_QPITEMPS
{
	int				StructSize;	// Initialize to sizeof(SQueryProgInfoTh)
	int             NumSensors;
	float           Celsius[16];
};

struct SQueryProgInfoVolts // uType = HCP_U_QPIVOLTS
{
	int				StructSize;	// Initialize to sizeof(SQueryProgInfoVo)
	int             NumSensors;
	float           Volts[16];
};

struct SDiagData
{
	int             StructSize;			//  0
	unsigned long   AcqTime;		    //  4
	// The following members are in the same order as the raw pixel data,
	// starting from pixel column 8:
	WORD            Protocol;			//  8  (pixel  8)
	WORD            PanelType;			// 10  (pixel  9)
	WORD            FwVersion;			// 12  (pixel 10)
	WORD            Exposed;			// 14  (pixel 11)
	WORD            RcptFrameId;		// 16  (pixel 12)
	WORD            BoardSNbr[3];		// 18  (pixel 13..15)
	WORD            TemperatureData[8];	// 24  (pixel 16..23)
	WORD            VoltageData[16]; 	// 40  (pixel 24..39)
	WORD            Reserved[28];		// 72
};

union  UQueryProgInfo
{
	SQueryProgInfo		qpi;
	SQueryProgInfoEx	qpix;
	SQueryProgInfoFw	qpifw;

	SQueryProgInfoRcpt  qpircpt;
	SQueryProgInfoFrame qpiframe;
	SQueryProgInfoTemps qpitemps;
	SQueryProgInfoVolts qpivolts;
	SDiagData           qpidiag;
};


//////////////////////////////////////////////////////////////////////////////
// **>> vip_select_receptor <<**
// The following are used in calls to vip_select_receptor(recSel).
// The recSel parameter can be loaded with the mode required so that
// a new receptor and mode can be selected together. If the call is made with
// just the receptor number then mode 0 is selected.
#define		SELREC_REC_MASK		0x00FF
#define		SELREC_MODE_MASK	0xFF00
#define		SELREC_MODE_SHIFT	8
// eg recSel = recNum + (modeNum << SELREC_MODE_SHIFT);

//////////////////////////////////////////////////////////////////////////////
// **>> vip_set_cal_acq_data <<**
// use these values in the DataType below
enum {
HCP_CALACQ_OFST,
HCP_CALACQ_GAIN,
HCP_CALACQ_ANLG, // analog offset
HCP_CALACQ_OFST_HI, // CB offset HI_GAIN state
HCP_CALACQ_OFST_LO, // CB offset LO_GAIN state
HCP_CALACQ_GAIN_HI, // CB gain HI_GAIN state
HCP_CALACQ_GAIN_LO, // CB gain LO_GAIN state
HCP_CALACQ_EXT0_HI, // CB extended gain cal - ofst - HI_GAIN (DYN)
HCP_CALACQ_EXT0_LO, // CB extended gain cal - ofst - LO_GAIN (FLG)
HCP_CALACQ_EXT1_HI, // CB extended gain cal - xry1 - HI_GAIN (DYN)
HCP_CALACQ_EXT1_LO, // CB extended gain cal - xry1 - LO_GAIN (FLG)
HCP_CALACQ_EXT2_HI, // CB extended gain cal - xry2 - HI_GAIN (DYN)
HCP_CALACQ_EXT2_LO, // CB extended gain cal - xry2 - LO_GAIN (FLG)
HCP_CALACQ_EXT3_HI, // CB extended gain cal - xry3 - HI_GAIN (DYN)
HCP_CALACQ_EXT3_LO, // CB extended gain cal - xry3 - LO_GAIN (FLG)
HCP_CALACQ_GN_RATIO,// CB gain ratio cal
HCP_CALACQ_MAX,
HCP_CALACQ_CALEND = 100	// no acquisition involved - used at end of extended
						// gain cal to force the calibration call and retriev
						// the HL_ratio
};

struct SCalAcqData
{
	int				StructSize;		// Initialize to sizeof(SCalAcqData)
	int				DataType;		// offset, gain, extended gain etc
	int				DataByteSize;	// size of data (int=4; WORD=2)
	int				FrmX;
	int				FrmY;
	void*			Data;
	int				NumFrames;
	float			HLratio;
	int				Reserved1;
};

//////////////////////////////////////////////////////////////////////////////
// **>> vip_set_debug <<**
// the following are used by vip_set_debug(enable) and relatedly to specify
// type of debug required
#define		HCP_DBG_OFF		0
#define		HCP_DBG_ON		1
#define		HCP_DBG_ON_FLSH	2
#define		HCP_DBG_ON_DLG	3



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// The following don't really need to be in hcpsundries - not needed for
// interface to VirtCp.dll, but kept here for now to avoid disruption

//////////////////////////////////////////////////////////////////////////////
struct SCallStatus
{
	int				StructSize;		// Initialize to sizeof(SCallStatus)
	SCallStatus*	LinkedCS;		// pointer to a CallStatus structure
					// (to allow a linked list if required.) Initialize to NULL
	int				ErrorCode;		// initialize to VIP_NO_ERR
	int				State;			// initialize to IDLE
	int				Timeout;		// initialize to 5000 (ms)
					// (Implementation not planned initially)
	void*			SetStatePtr;	// function pointer to the 'SetState' function
					// - initialize to the address of the SetState function.
	void*			GetFuncPtr;		// function pointer to the 'GetFunc' function
					// - initialize to the address of the GetFunc function.
	char			ErrorMsg[MAX_STR];	// (char[256] - initialize to "")
	char			ActionStr[MAX_STR];	// (char[256] - initialize to "")
	void*			RelLogIt;		// function pointer to the HcpLogIt
	long			Reserved1;
	long			Reserved2;
};


//////////////////////////////////////////////////////////////////////////////
enum HcpDllRef {
HCP_DLL_REF_AC,
HCP_DLL_REF_RC,
HCP_DLL_REF_CR,
HCP_DLL_REF_CA,
HCP_DLL_REF_MC,
HCP_DLL_REF_IO,
HCP_DLL_REF_RP,
HCP_DLL_REF_FG,
HCP_DLL_REF_MAX
};

//////////////////////////////////////////////////////////////////////////////
#endif // VIP_INC_SUNDRYSTRUCTS_H
