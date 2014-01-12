#include <windows.h>
#include <stdio.h>
#include <assert.h>
//#include "..\..\COMMON\XISL 4.0\XISL_4_0_WORKSPACE_VS60\Src\Acq.h"
#include "Acq.h"

//-------------------------------------------------------------------------------------------------------------//
// The XISL demonstration application is a simple console application running under Windows OS.
// It demonstrates how to acquire data continuously, how to acquire a sequence, how to create
// correction files and how to acquire corrected images.
//
// Files:
// The demonstration program was build with MS Visual C++ 6.0. The name of the project file is
// 'XISL_Demo.dsp'. If you want to use your own development environment insert XISL.lib in your
// project.
// The header file declaring the required function prototypes is named "Acq.h". The main application
// program source is called "main.c".
//-------------------------------------------------------------------------------------------------------------//


HANDLE hOutput = NULL, hInput = NULL;
DWORD dwCharsWritten=0;
unsigned short *pAcqBuffer=NULL, *pOffsetBuffer=NULL,*pBrightocBuffer=NULL,*pGainSeqBuffer=NULL,*pGainSeqMedBuffer=NULL;
DWORD *pGainBuffer = NULL, *pPixelBuffer = NULL;
HANDLE hevEndAcq=NULL; //signaled at end of acquisition by corresponding callback
char strBuffer[1000]; //buffer for outputs

CHwHeaderInfo Info; 
CHwHeaderInfoEx InfoEx; // new Header 1621
WORD actframe;

#define ACQ_CONT			1
#define ACQ_OFFSET			2
#define ACQ_GAIN			4
#define ACQ_SNAP			8
#define	ACQ_Brightoc		16

//----------------------------------------------------------------------------------------------------------------------//
// The OnEndFrameCallback function is called at the end of each frame's data transfer									//
// In the end of frame callback we can place output, do image processing, etc.											//
//----------------------------------------------------------------------------------------------------------------------//
void CALLBACK OnEndFrameCallback(HACQDESC hAcqDesc)
{
	DWORD dwActFrame, dwSecFrame, dwRow=128, dwCol=128;
	DWORD dwDataType, dwRows, dwColumns, dwFrames, dwSortFlags,
	dwIRQFlags, dwAcqType, dwSystemID, dwAcqData, dwSyncMode, dwHwAccess;

	//----------------------------------------------------------------------------------------------------------------------//
	// The pointer you retrieve by Acquisition_GetAcqData(..) has been set in the main function by Acquisition_SetAcqData.	//
	// Anyway, you are free to define for instance a global variable to indicate the different acquisition modes			//
	// in the callback functions. The above mentioned approach has the advantage of a encapsulation							//
	// similar to that used in object orientated programming.																//
	//----------------------------------------------------------------------------------------------------------------------//

#ifdef __X64
	void *vpAcqData=NULL;
	Acquisition_GetAcqData(hAcqDesc, &vpAcqData);
	dwAcqData = *((DWORD*)vpAcqData);
#else
	Acquisition_GetAcqData(hAcqDesc, &dwAcqData);
#endif

	Acquisition_GetConfiguration(hAcqDesc, &dwFrames, &dwRows,
		&dwColumns, &dwDataType, &dwSortFlags, &dwIRQFlags, &dwAcqType, 
		&dwSystemID, &dwSyncMode, &dwHwAccess);
	Acquisition_GetActFrame(hAcqDesc, &dwActFrame, &dwSecFrame);	

	// 1621 function demo
	Acquisition_GetLatestFrameHeader(hAcqDesc,&Info,&InfoEx);
	//if (Info.dwHeaderID==14 && InfoEx.wCameratype==1) //1621 ?)
	if (Info.dwHeaderID==14)
	{
		sprintf(strBuffer, "framecount: %d frametime %d,%d millisec\t",InfoEx.wFrameCnt,InfoEx.wRealInttime_milliSec,InfoEx.wRealInttime_microSec);
		WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
	}
	// 1621 function demo end

	// Depending from the passed status flag it is decided how to read out the acquisition buffer:
	if (dwAcqData & ACQ_SNAP)
	{
		sprintf(strBuffer, "acq buffer frame: %d, dest frame %d, row: %d, col: %d, value: %d\n", 
			dwActFrame, dwSecFrame,
			dwRow, dwCol, pAcqBuffer[dwColumns*dwRow+dwCol]);
			//dwRow, dwCol, pAcqBuffer[((dwSecFrame-1)*(dwColumns*dwRows)) + dwColumns*dwRow+dwCol]);
	}

	else if (dwAcqData & ACQ_CONT) 
	{
		sprintf(strBuffer, "acq buffer frame: %d, dest frame %d, row: %d, col: %d, value: %d\n", 
			dwActFrame, dwSecFrame,
			dwRow, dwCol, pAcqBuffer[/*((dwSecFrame-1)*(dwColumns*dwRows)) + */dwColumns*dwRow+dwCol]);


	} else if (dwAcqData & ACQ_OFFSET)
	{
		sprintf(strBuffer, "offset buffer frame: %d, dest frame %d, row: %d, col: %d, value: %d\n", 
			dwActFrame, dwSecFrame,
			dwRow, dwCol, pOffsetBuffer[dwColumns*dwRow+dwCol]);

	}  else if (dwAcqData & ACQ_GAIN)
	{
		sprintf(strBuffer, "gain buffer frame: %d, dest frame %d, row: %d, col: %d, value: %d\n", 
			dwActFrame, dwSecFrame,
			dwRow, dwCol, pGainBuffer[dwColumns*dwRow+dwCol]);

	} else	if (dwAcqData & ACQ_Brightoc)
	{
		sprintf(strBuffer, "gain buffer frame: %d, dest frame %d, row: %d, col: %d, value: %d\n", 
			dwActFrame, dwSecFrame,
			dwRow, dwCol, pBrightocBuffer[dwColumns*dwRow+dwCol]);

	}
	else
	{
		//printf("endframe \n");
		sprintf(strBuffer, "endframe image size %d\n\n",dwRows);
	}
	WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
}

//callback function that is called by XISL at end of acquisition
void CALLBACK OnEndAcqCallback(HACQDESC hAcqDesc)
{

	DWORD dwActFrame, dwSecFrame, dwRow=128, dwCol=128;
	DWORD dwDataType, dwRows, dwColumns, dwFrames, dwSortFlags,
	dwIRQFlags, dwAcqType, dwSystemID, dwAcqData, dwSyncMode, dwHwAccess;
	
#ifdef __X64
	void *vpAcqData=NULL;
	Acquisition_GetAcqData(hAcqDesc, &vpAcqData);
	dwAcqData = *((DWORD*)vpAcqData);
#else
	Acquisition_GetAcqData(hAcqDesc, &dwAcqData);
#endif


	Acquisition_GetConfiguration(hAcqDesc, &dwFrames, &dwRows,
		&dwColumns, &dwDataType, &dwSortFlags, &dwIRQFlags, &dwAcqType, 
		&dwSystemID, &dwSyncMode, &dwHwAccess);
	Acquisition_GetActFrame(hAcqDesc, &dwActFrame, &dwSecFrame);

	
	printf("dwActTyp %d\n",dwAcqType);

 	printf("End of Acquisition\n");
	SetEvent(hevEndAcq);
}

