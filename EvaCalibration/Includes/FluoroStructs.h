

//////////////////////////////////////////////////////////////////////////////
// Filename:	fluorostructs.h
// Description:	This file provides structures and related enums used by the
//				fluoro (frame grabber) interface.
// Copyright:	Varian Medical Systems
//				All Rights Reserved
//				Varian Proprietary
//////////////////////////////////////////////////////////////////////////////

#ifndef VIP_INC_FLUOROSTRUCTS_H
#define VIP_INC_FLUOROSTRUCTS_H

// this enum provides a reference for structure types used in calls
// vip_fluoro_get_prms(..) and vip_fluoro_set_prms(..)
enum HcpFluoroStruct {
HCP_FLU_SYS_PRMS,		// specifies pointer to a SFluoroSysPrms
HCP_FLU_MODE_PRMS,		// specifies pointer to a SFluoroModePrms
HCP_FLU_GRAB_PRMS,		// specifies pointer to a SGrbPrms
HCP_FLU_SEQ_PRMS,		// specifies pointer to a SSeqPrms
HCP_FLU_ACQ_PRMS,		// specifies pointer to a SAcqPrms
HCP_FLU_LIVE_PRMS,		// specifies pointer to a SLivePrms
HCP_FLU_THRESH_PRMS,	// specifies pointer to a SThreshSelect
HCP_FLU_STATS_PRMS,		// specifies pointer to a SSeqStats
HCP_FLU_REC_PRMS,		// not valid
HCP_FLU_TIME_INIT,		// specifies pointer to a SSeqTimer
HCP_FLU_INDEX_RANGE,	// specifies pointer to a SIndexRange
HCP_FLU_MAX
};

// this enum is used in the VideoStatus member of the SLivePrms structure
enum HcpFluoroStatus {
HCP_REC_IDLE,
HCP_REC_PAUSED,
HCP_REC_GRABBING,
HCP_REC_RECORDING,
HCP_REC_MAX
};

// this enum is used to specify corrections type requested by the user
// set the CorrType in SAcqPrms
// only used in fluoro modes, ignored in rad mode
enum HcpCorrType {
HCP_CORR_NONE,
HCP_CORR_STD,
HCP_CORR_MAX
};

// this enum is used by the vip_fluoro_get_event_name
enum HcpFgEvent {
HCP_FG_FRM_TO_DISP,
HCP_FG_FRM_GRBD,
HCP_FG_STRT_REC,
HCP_FG_HOST_SYNC,
HCP_FG_RAD_RESET_RELEASE, // Special for rad panel MVH 20060908
HCP_FG_RAD_RESET_ASSERT,  // Additional for rad panel MVH 20060922
HCP_FG_EVENT_MAX
};


// this structure is used in the vip_fluoro_init_sys(..) call
struct SFluoroSysPrms
{
	int   StructSize;
	int   FileRev;				// default to zero
	char* InitStr;				// =NULL default
	int   TestMode;
	int   RecModeControl;
	int	  TimeoutBoostMs;
	int	  NumVideoThreads;		// returned by device object - internal use only
	int	  Reserved3;
	int	  Reserved2;
	int	  Reserved1;
};

// this structure is used in the vip_fluoro_init_mode(..) call
struct SFluoroModePrms
{
	int   StructSize;
	int   FrameX;
	int   FrameY;
	int   BinX;
	int   BinY;
	int   RecType;				// =0 default
	float FrmRate;				// =0.0 default (if not needed)
	int   UserSync;				// =0 default
	int   TrigSrc;				// =0 default
	int   TrigMode;				// =0 default
	void* GrabPrms;				// =NULL default - not implemented
	void* TimingPrms;			// =NULL default - not implemented
	char* FilePath;				// =NULL default path to cnfg file
};

// this structure - currently unimplemented - is intended to be used to
// initialize the grab buffers - i.e. buffers written directly by the
// frame grabber
struct SGrbPrms
{
	int   StructSize;
	int   NumBuffers;
	int   GrbX;
	int   GrbY;
	int   ByteDepth;
};

