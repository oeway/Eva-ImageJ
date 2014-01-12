#ifndef _ACQUISITION_H
#define _ACQUISITION_H

//////////////////////////////////////////////////////
// IMPORTANT:
// When implementing the XISL dll into a .net environment some datatypes might have to be changed to to size definition:
//
// e.g. 
// XISL dll: long has 4 byte
// .net: int has 4 byte, long has 8 byte
// so long has to be important as int
// 
// XISL dll: char has 1 byte
// .net: char has 2 bytes
//
//////////////////////////////////////////////////////

#define DATASHORT               2 //2 byte integer
#define DATALONG                4 //4 byte integer
#define DATAFLOAT               8 //8 byte double
#define DATASIGNED              16 // signed

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

//#define __X64 // Define this for the 64bit Driver Version
#ifdef __X64
typedef void* ACQDESCPOS;
#else
typedef UINT ACQDESCPOS;
#endif

#ifdef __linux__
	typedef int HACQDESC;
#else
	// Windows
	/** 
	* AcquisitionDesc defines a data structure that is 
	* used by all functions of XISL. It contains all 
	* required parameters for the acquisition.
	* Access to the data fields is only possible via 
	* the XISL API functions. HACQDESC defines a HANLDE to the acquisition descriptor.
	* @see AcquisitionDesc
	**/
	typedef HANDLE HACQDESC;
    #ifdef _DLL_EXPORT
		#define _DLL_API __declspec(dllexport) 
    #else
		#define _DLL_API __declspec(dllimport) 
    #endif //_ACQDEFS_H

    #define HIS_RETURN _DLL_API UINT WINAPI
    
#endif


#define WINRESTSIZE					34
#define WINHARDWAREHEADERSIZE		32

#define DETEKTOR_DATATYPE_18BIT 16
#define MAX_GREY_VALUE_18BIT  262144

#ifndef __GBIF_Types
#define __GBIF_Types
	#define GBIF_IP_MAC_NAME_CHAR_ARRAY_LENGTH		16
	#define GBIF_STRING_DATATYPE unsigned char
	#define GBIF_STRING_DATATYPE_ELTEC char
#endif


typedef struct
{	
	GBIF_STRING_DATATYPE	ucMacAddress[GBIF_IP_MAC_NAME_CHAR_ARRAY_LENGTH];			// unsigned since the adress components can be higher than 128
	GBIF_STRING_DATATYPE	ucIP[GBIF_IP_MAC_NAME_CHAR_ARRAY_LENGTH];
	GBIF_STRING_DATATYPE	ucSubnetMask[GBIF_IP_MAC_NAME_CHAR_ARRAY_LENGTH];
	GBIF_STRING_DATATYPE	ucGateway[GBIF_IP_MAC_NAME_CHAR_ARRAY_LENGTH];
	GBIF_STRING_DATATYPE	ucAdapterIP[GBIF_IP_MAC_NAME_CHAR_ARRAY_LENGTH];
	GBIF_STRING_DATATYPE	ucAdapterMask[GBIF_IP_MAC_NAME_CHAR_ARRAY_LENGTH];

	DWORD					dwIPCurrentBootOptions;
	CHAR 					cManufacturerName[32];										// PerkinElmer
	CHAR					cModelName[32];												// GBIF
	CHAR					cGBIFFirmwareVersion[32];
	CHAR					cDeviceName[16];

} GBIF_DEVICE_PARAM;

// 128Byte in total
typedef struct
{	
	char	cDetectorType[32];															// e.g. XRD 0822 AO 14
	char	cManufacturingDate[8];														// e.g. 06-2010
	char	cPlaceOfManufacture[8];														// e.g. DE
	char	cDummy[80];
} GBIF_Detector_Properties;

#define HIS_GbIF_FIRST_CAM 0
#define HIS_GbIF_IP 1
#define HIS_GbIF_MAC 2
#define HIS_GbIF_NAME 3

#define HIS_GbIF_IP_STATIC	1
#define HIS_GbIF_IP_DHCP	2
#define HIS_GbIF_IP_LLA		4

//*/