long DetectorInit(HACQDESC* phAcqDesc, long bGigETest, int IBIN1, int iGain)
{
	int iRet;							// Return value
	int iCnt;							// 
	unsigned int uiNumSensors;			// Number of sensors
	HACQDESC hAcqDesc=NULL;
	unsigned short usTiming=0;
	unsigned short usNetworkLoadPercent=80;

	int iSelected;						// Index of selected GigE detector
	long ulNumSensors = 0;				// nr of GigE detector in network

	GBIF_DEVICE_PARAM Params;
	GBIF_Detector_Properties Properties;

	BOOL bSelfInit = TRUE;
	long lOpenMode = HIS_GbIF_IP;
	long lPacketDelay = 256;

	char* pTest = NULL;
	unsigned int dwRows=0, dwColumns=0;

	// First we tell the system to enumerate all available sensors
	// * to initialize them in polling mode, set bEnableIRQ = FALSE;
	// * otherwise, to enable interrupt support, set bEnableIRQ = TRUE;
	BOOL bEnableIRQ = TRUE;
	
	if (bGigETest)
	{	
		uiNumSensors = 0; 
	
		// find GbIF Detectors in Subnet	
		iRet = Acquisition_GbIF_GetDeviceCnt(&ulNumSensors);
		if (iRet != HIS_ALL_OK)
		{
			sprintf(strBuffer,"%s fail! Error Code %d\t\t\t\t\n","Acquisition_GbIF_GetDetectorCnt",iRet);
			WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
			return iRet;
		}
		else
		{
			sprintf(strBuffer,"%s\t\t\t\tPASS!\n","Acquisition_GbIF_GetDetectorCnt");
			WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
		}
				
		if(ulNumSensors>0)
		{
			// get device params of GbIF Detectors in Subnet
			GBIF_DEVICE_PARAM* pGbIF_DEVICE_PARAM = (GBIF_DEVICE_PARAM*)malloc( sizeof(GBIF_DEVICE_PARAM)*(ulNumSensors));
			
			iRet = Acquisition_GbIF_GetDeviceList(pGbIF_DEVICE_PARAM,ulNumSensors);
			if (iRet != HIS_ALL_OK)
			{
				sprintf(strBuffer,"%s fail! Error Code %d\t\t\t\t\n","Acquisition_GbIF_GetDeviceList",iRet);
				WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
				return iRet;
			}
			else
			{
				sprintf(strBuffer,"%s\t\t\t\tPASS!\n","Acquisition_GbIF_GetDeviceList");
			}
			
			WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);

			sprintf(strBuffer,"Select Sensor Nr:\n");
			WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
			
			for (iCnt = 0 ; iCnt < ulNumSensors; iCnt++)
			{
				sprintf(strBuffer,"%d - %s\n",iCnt,(pGbIF_DEVICE_PARAM[iCnt]).cDeviceName);
				WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
			}
			
			scanf("%d",&iSelected);
			sprintf(strBuffer,"%d - selected\n",iSelected);
			WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);

			if (iSelected>-1 && iSelected<ulNumSensors)
			{
				//try to init detector
				uiNumSensors = 0;
			
				// convert ip to string
				pTest = (char*)malloc(256*sizeof(char));

				iRet = Acquisition_GbIF_Init(
					&hAcqDesc,
					//iSelected,							// Index to access individual detector
					0,										// here set to zero for a single detector device
					bEnableIRQ, 
					dwRows, dwColumns,						// Image dimensions
					bSelfInit,								// retrieve settings (rows,cols.. from detector
					FALSE,									// If communication port is already reserved by another process, do not open
					lOpenMode,								// here: HIS_GbIF_IP, i.e. open by IP address 
					pGbIF_DEVICE_PARAM[iSelected].ucIP		// IP address of the connection to open
					);

				if (iRet != HIS_ALL_OK)
				{
					sprintf(strBuffer,"%s fail! Error Code %d\t\t\t\t\n","Acquisition_GbIF_Init",iRet);
					WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
					return iRet;
				}
				else
				{
					sprintf(strBuffer,"%s\t\t\t\tPASS!\n","Acquisition_GbIF_Init");

					// Calibrate connection
					if (Acquisition_GbIF_CheckNetworkSpeed(	hAcqDesc, &usTiming, &lPacketDelay, usNetworkLoadPercent)==HIS_ALL_OK)
					{
						sprintf(strBuffer,"%s result: suggested timing: %d packetdelay %d @%d networkload\t\t\t\n"
							,"Acquisition_GbIF_CheckNetworkSpeed",usTiming,lPacketDelay,usNetworkLoadPercent);
						WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);

					}

					iRet = Acquisition_GbIF_SetPacketDelay(hAcqDesc,lPacketDelay);
					if (iRet != HIS_ALL_OK)
					{
						sprintf(strBuffer,"%s fail! Error Code %d\t\t\t\n","Acquisition_GbIF_SetPacketDelay",iRet);
						WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
						return iRet;
					}
					else
					{
						sprintf(strBuffer,"%s %d\t\t\t\tPASS!\n","Acquisition_GbIF_SetPacketDelay",lPacketDelay);
					}
					WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);

					// Get Detector Params of already opened GigE Detector
					iRet = Acquisition_GbIF_GetDeviceParams(hAcqDesc,&Params);
					if (iRet != HIS_ALL_OK)
					{
						sprintf(strBuffer,"%s fail! Error Code %d\t\t\t\n","Acquisition_GBIF_GetDeviceParams",iRet);
						WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
						return iRet;
					}
					else
					{
						sprintf(strBuffer,"%s \t\t\t\tPASS!\n","Acquisition_GBIF_GetDeviceParams");
						sprintf(strBuffer, "Device Name: \t\t%s\nMAC: \t\t\t%s\nIP: \t\t\t%s\nSubnetMask: \t\t%s\nGateway: \t\t%s\nAdapterIP: \t\t%s\nAdapterSubnetMask: \t%s\nBootOptions Flag: \t%d\nGBIF Firmware: \t\t%s\n",
							Params.cDeviceName,
							Params.ucMacAddress,
							Params.ucIP, 
							Params.ucSubnetMask, 
							Params.ucGateway,
							Params.ucAdapterIP,
							Params.ucAdapterMask, 
							Params.dwIPCurrentBootOptions,
							Params.cGBIFFirmwareVersion
						);
						WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
					}

					// Read production data
					iRet = Acquisition_GbIF_GetDetectorProperties(hAcqDesc,&Properties);
					if (iRet != HIS_ALL_OK)
					{
						sprintf(strBuffer,"%s fail! Error Code %d\t\t\t\n","Acquisition_GbIF_GetDetectorProperties",iRet);
						WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
						return -1;
					}
					else
					{
						sprintf(strBuffer,"%s \t\t\t\tPASS!\n","Acquisition_GbIF_GetDetectorProperties");
						sprintf(strBuffer, "Detector Type: \t\t%s\nManufDate: \t\t%s\nManufPlace: \t\t%s\n", Properties.cDetectorType,
							Properties.cManufacturingDate, Properties.cPlaceOfManufacture);
						WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
					}
					//

					*phAcqDesc = hAcqDesc;
				}
			}
		}
		else
			return HIS_ERROR_NO_BOARD_IN_SUBNET;
	}
	else
	{
		//------------------------------------------------------------------------------------------------------------------//
		// Automatic Initialization:																						//
		// The XISL is able to recognize all sensors connected to the system automatically by its internal PROM settings.	//
		// In case of GigE connected detectors only point-to-point connected are opened with Enum-Sensors					//
		// The following code fragment shows the corresponding function call:												//
		//------------------------------------------------------------------------------------------------------------------//
		
		if ((iRet = Acquisition_EnumSensors(&uiNumSensors,	// Number of sensors
											1,				// Enable Interrupts
											FALSE			// If communication port is already reserved by another process, do not open. If this parameter is TRUE the XISL is capturing all communication port regardless if this port is already opened by other processes running on the system, which is only recommended for debug versions.
					 )
										   )!=HIS_ALL_OK)
		{
			sprintf(strBuffer,"%s fail! Error Code %d\t\t\t\t\n","Acquisition_EnumSensors",iRet);
			WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
			return iRet;
		}
		else
		{
			sprintf(strBuffer,"%s\t\t\t\tPASS!\n","Acquisition_EnumSensors");
		}
		WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
	}

	return HIS_ALL_OK;
}