// may be used to get or set parameters using vip_fluoro_set_prms
#define SNUG_MEM_FLAG 44
struct SSeqPrms
{
	int   StructSize;
	int   NumBuffers;			// number request - a smaller number may be allocated
								// and returned at this same location
	int   SeqX;					// dflt = 0 interpret = grbX
	int   SeqY;					// dflt = 0 interpret = grbY
	int   SumSize;				// dflt = 0 interpret =1
	int   SampleRate;			// dflt = 0 interpret =1
	int   BinFctr;				// dflt = 0 interpret =1
	int   StopAfterN;			// dflt = 0
	int   OfstX;				// dflt = 0
	int   OfstY;				// dflt = 0
	int	  RqType;				// must be zero
	int   SnugMemory;			// Arbitrary Value -- implies that the VCP will  
								// handle memory in its normal manner (once allocated, 
								// it stays allocated until VirtCp.dll detaches OR
								// CLSLNK_RELMEM flag set on close link).
								// SnugMemory=SNUG_MEM_FLAG -- implies that VCP will 
								// deallocate any excess memory above that needed 
								// to keep NumBuffers available.
};

// may be used when starting a grab to customize ot entire structure zeroed
// Note that a pointer to a SLiveParams structure is returned. Access
// to this normally should be handled more safely using a call to
// vip_fluoro_get_prms(..).
//
// Values for StartUp:
// IDLE, --> (Zero default) is converted to GRABBING
// PAUSED,
// GRABBING,
// RECORDING
struct SAcqPrms
{
	int   StructSize;
	int   StartUp;				// dflt = 0 --> GRABBING (fluoro)
								//				PAUSED (rad)
	int   ReqType;				// internal use only - must be zero
	int   CorrType;				// dflt = 0 --> no corrections. Use to
								// set required corrections in fluoro modes
								// see HcpCorrType enum
	void* CorrFuncPtr;			// dflt = NULL; user should not set this
	void* ThresholdSelect;		// dflt = NULL; pointer to a structure
								// set threshold select parameters
	double* CopyBegin;			// dflt = NULL
	double* CopyEnd;			// dflt = NULL
	int   ArraySize;			// dflt = 0
	int   MarkPixels;			// dflt = 0 --> off
	int   FrameErrorTolerance;		// dflt = 0
	void* LivePrmsPtr;			// pointer to the SLivePrms structure is returned here
};

// A structure of this type is updated during the acquisition.
//
// Values for VideoStatus:
// IDLE,
// PAUSED,
// GRABBING,
// RECORDING
struct SLivePrms
{
	int   StructSize;
	volatile int	NumFrames;
	volatile int	BufIndex; // index to buffer currently most recent for display
	volatile void*	BufPtr;	// pointer to buffer currently most recent for display
	volatile int	VideoStatus;
	volatile int	ErrorCode;
};

// not implemented currently
struct SThreshSelect
{
	int   StructSize;
	void* TimerPtr;
	int   ThreshHi;
	int	  Left;  // ROI definition..
	int	  Top;
	int	  Right;
	int	  Bottom;
};

// used to retrieve sequence statistics via a call to
// vip_fluoro_get_prms(..)
struct SSeqStats
{
	int   StructSize;
	int   SmplFrms;
	int   HookFrms;
	int   CaptFrms;
	int   HookOverrun;
	int   StartIdx;
	int   EndIdx;
	float CaptRate;
};


// used to initialize the time and retrieve the pointer to the
// CTickCounter
struct SSeqTimer
{
	int   StructSize;
	void* SeqTimerPtr;
};

// used to determine the range indices which reference available images
// stored on the receptor 
struct SIndexRange
{
	int   StructSize;
	unsigned long   StartIndex;
	unsigned long   EndIndex;
	int   ImgSizeX;
	int   ImgSizeY;
};


#endif // VIP_INC_FLUOROSTRUCTS_H