typedef struct
{
	DWORD	dwPROMID;
	DWORD	dwHeaderID;
	BOOL	bAddRow;
	BOOL	bPwrSave;
	DWORD	dwNrRows;
	DWORD	dwNrColumns;
	DWORD	dwZoomULRow;
	DWORD	dwZoomULColumn;
	DWORD	dwZoomBRRow;
	DWORD	dwZoomBRColumn;
	DWORD	dwFrmNrRows;
	DWORD	dwFrmRowType;
	DWORD	dwFrmFillRowIntervalls;
	DWORD	dwNrOfFillingRows;
	DWORD	dwDataType;
	DWORD	dwDataSorting;
	DWORD	dwTiming;
	DWORD	dwAcqMode;
	DWORD	dwGain;
	DWORD	dwOffset;
	DWORD	dwAccess;
	BOOL	bSyncMode;
	DWORD	dwBias;
	DWORD	dwLeakRows;
} CHwHeaderInfo;

typedef struct
{
	WORD	wHeaderID;		// 0
	WORD	wPROMID;		// 1
	WORD	wResolutionX;	// 2
	WORD	wResolutionY;	// 3
	WORD	wNrRows;		// 4
	WORD	wNrColumns;		// 5
	WORD	wZoomULRow;		// 6
	WORD	wZoomULColumn;	// 7
	WORD	wZoomBRRow;		// 8
	WORD	wZoomBRColumn;	// 9
	WORD	wFrmNrRows;		// A
	WORD	wFrmRowType;	// B
	WORD	wRowTime;		// C << 6
	WORD	wClock;			// D << 6
	WORD	wDataSorting;	// E
	WORD	wTiming;		// F
	WORD	wGain;			// 10
	WORD	wLeakRows;		// 11
	WORD	wAccess;		// 12
	WORD	wBias;			// 13
	WORD	wUgComp;		// 14
	WORD	wCameratype;	// 15
	WORD	wFrameCnt;		// 16
	WORD	wBinningMode;	// 17
	WORD	wRealInttime_milliSec;	// 18 
	WORD	wRealInttime_microSec;	// 19
	WORD	wStatus;		// 1A
	WORD	wCommand1;		// 1B
	WORD	wCommand2;		// 1C
	WORD	wCommand3;		// 1D
	WORD	wCommand4;		// 1E
	WORD	wDummy;			// 1F
}	CHwHeaderInfoEx;


typedef struct
{
	WORD FileType;			// File ID (0x7000)
	WORD HeaderSize;		// Size of this file header in Bytes
	WORD HeaderVersion;		// yy.y
	ULONG FileSize;			// Size of the whole file in Bytes
	WORD ImageHeaderSize;	// Size of the image header in Bytes
	WORD ULX, ULY, BRX, BRY;// bounding rectangle of the image
	WORD NrOfFrames;		// self explanatory
	WORD Correction;		// 0 = none, 1 = offset, 2 = gain, 4 = bad pixel, (ored)
	double IntegrationTime;	// frame time in microseconds
	WORD TypeOfNumbers;		// short, long integer, float, signed/unsigned, inverted, 
							// fault map, offset/gain correction data, badpixel correction data
	BYTE x[WINRESTSIZE];		// fill up to 68 byte
} WinHeaderType;

typedef struct
{
	DWORD	dwPROMID;
	char	strProject[6];		// project / cam-nr
	char	strSystemused[3];	//
	char	strPrefilter[9];	// used filter
	float	fKVolt;				// 
	float	fAmpere;			//
	WORD	n_avframes;		// average count
		
} WinImageHeaderType;

typedef struct
{
	unsigned char	wTiming;		//	Timing und Triggermode
	unsigned char	wValue0;		// 
	unsigned char	wValue1;
	unsigned char	wValue2;
	unsigned char	wValue3;
	unsigned char	wValue4;
	unsigned char	wValue5;
	unsigned char	wValue6;
} FPGAType;					// 8 Byte werden bertragen


HIS_RETURN Acquisition_Init(HACQDESC *phAcqDesc,
					  DWORD dwChannelType, int nChannelNr,
					  BOOL bEnableIRQ, 
					  UINT Rows, UINT Columns, 
					  UINT dwSortFlags,
					  BOOL bSelfInit, BOOL bAlwaysOpen
					  );