int main()
{
	DWORD time;
	int nRet = HIS_ALL_OK;
	int bEnableIRQ;
	DWORD *pPixelPtr = NULL;
	DWORD dwDataType, dwRows, dwColumns, dwFrames, dwSortFlags,
		dwAcqType, dwSystemID, dwSyncMode, dwHwAccess;
	HACQDESC hAcqDesc=NULL;
	INPUT_RECORD ir;
	DWORD dwRead;
	char szFileName[300];
	FILE *pFile = NULL;
	ACQDESCPOS Pos = 0;
	int nChannelNr;
	UINT nChannelType;
	DWORD dwDemoParam;

	int iSelected=0;

	// variables for bad pixel correction
	unsigned short* pwPixelMapData =NULL;
	int* pCorrList = NULL;
	int iListSize=0;

	//get an output handle to console
	hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOutput == INVALID_HANDLE_VALUE)
	{
		//error handling
		return 0;
	}

	hInput = GetStdHandle(STD_INPUT_HANDLE);
	if (hInput == INVALID_HANDLE_VALUE)
	{
		//error handling
		return 0;
	}
	//--------------------------------------------------------------------------------------------------//	
	// First we tell the system to enumerate all available sensors										//
	// * to initialize them in polling mode, set bEnableIRQ = FALSE;									//
	// * otherwise, to enable interrupt support, set bEnableIRQ = TRUE;									//
	//--------------------------------------------------------------------------------------------------//
	
	
	// Initialization Mode:
	sprintf(strBuffer,"Please choose init mode:\n0 - Automatic (Framegrabbers / point-to-point connected GigE Detectors)\n1 - Init of GigE Detectors: network and point-to-point connected\n");
	WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
	scanf("%d",&iSelected);
	sprintf(strBuffer,"%d - selected\n",iSelected);
	WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);

	// This function is implemented above to give examples of the different ways to initialize your detector(s)
	nRet = DetectorInit(&hAcqDesc, iSelected, 0, 1);

	if (nRet!=HIS_ALL_OK)
	{
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}

	// now we iterate through all this sensors, set further parameters (sensor size,
	// sorting scheme, system id, interrupt settings and so on) and extract information from every sensor.
	// To start the loop Pos must be initialized by zero.
	do
	{
		if ((nRet = Acquisition_GetNextSensor(&Pos, &hAcqDesc))!=HIS_ALL_OK)
		{
			char szMsg[300];
			sprintf(szMsg, "Error nr.: %d", nRet);
			MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
			goto Exit;
		}
	
		//ask for communication device type and its number
		if ((nRet=Acquisition_GetCommChannel(hAcqDesc, &nChannelType, &nChannelNr))!=HIS_ALL_OK)
		{
			//error handling
			char szMsg[300];
			sprintf(szMsg, "Error nr.: %d", nRet);
			MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
			return 0;
		}

		sprintf(strBuffer, "channel type: %d, ChannelNr: %d\n", nChannelType, nChannelNr);
		WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);

		switch (nChannelType)
		{
		case HIS_BOARD_TYPE_ELTEC_XRD_FGE_Opto:
			sprintf(strBuffer,"%s%d\n","HIS_BOARD_TYPE_ELTEC_XRD_FGE_Opto:",nChannelType);
			break;
		case HIS_BOARD_TYPE_ELTEC_XRD_FGX:
			sprintf(strBuffer,"%s%d\n","HIS_BOARD_TYPE_ELTEC_XRD_FGX:",nChannelType);
			break;
		case HIS_BOARD_TYPE_ELTEC:
			sprintf(strBuffer,"%s%d\n","HIS_BOARD_TYPE_ELTEC:",nChannelType);
			break;
		case HIS_BOARD_TYPE_ELTEC_GbIF:
			sprintf(strBuffer,"%s%d\n","HIS_BOARD_TYPE_ELTEC_GbIF:",nChannelType);
			break;
		default:
			sprintf(strBuffer,"%s%d\n","Unknown ChanelType:",nChannelType);
			break;
		}
		WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);

		//ask for data organization of all sensors
		if ((nRet=Acquisition_GetConfiguration(hAcqDesc, &dwFrames, &dwRows,
			&dwColumns, &dwDataType, &dwSortFlags, &bEnableIRQ
			
			, &dwAcqType, 
			&dwSystemID, &dwSyncMode, &dwHwAccess))!=HIS_ALL_OK)
		{
			//error handling
			char szMsg[300];
			sprintf(szMsg, "Error nr.: %d", nRet);
			MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
			return 0;
		}
	
		sprintf(strBuffer, "frames: %d\n", dwFrames);
		WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
		sprintf(strBuffer, "rows: %d\ncolumns: %d\n", dwRows, dwColumns);
		WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);

		// now set callbacks and messages for every sensor
		if ((nRet=Acquisition_SetCallbacksAndMessages(hAcqDesc, NULL, 0,
			0, OnEndFrameCallback, OnEndAcqCallback))!=HIS_ALL_OK)
		{
			//error handling
			char szMsg[300];
			sprintf(szMsg, "Error nr.: %d", nRet);
			MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
			goto Exit;
		}

	}
	while (Pos!=0);

	//--------------------------------------------------------------------------------------------------//
	// For further description see Acquisition_GetNextSensor, Acquisition_GetCommChannel,				//
	// Acquisition_SetCallbacksAndMessages and Acquisition_GetConfiguration.							//
	// For the further steps we select the last recognized sensor to demonstrate the remaining tasks.	//
	//--------------------------------------------------------------------------------------------------//

	// We continue with one detector to show the other XISL features
	// short demo how to use the features of the 1621 if one is connected
		
	//ask for communication device type and its number
	if ((nRet=Acquisition_GetCommChannel(hAcqDesc, &nChannelType, &nChannelNr))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		return 0;
	}				
	
	// check if optical framgrabber is used
	if (nChannelType==HIS_BOARD_TYPE_ELTEC_XRD_FGX
		||
		nChannelType==HIS_BOARD_TYPE_ELTEC_XRD_FGE_Opto) 
	{
		//create and receive FrameHeader
		CHwHeaderInfo Info; 
		CHwHeaderInfoEx InfoEx; // new Header 1621
		if ((nRet = Acquisition_GetHwHeaderInfoEx(hAcqDesc, &Info, &InfoEx))==HIS_ALL_OK)
		{
			// header could be retrieved
			
			//check if 1621 connected
			if (Info.dwHeaderID==14 && InfoEx.wCameratype>=1)
			{
				//--------------------------------------------------------------------------------------------------//
				// Acquiring Offset Images																			//
				// The sensor needs an offset correction to work properly. For this purpose the XISL provides a		//
				// special function Acquisition_Acquire_OffsetImage.												//
				// First we have to allocate a buffer for the offset data. The image size has to fit				//
				// the current image dimensions as the image data might be binned									//
				//--------------------------------------------------------------------------------------------------//
				unsigned short *pOffsetBufferBinning1=NULL,*pOffsetBufferBinning2=NULL;

				WORD wBinning=1;
				int timings = 8;
				//create lists to receive timings for different binning modes
				double* m_pTimingsListBinning1;
				double* m_pTimingsListBinning2;
				m_pTimingsListBinning1 = (double*) malloc(timings*sizeof(double));
				m_pTimingsListBinning2 = (double*) malloc(timings*sizeof(double));

				//  set detector timing and gain
				if ((nRet = Acquisition_SetCameraMode(hAcqDesc, 0))!=HIS_ALL_OK) goto exit;
				if ((nRet = Acquisition_SetCameraGain(hAcqDesc, 1))!=HIS_ALL_OK) goto exit;
				// set detector to default binning mode
				if ((nRet = Acquisition_SetCameraBinningMode(hAcqDesc,wBinning))!=HIS_ALL_OK) goto exit;
				// get int times for selected binning mode
				if ((nRet = Acquisition_GetIntTimes(hAcqDesc, m_pTimingsListBinning1, &timings))!=HIS_ALL_OK) goto exit;
				
				//ask for detector data organization to 
				if ((nRet = Acquisition_GetConfiguration(hAcqDesc, &dwFrames, &dwRows,
					&dwColumns, &dwDataType, &dwSortFlags, &bEnableIRQ, &dwAcqType, 
					&dwSystemID, &dwSyncMode, &dwHwAccess))!=HIS_ALL_OK) goto exit;
				
				// get offsetfile
				// allocate memory for offset data for binning1
				pOffsetBufferBinning1 = malloc(dwRows*dwColumns*sizeof(short));


				// Pointers can be defined to be available in the EndFrameCallback function.
				// Let's define
				dwDemoParam = 0;
				// and pass its address to the XISL
#ifdef __X64	
				if ((nRet=Acquisition_SetAcqData(hAcqDesc, &dwDemoParam))!=HIS_ALL_OK) goto exit;
#else
				if ((nRet=Acquisition_SetAcqData(hAcqDesc, dwDemoParam))!=HIS_ALL_OK) goto exit;
#endif				
				printf("\nAcquire offset data now!\n"); //valtest

				hevEndAcq = CreateEvent(NULL, FALSE, FALSE, NULL);
				
				// acquire 13 dark frames and average them. The XISL automatically averages those frame to an offset image.
				if ((nRet = Acquisition_Acquire_OffsetImage(hAcqDesc, pOffsetBufferBinning1, dwRows, dwColumns, 13))!=HIS_ALL_OK) goto exit;
					
				//wait for end of acquisition event
				if (WaitForSingleObject(hevEndAcq, INFINITE)!=WAIT_OBJECT_0) goto exit;
				
				// get gainimage is similar....

				// now change to other binning mode
				// set detector to default binning mode
				wBinning=2;
				// set detector to 2x2 binned mode
				if ((nRet =Acquisition_SetCameraBinningMode(hAcqDesc,wBinning))!=HIS_ALL_OK) goto exit;
				// get int times for selected binning mode
				if ((nRet =Acquisition_GetIntTimes(hAcqDesc, m_pTimingsListBinning2, &timings))!=HIS_ALL_OK) goto exit;
								
				//ask for changed detector data organization
				if ((nRet =Acquisition_GetConfiguration(hAcqDesc, &dwFrames, &dwRows,
					&dwColumns, &dwDataType, &dwSortFlags, &bEnableIRQ, &dwAcqType, 
					&dwSystemID, &dwSyncMode, &dwHwAccess))!=HIS_ALL_OK) goto exit;
				
				// get offsetfile
				// allocate memory for offset data for binning2 which has different numbers for rows and columns than binning 1. 
				// We retrieve the image dimensions for binning 2 by the above call of Acquisition_GetConfiguration
				pOffsetBufferBinning2 = malloc(dwRows*dwColumns*sizeof(short));

// Pointers can be defined to be available in the EndFrameCallback function.
// Let's define
				dwDemoParam = 0;
#ifdef __X64	
				if ((nRet=Acquisition_SetAcqData(hAcqDesc, &dwDemoParam))!=HIS_ALL_OK) goto exit;
#else
				if ((nRet=Acquisition_SetAcqData(hAcqDesc, dwDemoParam))!=HIS_ALL_OK) goto exit;
#endif	
				
				printf("\nAcquire offset data now!\n"); 

				hevEndAcq = CreateEvent(NULL, FALSE, FALSE, NULL);
				
				// acquire 13 dark frames for binning2 and average them
				if ((nRet=Acquisition_Acquire_OffsetImage(hAcqDesc, pOffsetBufferBinning2, dwRows, dwColumns, 13))!=HIS_ALL_OK) goto exit;
				
				//wait for end of acquisition event
				if (WaitForSingleObject(hevEndAcq, INFINITE)!=WAIT_OBJECT_0) goto exit;
				// get gainimage is similar....


				// now back to default binning
				wBinning=1;
				if ((nRet=Acquisition_SetCameraBinningMode(hAcqDesc,wBinning))!=HIS_ALL_OK) goto exit;
			
				// get offsetcorr averaged image binning1
				//ask for detector data organization
				if ((nRet=Acquisition_GetConfiguration(hAcqDesc, &dwFrames, &dwRows,
					&dwColumns, &dwDataType, &dwSortFlags, &bEnableIRQ, &dwAcqType, 
					&dwSystemID, &dwSyncMode, &dwHwAccess))!=HIS_ALL_OK) goto exit;
				
				//	For acquisition we have to allocate an acquisition buffer with the image dimensions we received from Acquisition_GetConfiguration:
				pAcqBuffer = malloc(1*dwRows*dwColumns*sizeof(short));
				if (!pAcqBuffer) goto Exit;

				dwDemoParam = ACQ_SNAP;
#ifdef __X64	
				if ((nRet=Acquisition_SetAcqData(hAcqDesc, &dwDemoParam))!=HIS_ALL_OK) goto exit;
#else
				if ((nRet=Acquisition_SetAcqData(hAcqDesc, dwDemoParam))!=HIS_ALL_OK) goto exit;
#endif

				// After that we have to pass the address of our buffer to the XISL as well as the numbers of rows and columns:
				if ((nRet=Acquisition_DefineDestBuffers(hAcqDesc, pAcqBuffer,
					1, dwRows, dwColumns))!=HIS_ALL_OK) goto exit;

				if ((Acquisition_Acquire_Image(hAcqDesc,15,0, 
					HIS_SEQ_AVERAGE, pOffsetBufferBinning1, NULL, NULL))!=HIS_ALL_OK) goto exit;

				// wait for end of acquisition event
				// Note: In a windowed application you should post a message to you acquisition windows in your end of acquisition callback function.
				if ( WaitForSingleObject(hevEndAcq, INFINITE)!=WAIT_OBJECT_0) goto exit;
				free(pAcqBuffer);
				
				// get offsetcorr averaged image binning2
				// now back to binning 2
				wBinning=2;
				if ((nRet=Acquisition_SetCameraBinningMode(hAcqDesc,wBinning))!=HIS_ALL_OK) goto exit;
			
				// get offsetcorr averaged image binning2
				//ask for detector data organization
				if ((nRet=Acquisition_GetConfiguration(hAcqDesc, &dwFrames, &dwRows,
					&dwColumns, &dwDataType, &dwSortFlags, &bEnableIRQ, &dwAcqType, 
					&dwSystemID, &dwSyncMode, &dwHwAccess))!=HIS_ALL_OK) goto exit;
				
				//allocate acquisition buffer
				pAcqBuffer = malloc(1*dwRows*dwColumns*sizeof(short));
				if (!pAcqBuffer) goto Exit;

				dwDemoParam = ACQ_SNAP;
#ifdef __X64
				if ((nRet=Acquisition_SetAcqData(hAcqDesc, (void*)&dwDemoParam))!=HIS_ALL_OK) goto exit;
#else
				if ((nRet=Acquisition_SetAcqData(hAcqDesc, dwDemoParam))!=HIS_ALL_OK) goto exit;
#endif
	
				if ((nRet=Acquisition_DefineDestBuffers(hAcqDesc, pAcqBuffer,
					1, dwRows, dwColumns))!=HIS_ALL_OK) goto exit;

				// Reset Framecounter to read out and display the framecnt in the endframecallback
				if ((nRet=Acquisition_ResetFrameCnt(hAcqDesc))!=HIS_ALL_OK) goto exit;
				
				if ((Acquisition_Acquire_Image(hAcqDesc,15,0, 
					HIS_SEQ_AVERAGE, pOffsetBufferBinning2, NULL, NULL))!=HIS_ALL_OK) goto exit;

				//wait for end of acquisition event
				if ( WaitForSingleObject(hevEndAcq, INFINITE)!=WAIT_OBJECT_0) goto exit;
				free(pAcqBuffer);
				free(pOffsetBufferBinning1);
				free(pOffsetBufferBinning2);

				// now back to default binning
				wBinning=1;
				if ((nRet=Acquisition_SetCameraBinningMode(hAcqDesc,wBinning))!=HIS_ALL_OK) goto exit;
				//ask for detector data organization
				if ((nRet=Acquisition_GetConfiguration(hAcqDesc, &dwFrames, &dwRows,
					&dwColumns, &dwDataType, &dwSortFlags, &bEnableIRQ, &dwAcqType, 
					&dwSystemID, &dwSyncMode, &dwHwAccess))!=HIS_ALL_OK) goto exit;
			}
		}
	}
	// -> board type is HIS_BOARD_TYPE_ELTEC or HIS_BOARD_TYPE_ELTEC_GbIF

	//acquire 10 frames
	dwFrames = 10;	
	
	//allocate acquisition buffer
	pAcqBuffer = malloc(dwFrames*dwRows*dwColumns*sizeof(short));
	if (!pAcqBuffer)
	{
		//error handling
		goto Exit;
	}

	//--------------------------------------------------------------------------------------------------//
	// We create a scheduler event here that is used to block the main thread from execution after		//
	// starting the acquisition. In a normal windowed application you should skip this step and post a	//
	// message to your acquisition window instead of setting a scheduler event in the end of acquisition//
	// callback function.																				//
	//--------------------------------------------------------------------------------------------------//
	// create end of acquisition event
	hevEndAcq = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!hevEndAcq)
	{
		//error handling
		goto Exit;
	}


	// Timing 0 Internal Timer 500msec
	Acquisition_SetCameraMode(hAcqDesc,0);