HIS_RETURN Acquisition_SetCallbacksAndMessages(HACQDESC pAcqDesc,
					  HWND hWnd,
					  UINT dwErrorMsg, UINT dwLoosingFramesMsg,
					  void (CALLBACK *lpfnEndFrameCallback)(HACQDESC), 
					  void (CALLBACK *lpfnEndAcqCallback)(HACQDESC)
					  );
HIS_RETURN Acquisition_EnumSensors(UINT *pdwNumSensors, BOOL bEnableIRQ, BOOL bAlwaysOpen);
HIS_RETURN Acquisition_GetNextSensor(ACQDESCPOS *Pos, 
					  HACQDESC *phAcqDesc);
HIS_RETURN Acquisition_GetCommChannel(HACQDESC pAcqDesc, UINT *pdwChannelType, int *pnChannelNr);
HIS_RETURN Acquisition_DefineDestBuffers(HACQDESC pAcqDesc, unsigned short *pProcessedData, UINT nFrames, UINT nRows, UINT nColumns);
HIS_RETURN Acquisition_Acquire_Image(HACQDESC pAcqDesc, UINT dwFrames, UINT dwSkipFrms, UINT dwOpt, unsigned short *pwOffsetData, DWORD *pdwGainData, DWORD *pdwPxlCorrList);
HIS_RETURN Acquisition_Acquire_Image_Ex(HACQDESC hAcqDesc, UINT dwFrames, UINT dwSkipFrms, UINT dwOpt, 
									  unsigned short *pwOffsetData, UINT dwGainFrames, unsigned short *pwGainData,
									  unsigned short *pwGainAvgData, DWORD *pdwGainData, DWORD *pdwPxlCorrList);
HIS_RETURN Acquisition_Abort(HACQDESC hAcqDesc);
HIS_RETURN Acquisition_SetFPGACameraMode(HACQDESC hAcqDesc, FPGAType FPGACommand, BOOL bInverse);
HIS_RETURN Acquisition_SetCameraMode(HACQDESC hAcqDesc, UINT dwMode);
HIS_RETURN Acquisition_Acquire_OffsetImage(HACQDESC hAcqDesc, unsigned short *pOffsetData, UINT nRows, UINT nCols, UINT nFrames);
HIS_RETURN Acquisition_Acquire_OffsetImage_Ex(HACQDESC hAcqDesc, unsigned short *pOffsetData, UINT nRows, UINT nCols, UINT nFrames, UINT dwOpt);
HIS_RETURN Acquisition_Acquire_GainImage(HACQDESC hAcqDesc, WORD *pOffsetData, DWORD *pGainData, UINT nRows, UINT nCols, UINT nFrames);
HIS_RETURN Acquisition_Acquire_GainImage_Ex(HACQDESC hAcqDesc, WORD *pOffsetData, DWORD *pGainData, UINT nRows, UINT nCols, UINT nFrames, UINT dwOpt);
HIS_RETURN Acquisition_CreateGainMap(WORD *pGainData, WORD *pGainAVG, int nCount, int nFrame );
HIS_RETURN Acquisition_CreatePixelMap(WORD *pData, int nDataRows, int nDataColumns, int *pCorrList, int *nCorrListSize);
HIS_RETURN Acquisition_DoOffsetCorrection(WORD *pSource, WORD *pDest, WORD *pOffsetData, int nCount);
HIS_RETURN Acquisition_DoOffsetGainCorrection(WORD *pSource, WORD *pDest, WORD *pOffsetData, DWORD *pGainData, int nCount);
HIS_RETURN Acquisition_DoOffsetGainCorrection_Ex(WORD *pSource, WORD *pDest, WORD *pOffsetData, WORD *pGainData, 
															WORD *pGainAVG, int nCount, int nFrame);	//	19.09.02

HIS_RETURN Acquisition_DoOffsetCorrection32(unsigned long *pSource, unsigned long *pDest, unsigned long *pOffsetData, int nCount); // val 20070124
HIS_RETURN Acquisition_DoOffsetGainCorrection32(unsigned long *pSource, unsigned long *pDest, unsigned long *pOffsetData, unsigned long *pGainData, int nCount);// val 20070124
HIS_RETURN Acquisition_DoOffsetGainCorrection_Ex32(unsigned long *pSource, unsigned long *pDest, unsigned long *pOffsetData, unsigned long *pGainData,
							unsigned long *pGainAVG, int nCount, int nFrame); // val 20070124