//	change of the syncmode from to internal trigger mode
	Acquisition_SetFrameSyncMode(hAcqDesc,HIS_SYNCMODE_INTERNAL_TIMER);

//	setting of the integration time in µs (integration time = read out time plus delay time)
	time=1000*500;
	Acquisition_SetTimerSync(hAcqDesc,&time);


	// Pointers can be defined to be available in the EndFrameCallback function.
	// Let's define
	dwDemoParam = ACQ_SNAP;
	// and pass its address to the XISL
#ifdef __X64
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, (void*)&dwDemoParam))!=HIS_ALL_OK) goto exit;
#else
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, dwDemoParam))!=HIS_ALL_OK) goto exit;
#endif

	if ((nRet=Acquisition_DefineDestBuffers(hAcqDesc, pAcqBuffer,
		dwFrames, dwRows, dwColumns))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}

	// now we acquire dwFrames (in this example dwFrames is 10) images
	if((Acquisition_Acquire_Image(hAcqDesc,dwFrames,0, 
	//		HIS_SEQ_DEST_ONE_FRAME, NULL, NULL, NULL))!=HIS_ALL_OK)
	//		HIS_SEQ_ONE_BUFFER, NULL, NULL, NULL))!=HIS_ALL_OK)
	//		HIS_SEQ_TWO_BUFFERS, NULL, NULL, NULL))!=HIS_ALL_OK)
//			HIS_SEQ_CONTINUOUS, NULL, NULL, NULL))!=HIS_ALL_OK)
	//		HIS_SEQ_AVERAGE, NULL, NULL, NULL))!=HIS_ALL_OK)
	//		HIS_SEQ_COLLATE, NULL, NULL, NULL))!=HIS_ALL_OK)
			HIS_SEQ_ONE_BUFFER, NULL, NULL, NULL))!=HIS_ALL_OK)
		{
			//error handling
			char szMsg[300];
			sprintf(szMsg, "Error nr.: %d", nRet);
			MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
			goto Exit;
		}
		
		sprintf(strBuffer, "I'm waiting\n");
		WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);

	//--------------------------------------------------------------------------------------------------//
	// Now we have to prevent the application from calling other XISL functions during data acquisition //
	// except the Acquisisition_Abort, Acquisition_DoOffsetCorrection, Acquisition_DoGainCorrection,	//
	// Acquisition_DoOffsetGainCorrection and Acquisition_DoPixelCorrection, functions. In this			//
	// console application I block the main thread from execution until the end of acquisition event is	//
	// signaled by the end of acquisition callback. In a windowed application you cannot use this		//
	// approach because the message handling of your program would block either. You are responsible	//
	// by yourself to prevent the above mentioned additional XISL functions from execution (for instance//
	// gray corresponding menu items).																	//
	//--------------------------------------------------------------------------------------------------//

	// wait for end of acquisition event
	nRet = WaitForSingleObject(hevEndAcq, INFINITE);
		

	free(pAcqBuffer);
	switch(nRet)
	{
	case WAIT_FAILED:
		MessageBox(NULL, "wait failed", "debug message", MB_OK | MB_ICONSTOP);
		goto Exit;
	case WAIT_OBJECT_0:
		break;
	case WAIT_ABANDONED:
		MessageBox(NULL, "wait abandoned by mutex", "debug message", MB_OK | MB_ICONSTOP);
		break;
	case WAIT_TIMEOUT:
		MessageBox(NULL, "wait time out occured", "debug message", MB_OK | MB_ICONSTOP);
		break; 
	}


	//--------------------------------------------------------------------------------------------------//
	// Continuous Acquisition																			//
	//--------------------------------------------------------------------------------------------------//
	// Free Running w/ Timing 1
	Acquisition_SetCameraMode(hAcqDesc,1);
	Acquisition_SetFrameSyncMode(hAcqDesc,HIS_SYNCMODE_FREE_RUNNING);
	
	// At first we allocate memory for one frame.
	dwFrames=1;
	pAcqBuffer = malloc(dwFrames*dwRows*dwColumns*sizeof(short));
	if (!pAcqBuffer)
	{
		//error handling
		goto Exit;
	}

	// Route acquisition buffer to XISL
	if ((nRet=Acquisition_DefineDestBuffers(hAcqDesc, pAcqBuffer,
 		dwFrames, dwRows, dwColumns))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}

	// create end of acquisition event
	hevEndAcq = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!hevEndAcq)
	{
		//error handling
		goto Exit;
	}

	
	// Pointers can be defined to be available in the EndFrameCallback function.
	// Let's define:
	dwDemoParam = ACQ_SNAP;
	// and pass its address to the XISL