HIS_RETURN Acquisition_CreateGainMap32(unsigned long *pGainData, unsigned long *pGainAVG, int nCount, int nFrame );


HIS_RETURN Acquisition_DoPixelCorrection(WORD *pData, int *pCorrList);
HIS_RETURN Acquisition_IsAcquiringData(HACQDESC hAcqDesc);
HIS_RETURN Acquisition_Close(HACQDESC hAcqDesc);
HIS_RETURN Acquisition_CloseAll();
HIS_RETURN Acquisition_SetReady(HACQDESC hAcqDesc, BOOL bFlag);
HIS_RETURN Acquisition_GetReady(HACQDESC hAcqDesc);
HIS_RETURN Acquisition_GetErrorCode(HACQDESC hAcqDesc, DWORD *dwHISError, DWORD *dwBoardError);
HIS_RETURN Acquisition_GetConfiguration(HACQDESC hAcqDesc, 
					UINT *dwFrames, UINT *dwRows, UINT *dwColumns, UINT *dwDataType,
					UINT *dwSortFlags, BOOL *bIRQEnabled, DWORD *dwAcqType, DWORD *dwSystemID,
					DWORD *dwSyncMode, DWORD *dwHwAccess);
HIS_RETURN Acquisition_GetIntTimes(HACQDESC hAcqDesc, double *dblIntTime, int *nIntTimes);
HIS_RETURN Acquisition_GetWinHandle(HACQDESC hAcqDesc, HWND *hWnd);
HIS_RETURN Acquisition_GetActFrame(HACQDESC hAcqDesc, DWORD *dwActAcqFrame, DWORD *dwActSecBuffFrame);
#ifdef __X64
HIS_RETURN Acquisition_SetAcqData(HACQDESC hAcqDesc, void *AcqData);
HIS_RETURN Acquisition_GetAcqData(HACQDESC hAcqDesc, void **VoidAcqData);
#else
HIS_RETURN Acquisition_SetAcqData(HACQDESC hAcqDesc, DWORD dwAcqData);
HIS_RETURN Acquisition_GetAcqData(HACQDESC hAcqDesc, DWORD *dwAcqData);
#endif // __X64
HIS_RETURN Acquisition_GetHwHeaderInfo(HACQDESC hAcqDesc, CHwHeaderInfo *pInfo);
HIS_RETURN Acquisition_SetFrameSync(HACQDESC hAcqDesc);
HIS_RETURN Acquisition_SetFrameSyncMode(HACQDESC hAcqDesc, DWORD dwMode);
HIS_RETURN Acquisition_SetTimerSync(HACQDESC hAcqDesc, DWORD *dwCycleTime);
HIS_RETURN Acquisition_AbortCurrentFrame(HACQDESC hAcqDesc);
HIS_RETURN Acquisition_SetCorrData(HACQDESC hAcqDesc, unsigned short *pwOffsetData, DWORD *pdwGainData, DWORD *pdwPxlCorrList);
HIS_RETURN Acquisition_SetCorrData_Ex(HACQDESC hAcqDesc, unsigned short *pwOffsetData, unsigned short *pwGainData, 
									unsigned short *pwGainAvgData, UINT nGainFrames,
									DWORD *pdwGainData, DWORD *pdwPxlCorrList);
HIS_RETURN Acquisition_GetCorrData(HACQDESC hAcqDesc, unsigned short **ppwOffsetData, DWORD **ppdwGainData, DWORD **ppdwPxlCorrList);
HIS_RETURN Acquisition_GetCorrData_Ex(HACQDESC hAcqDesc, unsigned short **ppwOffsetData, unsigned short **ppwGainData, 
									unsigned short **ppwGainAvgData, UINT **nGainFrames, DWORD **pdwGainData, DWORD **pdwPxlCorrList);
HIS_RETURN Acquisition_SetCameraGain(HACQDESC hAcqDesc, WORD wMode);
HIS_RETURN Acquisition_SetFrameSyncTimeMode(HACQDESC hAcqDesc, unsigned int uiMode, unsigned int dwDelayTime);

HIS_RETURN Acquisition_Acquire_GainImage_Ex_ROI(HACQDESC hAcqDesc, WORD *pOffsetData, DWORD *pGainData, UINT nRows, UINT nCols, UINT nFrames, UINT dwOpt,UINT uiULX,UINT uiULY,UINT uiBRX,UINT uiBRY,UINT uiMode);
HIS_RETURN Acquisition_Acquire_Image_PreloadCorr(HACQDESC hAcqDesc, UINT dwFrames, UINT dwSkipFrms, UINT dwOpt);
HIS_RETURN Acquisition_Acquire_OffsetImage_PreloadCorr(HACQDESC hAcqDesc, WORD *pwOffsetData, UINT nRows, UINT nColumns, UINT nFrames, UINT dwOpt);
HIS_RETURN Acquisition_GetHwHeader(HACQDESC hAcqDesc,unsigned char* pData,unsigned int uiSize);
HIS_RETURN Acquisition_Acquire_GainImage_Ex_ROI_PreloadCorr(HACQDESC hAcqDesc, DWORD *pGainData, UINT nRows, UINT nCols, UINT nFrames, UINT dwOpt,UINT uiULX,UINT uiULY,UINT uiBRX,UINT uiBRY,UINT uiMode);
HIS_RETURN Acquisition_Acquire_GainImage_PreloadCorr(HACQDESC hAcqDesc, DWORD *pGainData, UINT nRows, UINT nCols, UINT nFrames);

HIS_RETURN Acquisition_SetCameraBinningMode(HACQDESC hAcqDesc, WORD wMode);
HIS_RETURN Acquisition_GetCameraBinningMode(HACQDESC hAcqDesc, WORD* wMode);
HIS_RETURN Acquisition_ResetFrameCnt(HACQDESC hAcqDesc);
HIS_RETURN Acquisition_GetLatestFrameHeader(HACQDESC hAcqDesc, CHwHeaderInfo *pInfo, CHwHeaderInfoEx *pInfoEx);
HIS_RETURN Acquisition_GetHwHeaderInfoEx(HACQDESC hAcqDesc, CHwHeaderInfo *pInfo ,CHwHeaderInfoEx *pInfoEx);
HIS_RETURN Acquisition_SetCameraTriggerMode(HACQDESC hAcqDesc, WORD wMode);
HIS_RETURN Acquisition_GetCameraTriggerMode(HACQDESC hAcqDesc, WORD *wMode);

HIS_RETURN Acquisition_SetRotationAngle(HACQDESC hAcqDesc, long lRotAngle); // FG-E only can be -90 | 0 | 90
HIS_RETURN Acquisition_GetRotationAngle(HACQDESC hAcqDesc, long* lRotAngle);


HIS_RETURN Acquisition_GbIF_Init(					HACQDESC *phAcqDesc,
													int nChannelNr,
													BOOL bEnableIRQ, 
													UINT uiRows, UINT uiColumns, 
													BOOL bSelfInit, BOOL bAlwaysOpen,
													long lInitType, 
													GBIF_STRING_DATATYPE* ucAddress
												  );

HIS_RETURN Acquisition_GbIF_GetDeviceList(GBIF_DEVICE_PARAM* pGBIF_DEVICE_PARAM, int nDeviceCnt);
HIS_RETURN Acquisition_GbIF_GetDevice(GBIF_STRING_DATATYPE* ucAddress, DWORD dwAddressType, GBIF_DEVICE_PARAM* pDevice);
HIS_RETURN Acquisition_GbIF_GetDeviceCnt(long* plNrOfboards);

//HIS_RETURN Acquisition_GbIF_UploadPKIFirmware(			GBIF_STRING_DATATYPE* cMacAddress,
//													void* pBitStream, long lBitStreamLength
//										);

HIS_RETURN Acquisition_GbIF_SetConnectionSettings(	GBIF_STRING_DATATYPE* cMAC,
													unsigned long ulBootOptions,
													GBIF_STRING_DATATYPE* cDefIP,
													GBIF_STRING_DATATYPE* cDefSubNetMask,
													GBIF_STRING_DATATYPE* cStdGateway
												);