#ifdef __X64
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, (void*)&dwDemoParam))!=HIS_ALL_OK) goto exit;
#else
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, dwDemoParam))!=HIS_ALL_OK) goto exit;
#endif

	// Acquire 10 Frames Averaged 
	if ((nRet=Acquisition_Acquire_Image(hAcqDesc,10,0,
		HIS_SEQ_AVERAGE, NULL, NULL, NULL))!=HIS_ALL_OK)
		{
			//error handling
			char szMsg[300];
			sprintf(szMsg, "Error nr.: %d", nRet);
			MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
			goto Exit;
		}
		
		sprintf(strBuffer, "I'm waiting\n");
		WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);

		//wait for end of acquisition event
		nRet = WaitForSingleObject(hevEndAcq, INFINITE);

	free(pAcqBuffer);
	switch(nRet)
	{
	case WAIT_FAILED:
		MessageBox(NULL, "wait failed", "debug message", MB_OK | MB_ICONSTOP);
		goto Exit;
	case WAIT_OBJECT_0:
		break;
	case WAIT_ABANDONED:
		MessageBox(NULL, "wait abandoned by mutex", "debug message", MB_OK | MB_ICONSTOP);
		break;
	case WAIT_TIMEOUT:
		MessageBox(NULL, "wait time out occured", "debug message", MB_OK | MB_ICONSTOP);
		break; 
	}


	hevEndAcq = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!hevEndAcq)
	{
		//error handling
		goto Exit;
	}

	dwFrames=1;
	sprintf(strBuffer,"\nPress a key to cancel acquisition!\n");
	WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
	pAcqBuffer = malloc(dwFrames*dwRows*dwColumns*sizeof(short));
	if (!pAcqBuffer)
	{
		//error handling
		goto Exit;
	}

	//route acquisition buffer to XISL
	if ((nRet=Acquisition_DefineDestBuffers(hAcqDesc, pAcqBuffer,
		dwFrames, dwRows, dwColumns))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}
	
	//set acquisition data to use it in callback

	dwDemoParam = ACQ_CONT;