HIS_RETURN Acquisition_GbIF_GetConnectionSettings(	GBIF_STRING_DATATYPE* ucMAC,
													unsigned long* ulBootOptions,
													GBIF_STRING_DATATYPE* ucDefIP,
													GBIF_STRING_DATATYPE* ucDefSubNetMask,
													GBIF_STRING_DATATYPE* ucStdGateway
												);

HIS_RETURN Acquisition_GbIF_GetPacketDelay(			HACQDESC hAcqDesc,
													long* lPacketdelay
												);

HIS_RETURN Acquisition_GbIF_SetPacketDelay(			HACQDESC hAcqDesc,
													long lPacketdelay
											);

HIS_RETURN Acquisition_GbIF_ForceIP(				GBIF_STRING_DATATYPE* cMAC,
													GBIF_STRING_DATATYPE* cDefIP,
													GBIF_STRING_DATATYPE* cDefSubNetMask,
													GBIF_STRING_DATATYPE* cStdGateway
									);

HIS_RETURN Acquisition_GbIF_GetFilterDrvState(		HACQDESC hAcqDesc
											  );

HIS_RETURN Acquisition_GbIF_CheckNetworkSpeed(	HACQDESC hAcqDesc, WORD* wTiming, long* lPacketDelay, long lMaxNetworkLoadPercent);
HIS_RETURN Acquisition_GbIF_GetDetectorProperties(HACQDESC hAcqDesc, GBIF_Detector_Properties* pDetectorProperties);
HIS_RETURN Acquisition_GbIF_GetDeviceParams(HACQDESC hAcqDesc, GBIF_DEVICE_PARAM* pDevice);

HIS_RETURN Acquisition_ActivateServiceMode(HACQDESC hAcqDesc, BOOL bActivate);

HIS_RETURN Acquisition_SetCameraROI(HACQDESC hAcqDesc, unsigned short usActivateGrp); // val 2010-05-12
HIS_RETURN Acquisition_GetCameraROI(HACQDESC hAcqDesc, unsigned short *usActivateGrp); // val 2010-05-17

HIS_RETURN Acquisition_SetTriggerOutSignalOptions(HACQDESC hAcqDesc,unsigned short usTiggerOutSignalMode,
																	unsigned short usEP_SeqLength, 
																	unsigned short usEP_FirstBrightFrm,
																	unsigned short usEP_LastBrightFrm,
																	unsigned short usEP_Delay1,
																	unsigned short usEP_Delay2,
																	unsigned short usDDD_Delay,
																	int iTriggerOnRisingEdgeEnable,
																	int iSaveAsDefault
																	); // val 2010-05-12


#ifdef __cplusplus
}
#endif  /* __cplusplus */

//error codes

#define HIS_ALL_OK							0
#define HIS_ERROR_MEMORY					1
#define HIS_ERROR_BOARDINIT					2
#define HIS_ERROR_NOCAMERA					3
#define HIS_ERROR_CORRBUFFER_INCOMPATIBLE	4
#define HIS_ERROR_ACQ_ALREADY_RUNNING		5
#define HIS_ERROR_TIMEOUT					6
#define HIS_ERROR_INVALIDACQDESC			7
#define HIS_ERROR_VXDNOTFOUND				8
#define HIS_ERROR_VXDNOTOPEN				9
#define HIS_ERROR_VXDUNKNOWNERROR			10
#define HIS_ERROR_VXDGETDMAADR				11
#define HIS_ERROR_ACQABORT					12
#define HIS_ERROR_ACQUISITION				13
#define HIS_ERROR_VXD_REGISTER_IRQ			14
#define HIS_ERROR_VXD_REGISTER_STATADR		15
#define HIS_ERROR_GETOSVERSION				16
#define HIS_ERROR_SETFRMSYNC				17
#define HIS_ERROR_SETFRMSYNCMODE			18
#define HIS_ERROR_SETTIMERSYNC				19
#define HIS_ERROR_INVALID_FUNC_CALL			20
#define HIS_ERROR_ABORTCURRFRAME			21
#define HIS_ERROR_GETHWHEADERINFO			22
#define HIS_ERROR_HWHEADER_INV				23
#define HIS_ERROR_SETLINETRIG_MODE			24
#define HIS_ERROR_WRITE_DATA				25
#define HIS_ERROR_READ_DATA					26
#define HIS_ERROR_SETBAUDRATE				27
#define HIS_ERROR_NODESC_AVAILABLE			28
#define HIS_ERROR_BUFFERSPACE_NOT_SUFF		29
#define HIS_ERROR_SETCAMERAMODE				30
#define HIS_ERROR_FRAME_INV					31
#define HIS_ERROR_SLOW_SYSTEM				32
#define HIS_ERROR_GET_NUM_BOARDS			33
#define HIS_ERROR_HW_ALREADY_OPEN_BY_ANOTHER_PROCESS	34
#define HIS_ERROR_CREATE_MEMORYMAPPING				35
#define HIS_ERROR_VXD_REGISTER_DMA_ADDRESS			36
#define HIS_ERROR_VXD_REGISTER_STAT_ADDR			37
#define HIS_ERROR_VXD_UNMASK_IRQ					38
#define HIS_ERROR_LOADDRIVER						39
#define HIS_ERROR_FUNC_NOTIMPL						40
#define HIS_ERROR_MEMORY_MAPPING					41
#define HIS_ERROR_CREATE_MUTEX						42
#define HIS_ERROR_ACQ								43
#define HIS_ERROR_DESC_NOT_LOCAL					44
#define HIS_ERROR_INVALID_PARAM						45
#define HIS_ERROR_ABORT								46
#define HIS_ERROR_WRONGBOARDSELECT					47
#define HIS_ERROR_WRONG_CAMERA_MODE					48	 
#define HIS_ERROR_AVERAGED_LOST						49	 
#define HIS_ERROR_BAD_SORTING_PARAM					50	
#define HIS_ERROR_UNKNOWN_IP_MAC_NAME				51	
#define HIS_ERROR_NO_BOARD_IN_SUBNET				52
#define HIS_ERROR_UNABLE_TO_OPEN_BOARD				53
#define HIS_ERROR_UNABLE_TO_CLOSE_BOARD				54
#define HIS_ERROR_UNABLE_TO_ACCESS_DETECTOR_FLASH	55
#define HIS_ERROR_HEADER_TIMEOUT					56
#define HIS_ERROR_NO_PING_ACK						57
#define HIS_ERROR_NR_OF_BOARDS_CHANGED				58
#define HIS_ERROR_UNDEFINED							100
 
//sort definitions

#define HIS_SORT_NOSORT						0
#define HIS_SORT_QUAD						1
#define HIS_SORT_COLUMN						2
#define HIS_SORT_COLUMNQUAD					3
#define HIS_SORT_QUAD_INVERSE				4
#define HIS_SORT_QUAD_TILE					5
#define HIS_SORT_QUAD_TILE_INVERSE			6
#define HIS_SORT_QUAD_TILE_INVERSE_SCRAMBLE	7
#define HIS_SORT_OCT_TILE_INVERSE			8		//	1640 and 1620
#define HIS_SORT_OCT_TILE_INVERSE_BINDING	9		//	1680
#define HIS_SORT_OCT_TILE_INVERSE_DOUBLE	10		//	1620 reverse
#define HIS_SORT_HEX_TILE_INVERSE			11		//	1621 ADIC
#define HIS_SORT_HEX_CS						12		//	1620/1640 continous scan
#define HIS_SORT_12x1						13		//	12X1 Combo
#define HIS_SORT_14							14		//	


//sequence acquisition options
#define HIS_SEQ_TWO_BUFFERS				0x1
#define HIS_SEQ_ONE_BUFFER				0x2
#define HIS_SEQ_AVERAGE					0x4
#define HIS_SEQ_DEST_ONE_FRAME			0x8
#define HIS_SEQ_COLLATE					0x10
#define HIS_SEQ_CONTINUOUS				0x100		
#define HIS_SEQ_LEAKAGE					0x1000
#define HIS_SEQ_NONLINEAR				0x2000
#define HIS_SEQ_AVERAGESEQ				0x4000	// sequence of averaged frames


#define HIS_SYNCMODE_SOFT_TRIGGER		1
#define HIS_SYNCMODE_INTERNAL_TIMER		2
#define HIS_SYNCMODE_EXTERNAL_TRIGGER	3
#define HIS_SYNCMODE_FREE_RUNNING		4