#ifdef __X64
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, (void*)&dwDemoParam))!=HIS_ALL_OK) goto exit;
#else
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, dwDemoParam))!=HIS_ALL_OK) goto exit;
#endif

	//continuous acquisition
	if ((nRet=Acquisition_Acquire_Image(hAcqDesc, dwFrames, 0, HIS_SEQ_CONTINUOUS, NULL, NULL, NULL))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}
	
	FlushConsoleInputBuffer(hInput);

	do
	{
		WaitForSingleObject(hInput, INFINITE);
		ReadConsoleInput(hInput, &ir, 1, &dwRead);
	} while(!(ir.EventType==KEY_EVENT && ir.Event.KeyEvent.bKeyDown));


	Acquisition_Abort(hAcqDesc);
	nRet = WaitForSingleObject(hevEndAcq, INFINITE);

	switch(nRet)
	{
	case WAIT_FAILED:
		MessageBox(NULL, "wait failed", "debug message", MB_OK | MB_ICONSTOP);
		goto Exit;
	case WAIT_OBJECT_0:
		break;
	case WAIT_ABANDONED:
		MessageBox(NULL, "wait abandoned by mutex", "debug message", MB_OK | MB_ICONSTOP);
		break;
	case WAIT_TIMEOUT:
		MessageBox(NULL, "wait time out occured", "debug message", MB_OK | MB_ICONSTOP);
		break;
	}


	dwFrames=1;

	hevEndAcq = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!hevEndAcq)
	{
		//error handling
		goto Exit;
	}

	if (pAcqBuffer)
		free (pAcqBuffer);
	pAcqBuffer = malloc(dwFrames*dwRows*dwColumns*sizeof(short));
	//route acquisition buffer to XISL
	if ((nRet=Acquisition_DefineDestBuffers(hAcqDesc, pAcqBuffer,
		dwFrames, dwRows, dwColumns))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}
	
	// Select a different frame time (timing 4)
	printf("\nSet camera mode to 4!\n");
	if ((nRet=Acquisition_SetCameraMode(hAcqDesc, 4))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}
	Acquisition_SetFrameSyncMode(hAcqDesc,HIS_SYNCMODE_FREE_RUNNING);

	// Set acquisition data to use it in callback
	dwDemoParam = ACQ_SNAP;
#ifdef __X64
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, (void*)&dwDemoParam))!=HIS_ALL_OK) goto exit;
#else
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, dwDemoParam))!=HIS_ALL_OK) goto exit;
#endif

	// Continuous acquisition
	printf("\nAcquire Image!\n");
	if ((nRet=Acquisition_Acquire_Image(hAcqDesc, dwFrames, 0, HIS_SEQ_CONTINUOUS, NULL, NULL, NULL))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}

//----------------------------------------------------------------------------------------------------------//
//	In the demonstration the main thread polls now the keyboard until a key is pressed. After that is		//
//	fires the abort request:																				//
//----------------------------------------------------------------------------------------------------------//
	sprintf(strBuffer, "Press a key to cancel acquisition!\n");
	WriteConsole(hOutput, strBuffer, strlen(strBuffer), &dwCharsWritten, NULL);
	
	Sleep(200);
	FlushConsoleInputBuffer(hInput);
	do
	{
		printf("warte..\n");
		WaitForSingleObject(hInput, INFINITE);
		ReadConsoleInput(hInput, &ir, 1, &dwRead);
	} while(!(ir.EventType==KEY_EVENT && ir.Event.KeyEvent.bKeyDown));


	printf("abort...\n");
	Acquisition_Abort(hAcqDesc);

//--------------------------------------------------------------------------------------------------//
// In a windowed application you should prevent all XISL calls until your application got a respond	//
// from the end of acquisition callback to the call of Acquisition_Abort.							//
//--------------------------------------------------------------------------------------------------//

	nRet = WaitForSingleObject(hevEndAcq, INFINITE);
	switch(nRet)
	{
	case WAIT_FAILED:
		MessageBox(NULL, "wait failed", "debug message", MB_OK | MB_ICONSTOP);
		goto Exit;
	case WAIT_OBJECT_0:
		break;
	case WAIT_ABANDONED:
		MessageBox(NULL, "wait abandoned by mutex", "debug message", MB_OK | MB_ICONSTOP);
		break;
	case WAIT_TIMEOUT:
		MessageBox(NULL, "wait time out occured", "debug message", MB_OK | MB_ICONSTOP);
		break;
	}

	hevEndAcq = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!hevEndAcq)
	{
		//error handling
		goto Exit;
	}
	//allocate memory for offset data
	pOffsetBuffer = malloc(dwRows*dwColumns*sizeof(short));
	dwDemoParam = ACQ_OFFSET;
#ifdef __X64
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, (void*)&dwDemoParam))!=HIS_ALL_OK) goto exit;
#else
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, dwDemoParam))!=HIS_ALL_OK) goto exit;
#endif
	printf("\nAcquire offset data now!\n"); //valtest

// Changing frame time to t0
// Note: It is not possible to change the frame time during a running acquisition.

	Acquisition_SetCameraMode(hAcqDesc, 0);

	//acquire 13 dark frames and average them
	if ((nRet=Acquisition_Acquire_OffsetImage(hAcqDesc, pOffsetBuffer, dwRows, dwColumns, 13))!=HIS_ALL_OK)
	{
		//error handler
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}

	//wait for end of acquisition event
	nRet = WaitForSingleObject(hevEndAcq, INFINITE);
	switch(nRet)
	{
	case WAIT_FAILED:
		MessageBox(NULL, "wait failed", "debug message", MB_OK | MB_ICONSTOP);
		goto Exit;
	case WAIT_OBJECT_0:
		break;
	case WAIT_ABANDONED:
		MessageBox(NULL, "wait abandoned by mutex", "debug message", MB_OK | MB_ICONSTOP);
		break;
	case WAIT_TIMEOUT:
		MessageBox(NULL, "wait time out occured", "debug message", MB_OK | MB_ICONSTOP);
		break;
	}
	
	strcpy(szFileName, "Offset.his");
	printf("Save Offset Image under filename:\n%s\n", szFileName);
	pFile = fopen(szFileName, "wb");
	if (pFile)
	{
		WinHeaderType FileHeader;
		int nDummy = 0;
		memset(&FileHeader, 0, sizeof(WinHeaderType));
		FileHeader.FileType=0x7000;
		FileHeader.HeaderSize = sizeof(WinHeaderType);
		FileHeader.ImageHeaderSize = 32;
		FileHeader.FileSize = FileHeader.HeaderSize+FileHeader.ImageHeaderSize+dwRows*dwColumns*sizeof(WORD);
		FileHeader.ULX = FileHeader.ULY = 1;
		FileHeader.BRX = (WORD)dwColumns;
		FileHeader.BRY = (WORD)dwRows;
		FileHeader.NrOfFrames = 1;
		FileHeader.TypeOfNumbers = 4;//DATASHORT;
		FileHeader.IntegrationTime = 0;
		fwrite(&FileHeader, sizeof(WinHeaderType), 1, pFile);
		fwrite(&nDummy, sizeof(nDummy), 8, pFile);
		fwrite(pOffsetBuffer, sizeof(WORD)*dwRows*dwColumns, 1, pFile);
		fclose(pFile);
	} else printf("Error creating Offset file!\n");

	printf("\nOffset corrected acquisition!\n");
	//set acquisition data to use it in callback
	dwDemoParam = ACQ_CONT | ACQ_OFFSET;
#ifdef __X64	
				if ((nRet=Acquisition_SetAcqData(hAcqDesc, (void*)&dwDemoParam))!=HIS_ALL_OK) goto exit;
#else
				if ((nRet=Acquisition_SetAcqData(hAcqDesc, dwDemoParam))!=HIS_ALL_OK) goto exit;
#endif

	//route acquisition buffer to XISL again
	if ((nRet=Acquisition_DefineDestBuffers(hAcqDesc, pAcqBuffer,
		dwFrames, dwRows, dwColumns))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}

	//--------------------------------------------------------------------------------------------------------------//
	// Image acquisition with offset correction																		//
	// Because we have offset images available we now can acquire offset corrected images.							//
	// As we have allocated memory for the acquisition buffer and passed its address to the XISL					//
	// we can start grabbing, e.g continiously																		//
	//--------------------------------------------------------------------------------------------------------------//
	if ((nRet=Acquisition_Acquire_Image(hAcqDesc, dwFrames, 0, HIS_SEQ_CONTINUOUS, pOffsetBuffer, NULL, NULL))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}
	//--------------------------------------------------------------------------------------------------------------------------//
	// Here the offset correction is done during acquisition. Alternatively you could perform this								//
	// task afterwards if uncorrected image data is already available. For that use Acquisition_DoOffsetCorrection which is		//
	// optimized for time critical tasks.																						//
	//--------------------------------------------------------------------------------------------------------------------------//
	
	FlushConsoleInputBuffer(hInput);

	do
	{
		WaitForSingleObject(hInput, INFINITE);
		ReadConsoleInput(hInput, &ir, 1, &dwRead);
	} while(!(ir.EventType==KEY_EVENT  && ir.Event.KeyEvent.bKeyDown));

	Acquisition_Abort(hAcqDesc);
	nRet = WaitForSingleObject(hevEndAcq, INFINITE);
	switch(nRet)
	{
	case WAIT_FAILED:
		MessageBox(NULL, "wait failed", "debug message", MB_OK | MB_ICONSTOP);
		goto Exit;
	case WAIT_OBJECT_0:
		break;
	case WAIT_ABANDONED:
		MessageBox(NULL, "wait abandoned by mutex", "debug message", MB_OK | MB_ICONSTOP);
		break;
	case WAIT_TIMEOUT:
		MessageBox(NULL, "wait time out occured", "debug message", MB_OK | MB_ICONSTOP);
		break;
	}

	//--------------------------------------------------------------------------------------------------------------//
	// Acquiring Gain/Offset Data																					//
	// After we acquired offset images we are able to acquire gain images either. It isn’t possible to				//
	// acquire valid gain images without offset images. First we have to allocate the buffer for the gain			//
	// image. The XISL uses a 32 bit data format for gain images. That’s why the allocation step is a				//
	// little bit different from that above:																		//
	//--------------------------------------------------------------------------------------------------------------//
	pGainBuffer = malloc(dwRows*dwColumns*sizeof(DWORD));
	dwDemoParam = ACQ_GAIN;
#ifdef __X64	
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, (void*)&dwDemoParam))!=HIS_ALL_OK) goto exit;
#else
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, dwDemoParam))!=HIS_ALL_OK) goto exit;
#endif
	printf("\nAcquire gain data now!\n");

	//--------------------------------------------------------------------------------------------------------------//
	// Now the sensor must be illuminated, so that we can acquire the gain images:									//
	//--------------------------------------------------------------------------------------------------------------//
	// acquire 17 bright frames with exposed sensor and average them
	if ((nRet=Acquisition_Acquire_GainImage(hAcqDesc, pOffsetBuffer, pGainBuffer, dwRows, dwColumns, 17))!=HIS_ALL_OK)
	{
		//error handler
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}

	// wait for end of acquisition event
	assert(hevEndAcq);
	nRet = WaitForSingleObject(hevEndAcq, INFINITE);
	switch(nRet)
	{
	case WAIT_FAILED:
		MessageBox(NULL, "wait failed", "debug message", MB_OK | MB_ICONSTOP);
		goto Exit;
	case WAIT_OBJECT_0:
		break;
	case WAIT_ABANDONED:
		MessageBox(NULL, "wait abandoned by mutex", "debug message", MB_OK | MB_ICONSTOP);
		break;
	case WAIT_TIMEOUT:
		MessageBox(NULL, "wait time out occured", "debug message", MB_OK | MB_ICONSTOP);
		break;
	}

	printf("\nGain/Offset corrected acquisition!\n");
	//set acquisition data to use it in callback
	dwDemoParam = ACQ_CONT;
#ifdef __X64	
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, (void*)&dwDemoParam))!=HIS_ALL_OK) goto exit;
#else
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, dwDemoParam))!=HIS_ALL_OK) goto exit;
#endif

	//route acquisition buffer to XISL again
	if ((nRet=Acquisition_DefineDestBuffers(hAcqDesc, pAcqBuffer,
		dwFrames, dwRows, dwColumns))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}

	//--------------------------------------------------------------------------------------------------------------//
	// Image acquisition with gain/offset correction																//
	// Because we have offset images available we now can acquire offset corrected images.							//
	// As we have allocated memory for the acquisition buffer and passed its address to the XISL					//
	// we can start grabbing, e.g continiously																		//
	//--------------------------------------------------------------------------------------------------------------//
	if ((nRet=Acquisition_Acquire_Image(hAcqDesc, dwFrames, 0, HIS_SEQ_CONTINUOUS, pOffsetBuffer, pGainBuffer, NULL))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}
	//----------------------------------------------------------------------------------------------------------------------//
	// Note: If you pass valid pointers to Acquisition_Acquire_Image for pOffsetData and pGainData the						//
	// correction is automatically executed in the XISL. There is no need to call the upper code fragment.					//
	// Also here, you could also do the correction after image acquisition by calling Acquisition_DoOffsetGainCorrection.	//
	//----------------------------------------------------------------------------------------------------------------------//
	
	FlushConsoleInputBuffer(hInput);

	do
	{
		WaitForSingleObject(hInput, INFINITE);
		ReadConsoleInput(hInput, &ir, 1, &dwRead);
	} while(ir.EventType!=KEY_EVENT);

	Acquisition_Abort(hAcqDesc);
	WaitForSingleObject(hevEndAcq, INFINITE);