#define HIS_CAMMODE_SETSYNC		0x8
#define HIS_CAMMODE_TIMEMASK	0x7
#define HIS_CAMMODE_FPGA		0x7F

#define HIS_BOARD_TYPE_NOONE					0x0
#define HIS_BOARD_TYPE_ELTEC					0x1
#define HIS_BOARD_TYPE_DIPIX					0x2
#define HIS_BOARD_TYPE_RS232					0x3
#define HIS_BOARD_TYPE_USB						0x4
//#define HIS_BOARD_TYPE_ELTEC_SCIURIUS			0x8
#define HIS_BOARD_TYPE_ELTEC_XRD_FGX			0x8
#define HIS_BOARD_TYPE_ELTEC_XRD_FGE_Opto		0x10
#define HIS_BOARD_TYPE_ELTEC_GbIF				0x20

#define HIS_MAX_TIMINGS							0x8


/// detector supported options
#define XIS_DETECTOR_SERVICE_MODE_AVAILABLE		0x1 
#define XIS_DETECTOR_TRIGGER_SOURCE_SELECTABLE	0x2
#define XIS_DETECTOR_SUPPORTS_PING				0x3
#define XIS_DETECTOR_SUPPORTED_ROI_MODES		0x4
#define XIS_DETECTOR_SUPPORTED_BINNING_MODES	0x5
#define XIS_DETECTOR_SUPPORTS_GAIN_CHANGE		0x6
#define XIS_DETECTOR_SUPPORTS_MULTIPLE_TRIGGER_MODES	0x7
#define XIS_DETECTOR_SUPPORTS_CONFIGURABLE_TRIGGER_OUT	0x8
#define XIS_DETECTOR_GRPSIZE_ROI_Y				0x9
#define XIS_DETECTOR_LIVEBUFFERSIZE				0xA


//Grps 1&2&3&4, 3&4, 2&3, 1&2 ,4, 3, 2, 1
#define XIS_DETECTOR_PROVIDES_ROI_GRP_1			0x1
#define XIS_DETECTOR_PROVIDES_ROI_GRP_2			0x2
#define XIS_DETECTOR_PROVIDES_ROI_GRP_3			0x4
#define XIS_DETECTOR_PROVIDES_ROI_GRP_4			0x8
#define XIS_DETECTOR_PROVIDES_ROI_GRP_5			0x10 // val 2011-11-14 1642
#define XIS_DETECTOR_PROVIDES_ROI_GRP_6			0x20 // val 2011-11-14 1642
#define XIS_DETECTOR_PROVIDES_ROI_GRP_7			0x40 // val 2011-11-14 1642
#define XIS_DETECTOR_PROVIDES_ROI_GRP_8			0x80 // val 2011-11-14 1642

#define XIS_DETECTOR_PROVIDES_ROI_1_GRP			0x1
#define XIS_DETECTOR_PROVIDES_ROI_2_GRPS		0x2
#define XIS_DETECTOR_PROVIDES_ROI_3_GRPS		0x4
#define XIS_DETECTOR_PROVIDES_ROI_4_GRPS		0x8
#define XIS_DETECTOR_PROVIDES_ROI_ALL_GRPS		0x1000


/*
#define XIS_DETECTOR_PROVIDES_ROI_GRP_1_2		0x10
#define XIS_DETECTOR_PROVIDES_ROI_GRP_2_3		0x20
#define XIS_DETECTOR_PROVIDES_ROI_GRP_3_4		0x40
#define XIS_DETECTOR_PROVIDES_ROI_GRP_1_2_3_4	0x80
*/
// BINNING MODES
#define XIS_DETECTOR_PROVIDES_BINNING_1x1		0x1
#define XIS_DETECTOR_PROVIDES_BINNING_2x2		0x2
#define XIS_DETECTOR_PROVIDES_BINNING_4x4		0x4
#define XIS_DETECTOR_PROVIDES_BINNING_1x2		0x8
#define XIS_DETECTOR_PROVIDES_BINNING_1x4		0x10

#define XIS_DETECTOR_PROVIDES_BINNING_AVG		0x100
#define XIS_DETECTOR_PROVIDES_BINNING_SUM		0x200
// AVG bit 8
// SUM bit 9


#endif	//_ACQUISITION_H