//*/

	//----------------------------------------------------------------------------------------------------------------------//
	// Multi gain correction																								//
	// for the multi gain correction we have to:																			//
	// 1. create a sequence of offset-corrected bright images, each having a different intensity. In fact,					//
	// the intensity must increase with the image index in the sequence.													//
	// 2. create a list of Median values (gain map) representing the images of the above mentioned sequence					//
	// 3. Aquiere images with offset and multigain correction data linked													//
	//----------------------------------------------------------------------------------------------------------------------//
	hevEndAcq = CreateEvent(NULL, FALSE, FALSE, NULL);
	Acquisition_SetCameraMode(hAcqDesc,0);
	Acquisition_SetFrameSyncMode(hAcqDesc,HIS_SYNCMODE_FREE_RUNNING);

	// example: 2 point multi gain
	// first aquire 2 offset-corrected bright images. one for each intensity
	// and build a sequence of the data
	dwFrames=2;
	pGainSeqBuffer = malloc(dwFrames*dwRows*dwColumns*sizeof(short));

	// first we acquire a fresh offset image
	dwDemoParam = ACQ_OFFSET;
#ifdef __X64	
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, (void*)&dwDemoParam))!=HIS_ALL_OK) goto exit;
#else
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, dwDemoParam))!=HIS_ALL_OK) goto exit;
#endif
	
	Acquisition_SetCameraMode(hAcqDesc, 0);				// set timing, in this case timing 0

	//acquire offset of averaged e.g. 50 dark frames
	if ((nRet=Acquisition_Acquire_OffsetImage(hAcqDesc, pOffsetBuffer, dwRows, dwColumns, 50))!=HIS_ALL_OK)
	{
		//error handler
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}

	//wait for end of offset image acquisition
	nRet = WaitForSingleObject(hevEndAcq, INFINITE);

	// buffer for offset corrected bright image
	pBrightocBuffer=malloc(dwRows*dwColumns*sizeof(short));

	dwDemoParam = ACQ_Brightoc;
#ifdef __X64	
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, (void*)&dwDemoParam))!=HIS_ALL_OK) goto exit;
#else
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, dwDemoParam))!=HIS_ALL_OK) goto exit;
#endif

	if ((nRet=Acquisition_DefineDestBuffers(hAcqDesc, pBrightocBuffer,
			1, dwRows, dwColumns))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}

	// acquire offsetcorrected-avg bright image with low intensity
	if((Acquisition_Acquire_Image(hAcqDesc,15,0, 
			HIS_SEQ_AVERAGE, pOffsetBuffer, NULL, NULL))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}
	//wait for end of acquisition event
	nRet = WaitForSingleObject(hevEndAcq, INFINITE);
	
	// copy acquired data to sequence buffer
	memcpy(pGainSeqBuffer,pBrightocBuffer,dwRows*dwColumns*sizeof(short)); 
	
	// acquire offsetcorrected-avg bright image with higher intensity
	if((Acquisition_Acquire_Image(hAcqDesc,15,0, 
			HIS_SEQ_AVERAGE, pOffsetBuffer, NULL, NULL))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}
	//wait for end of acquisition event
	nRet = WaitForSingleObject(hevEndAcq, INFINITE);

	// copy acquired data to sequence buffer
	memcpy(pGainSeqBuffer+dwRows*dwColumns,pBrightocBuffer,dwRows*dwColumns*sizeof(short));

	// build buffer for median data to be used with the multi-gain correction
	pGainSeqMedBuffer= malloc(2*sizeof(short));
	
	// create gain map
	Acquisition_CreateGainMap(pGainSeqBuffer,pGainSeqMedBuffer,dwRows*dwColumns,2);
	
	// now acquire image with offset and multigain correction
	dwFrames=1;
	if (pAcqBuffer)
		free (pAcqBuffer);
	pAcqBuffer = malloc(dwRows*dwColumns*sizeof(short));
	
	//route acquisition buffer to XISL
	if ((nRet=Acquisition_DefineDestBuffers(hAcqDesc, pAcqBuffer,
		dwFrames, dwRows, dwColumns))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}
	dwDemoParam = ACQ_SNAP;
#ifdef __X64	
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, (void*)&dwDemoParam))!=HIS_ALL_OK) goto exit;
#else
	if ((nRet=Acquisition_SetAcqData(hAcqDesc, dwDemoParam))!=HIS_ALL_OK) goto exit;
#endif
	if((Acquisition_Acquire_Image_Ex(hAcqDesc,30,0, 
			HIS_SEQ_AVERAGE,pOffsetBuffer,2,pGainSeqBuffer,pGainSeqMedBuffer,NULL,NULL))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}
	
	//wait for end of acquisition event
	nRet = WaitForSingleObject(hevEndAcq, INFINITE);


	
	//--------------------------------------------------------------------------------------------------------------//
	// Pixel correction																								//
	//--------------------------------------------------------------------------------------------------------------//
	printf("\nPixel corrected acquisition!\n");
	//now we want to perform a pixel correction of pixel (100,102) and pixel 125,113)

	// create a defect pixel image or load the defect pixel image which is already exiting (e.g. from the PKI detector CD) 
	pwPixelMapData = (unsigned short*) malloc( dwRows*dwColumns*(sizeof (unsigned short)));

	memset(pwPixelMapData,0,dwRows*dwColumns*sizeof (unsigned short)); 
	pwPixelMapData [100 + 102* dwColumns] = 0xFFFF; // mark pixel in image as defect
	pwPixelMapData [125 + 113* dwColumns] = 0xFFFF; // mark pixel in image as defect

	pCorrList = NULL;
	iListSize=0;
	// 1) retrieve the required size for the list "&iListSize"
	Acquisition_CreatePixelMap(pwPixelMapData, dwColumns, dwRows, pCorrList, &iListSize);
	// 2) Allocate the memory for the list
	pCorrList = (int *) malloc(iListSize);
	// 3) Import the data into the list
	Acquisition_CreatePixelMap(pwPixelMapData, dwColumns, dwRows, pCorrList, &iListSize);
	// 4a) Correct the already acquired data
	// Acquisition_DoPixelCorrection(pwImageData,pCorrList);
	// or
	// 4b) Correction during acquisition as done below
	//Acquisition_Acquire_Image(hAcqDesc, ...., (DWORD*)pCorrList)

	if ((nRet=Acquisition_Acquire_Image(hAcqDesc, dwFrames, 0, HIS_SEQ_CONTINUOUS, pOffsetBuffer, pGainBuffer, pCorrList))!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		goto Exit;
	}
	Sleep(200);
	if (!FlushConsoleInputBuffer(hInput)) printf("Console flush didn't work!");

	do
	{
		WaitForSingleObject(hInput, INFINITE);
		ReadConsoleInput(hInput, &ir, 1, &dwRead);
	} while(!(ir.EventType==KEY_EVENT && ir.Event.KeyEvent.bKeyDown));

	Acquisition_Abort(hAcqDesc);
	WaitForSingleObject(hevEndAcq, INFINITE);
//*/
Exit:
exit:
	//close acquisition and clean up
	//free event object
	if (hevEndAcq) CloseHandle(hevEndAcq);
	hevEndAcq = NULL;
	if (pAcqBuffer) free(pAcqBuffer);
	if (pOffsetBuffer) free(pOffsetBuffer);
	if (pGainBuffer) free(pGainBuffer);
	if (pPixelBuffer) free(pPixelBuffer);
	
	if ((nRet=Acquisition_CloseAll())!=HIS_ALL_OK)
	{
		//error handling
		char szMsg[300];
		sprintf(szMsg, "Error nr.: %d", nRet);
		MessageBox(NULL, szMsg, "debug message!", MB_OK | MB_ICONSTOP);
		return 0;
	}

	if (hOutput)
	{
		CloseHandle(hOutput);
	}

	if (hInput)
	{
		CloseHandle(hInput);
	}
	return 0;
}