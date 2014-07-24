//----------------------------------------------------------------------
//
//  RadTest.cpp
//  ===========
//  Simple example code for performing Radiographic image acquisition
//  and calibration.
//
//  17 MAY 2005  MVH
//   7 SEP 2005  CW - added software handshaking version
//
//----------------------------------------------------------------------

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "HcpErrors.h"
#include "HcpFuncDefs.h"
#include "iostatus.h"

// In the following string, "1234" should be replaced with the imager serial number
char *default_path = "C:\\IMAGERs\\1234"; // Path to IMAGER tables

// In the following the mode number for the rad mode required should be set
int  crntModeSelect = 0;

SSysInfo   sysInfo;

SModeInfo  modeInfo;

UQueryProgInfo crntStatus;
UQueryProgInfo prevStatus;  // So we can tell when crntStatus changes

#define ESC_KEY   (0x1B)
#define ENTER_KEY (0x0D)

int  crntIoState;
int  crntExpState;
int  prevIoState = -1;
int  prevExpState = -1;



#define LAST_IO_STATE  (14)
#define LAST_EXP_STATE  (6)

char *ioStateMsg[] =
{
	"IO_STANDBY",			//  0
	"IO_PREP",				//  1
	"IO_READY",				//  2
	"IO_ACQ",				//  3
	"IO_FETCH",				//  4
	"IO_DONE",				//  5
	"IO_ABORT",				//  6
	"IO_INIT",				//  7
	"IO_INIT_ERROR",		//  8
};

char *expStateMsg[] =
{
	"EXP_STANDBY",				//  0
	"EXP_AWAITING_PERMISSION",	//  1
	"EXP_PERMITTED",			//  2
	"EXP_REQUESTED",			//  3
	"EXP_CONFIRMED",			//  4
	"EXP_TIMED_OUT",			//  5
	"EXP_COMPLETED",			//  6
};
//-----------for fluro mode -----------------------------------

#ifndef VIP_NO_ERR
#define VIP_NO_ERR 0
#endif

#ifndef MAX_STR
#define MAX_STR 256
#endif
#define RECORD_TIMEOUT 100
#define MAX_HCP_ERROR_CNT 65536
//////////////////////////////////////////////////////////////////////////////
// a thread used to display or process images in real time
DWORD WINAPI ImgDisplayThread(LPVOID lpParameter);
// function to retrieve error string
void GetCPError(int errCode, char* errMsg);
int  DoCal(int mode);
int  FluoroAnalogOffsetCal(int mode);
int  FluoroGainCal(int mode);
int  FluoroOffsetCal(int mode);
int  CheckRecLink();

//////////////////////////////////////////////////////////////////////////////
// some globals
char GFrameReadyName[MAX_STR]="";
//char GFrameRequestName[MAX_STR]=""; // this would be used for Just-In-Time
										// corrections

static	bool	GGrabbingIsActive=false; 
static	int		GImgSize=0;
static	int		GImgX=0;
static	int		GImgY=0;
//static	SLivePrms*	GLiveParams=NULL;
static	long	GNumFrames=0;
static	long	GImgDisplayThreadIsAlive=0;	


//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------
//
//  queryProgress
//
//----------------------------------------------------------------------

int queryProgress(bool showAll = false)
{
	memset(&crntStatus, 0, sizeof(SQueryProgInfo));
	crntStatus.qpi.StructSize = sizeof(SQueryProgInfo);
	int result = vip_query_prog_info(HCP_U_QPI, &crntStatus);

	if (result == HCP_NO_ERR)
	{
		if (showAll
		|| (prevStatus.qpi.NumFrames != crntStatus.qpi.NumFrames)
		|| (prevStatus.qpi.Complete != crntStatus.qpi.Complete)
		|| (prevStatus.qpi.NumPulses != crntStatus.qpi.NumPulses)
		|| (prevStatus.qpi.ReadyForPulse != crntStatus.qpi.ReadyForPulse))
		{
			printf("vip_query_prog_info: frames=%d complete=%d pulses=%d ready=%d\n",
				crntStatus.qpi.NumFrames,
				crntStatus.qpi.Complete,
				crntStatus.qpi.NumPulses,
				crntStatus.qpi.ReadyForPulse);

			prevStatus.qpi = crntStatus.qpi;
		}
	}
	else
		printf("**** vip_query_prog_info returns error %d\n", result);

	return result;
}


//----------------------------------------------------------------------
//
//  ioQueryStatus
//
//----------------------------------------------------------------------

int ioQueryStatus(bool showAll = false)
{
	int result;

	result = vip_io_query_status(&crntIoState, &crntExpState);
	
	if (result == HCP_NO_ERR)
	{
		if (showAll
		|| (crntIoState != prevIoState)
		|| (crntExpState != prevExpState))
		{
			if ((0 <= crntIoState) && (crntIoState <= LAST_IO_STATE)
			&& (0 <= crntExpState) && (crntExpState <= LAST_EXP_STATE))
			{
				printf("vip_io_query_status: %s, %s\n",
					ioStateMsg[crntIoState],
					expStateMsg[crntExpState]);
			}
			else
				printf("vip_io_query_status: ILLEGAL VALUE (%d, %d)", crntIoState, crntExpState);

			prevIoState = crntIoState;
			prevExpState = crntExpState;
		}
	}
	else
		printf("**** vip_io_query_status returns error %d\n", result);

	return result;
}


//----------------------------------------------------------------------
//
//  checkAcqStatus
//
//----------------------------------------------------------------------

int checkAcqStatus()
{
	int result;

	result = ioQueryStatus();
	if (crntExpState == EXP_AWAITING_PERMISSION)
	{
		vip_io_permit_exposure();
	}
	if (result == HCP_NO_ERR)
	{
		result = queryProgress();
	}
	return result;
}


//----------------------------------------------------------------------
//
//  performOffsetCalibration
//
//----------------------------------------------------------------------

int  performOffsetCalibration()
{
	int modeNum = crntModeSelect;

	int result = vip_reset_state();

	printf("Starting offset calibration\n");
	result = vip_offset_cal(modeNum);
	if (result != HCP_NO_ERR)
	{
		printf("*** vip_offset_cal returns error %d\n", result);
		return result;
	}

	result = queryProgress(true);
	if (result != HCP_NO_ERR)
		return result;

	while (crntStatus.qpi.Complete)
	{
		if (_kbhit())
		{
			vip_reset_state();
			return -1;
		}

		Sleep(50);

		result = queryProgress();
		if (result != HCP_NO_ERR)
			return result;
	}

	while (!crntStatus.qpi.Complete)
	{
		if (_kbhit())
		{
			vip_reset_state();
			return -1;
		}

		Sleep(50);

		result = queryProgress();
		if (result != HCP_NO_ERR)
			return result;
	}

	printf("Offset calibration complete\n");

	return 0;
}


//----------------------------------------------------------------------
//
//  performHwGainCalibration
//
//----------------------------------------------------------------------

int  performHwGainCalibration()
{
	int  numGainCalImages = 4;
	int  modeNum = crntModeSelect;

// NOTE: for simplicity some error checking left out in the following
	int numOffsetCalFrames=2;
	vip_get_num_cal_frames(crntModeSelect, &numOffsetCalFrames);

	int  keyCode;

	printf("Gain calibration\n");

	int  result = vip_reset_state();

	result = vip_gain_cal_prepare(modeNum, false);
	if (result != HCP_NO_ERR)
	{
		printf("*** vip_gain_cal_prepare returns error %d\n", result);
		return result;
	}
	result = vip_sw_handshaking(VIP_SW_PREPARE, 1);
	if (result != HCP_NO_ERR)
	{
		printf("*** vip_sw_handshaking(VIP_SW_PREPARE, 1) returns error %d\n", result);
		return result;
	}

	printf("Performing initial offset calibration prior to gain calibration\n");
	printf("Press Esc to exit without modifying calibration files\n");

	do
	{
		result = queryProgress();
		if (result != HCP_NO_ERR)
			return result;

		SleepEx(50, FALSE);
		if (_kbhit())
		{
			printf("Key pressed - exiting calibration\n");
			vip_reset_state();
			return -1;
		}
	} while (crntStatus.qpi.NumFrames < numOffsetCalFrames);
	printf("Initial offset calibration complete\n\n");

	vip_io_enable(HS_ACTIVE);
	printf("Ready for X-rays, use handswitch to initiate acquisitions\n");
	printf("Acquiring %d images - press Enter to finish calibration with fewer images\n", numGainCalImages);
	printf("Press Esc to exit without modifying calibration files\n");
	while (crntStatus.qpi.NumPulses < numGainCalImages)
	{
		SleepEx(100, FALSE);
		result = checkAcqStatus();
		if (result != HCP_NO_ERR)
			return result;

		if (_kbhit())
		{
			keyCode = _getch();
			if (keyCode == ESC_KEY)
			{
				printf("Esc key pressed - exiting calibration\n");
				vip_reset_state();
				return -1;
			}
			else if (keyCode == ENTER_KEY)
			{
				printf("Enter key pressed - finishing calibration\n");
				break;
			}
		}
	}
	
	printf("Setting PREPARE=0\n");
	result = vip_sw_handshaking(VIP_SW_PREPARE, 0);

	while (!crntStatus.qpi.Complete)
	{
		SleepEx(500, FALSE);
		queryProgress();
		if (_kbhit())
		{
			keyCode = _getch();
			if (keyCode == ESC_KEY)
			{
				printf("Esc key pressed - forcing exit from calibration\n");
				vip_reset_state();
				return -1;
			}
		}
	}

	printf("Gain calibration complete\n");

	return 0;
}
//----------------------------------------------------------------------
//
//  performSwGainCalibration
//
//----------------------------------------------------------------------

int  performSwGainCalibration()
{
	int  numGainCalImages = 4;
	int  keyCode;

	printf("Gain calibration\n");

	int  result = vip_reset_state();

// NOTE: for simplicity some error checking left out in the following
	int numOfstCal=2;
	vip_get_num_cal_frames(crntModeSelect, &numOfstCal);

	result = vip_gain_cal_prepare(crntModeSelect, false);
	if (result != HCP_NO_ERR)
	{
		printf("*** vip_gain_cal_prepare returns error %d\n", result);
		return result;
	}
	result = vip_sw_handshaking(VIP_SW_PREPARE, 1);
	if (result != HCP_NO_ERR)
	{
		printf("*** vip_sw_handshaking(VIP_SW_PREPARE, 1) returns error %d\n", result);
		return result;
	}

	printf("Performing initial offset calibration prior to gain calibration\n");
	printf("Press Esc to exit without modifying calibration files\n");

	do
	{
		result = queryProgress();
		if (result != HCP_NO_ERR)
			return result;

		SleepEx(50, FALSE);
		if (_kbhit())
		{
			printf("Key pressed - exiting calibration\n");
			vip_reset_state();
			return -1;
		}
	} while (crntStatus.qpi.NumFrames < numOfstCal);
	printf("Initial offset calibration complete\n\n");

	int numPulses=0;
	
	vip_enable_sw_handshaking(TRUE);

	printf("\nPress any key to begin flat field acquisition\n");	
	while(!_kbhit()) Sleep (100);

	while (1)
	{
		crntStatus.qpi.ReadyForPulse = FALSE;

		while(!crntStatus.qpi.ReadyForPulse)
		{
			result = queryProgress();
			if(result != HCP_NO_ERR) return result;
			if(crntStatus.qpi.ReadyForPulse) break;
			SleepEx(100, FALSE);
		}
		vip_sw_handshaking(VIP_SW_VALID_XRAYS, TRUE);
		Sleep(300);

		printf("Ready for X-rays, EXPOSE NOW\n");

		crntStatus.qpi.NumPulses = numPulses;
		while(crntStatus.qpi.NumPulses == numPulses)
		{
			result = queryProgress();
			if(result != HCP_NO_ERR) return result;
			if(crntStatus.qpi.NumPulses != numPulses) break;
			SleepEx(100, FALSE);
		}
		numPulses = crntStatus.qpi.NumPulses;

		printf("Number of pulses=%d", numPulses);
		if(numPulses == numGainCalImages) break;
	}
	
	printf("Setting PREPARE=0\n");
	result = vip_sw_handshaking(VIP_SW_PREPARE, 0);

	while (!crntStatus.qpi.Complete)
	{
		SleepEx(500, FALSE);
		queryProgress();
		if (_kbhit())
		{
			keyCode = _getch();
			if (keyCode == ESC_KEY)
			{
				printf("Esc key pressed - forcing exit from calibration\n");
				vip_reset_state();
				return -1;
			}
		}
	}

	printf("Gain calibration complete\n");

	// no need to do this if not using Hw handshaking at all
	vip_enable_sw_handshaking(FALSE);

	return 0;
}

//----------------------------------------------------------------------
//
//  getModeInfo
//
//----------------------------------------------------------------------

void getModeInfo()
{
	int result = 0;

	modeInfo.StructSize = sizeof(SModeInfo);
	printf("Calling vip_get_mode_details\n");

	result = vip_get_mode_info(crntModeSelect, &modeInfo);

	if (result == HCP_NO_ERR)
	{
		printf("  >> ModeDescription=\"%s\"\n", modeInfo.ModeDescription);
		printf("  >> AcqType=             %5d\n", modeInfo.AcqType);
		printf("  >> FrameRate=          %6.3f, AnalogGain=         %6.3f\n",
			modeInfo.FrameRate, modeInfo.AnalogGain);
		printf("  >> LinesPerFrame=       %5d, ColsPerFrame=        %5d\n",
			modeInfo.LinesPerFrame, modeInfo.ColsPerFrame);
		printf("  >> LinesPerPixel=       %5d, ColsPerPixel=        %5d\n",
			modeInfo.LinesPerPixel, modeInfo.ColsPerPixel);
	}
	else
		printf("**** vip_get_mode_info returns error %d\n", result);
}


//----------------------------------------------------------------------
//
//  getSysInfo
//
//----------------------------------------------------------------------

void getSysInfo()
{
	printf("Calling vip_get_sys_info\n");

	sysInfo.StructSize = sizeof(SSysInfo);

	int   result =	vip_get_sys_info(&sysInfo);

	if (result == HCP_NO_ERR)
	{
		printf("  >> SysDescription=\"%s\"\n", sysInfo.SysDescription);
		printf("  >> NumModes=         %5d,   DfltModeNum=   %5d\n",
			sysInfo.NumModes, sysInfo.DfltModeNum);
		printf("  >> MxLinesPerFrame=  %5d,   MxColsPerFrame=%5d\n",
			sysInfo.MxLinesPerFrame, sysInfo.MxColsPerFrame);
		printf("  >> MxPixelValue=     %5d,   HasVideo=      %5d\n",
			sysInfo.MxPixelValue, sysInfo.HasVideo);
		printf("  >> StartUpConfig=    %5d,   NumAsics=      %5d\n",
			sysInfo.StartUpConfig, sysInfo.NumAsics);
		printf("  >> ReceptorType=     %5d\n", sysInfo.ReceptorType);
	}
	else
		printf("**** vip_get_sys_info returns error %d\n", result);
}


//----------------------------------------------------------------------
//
//  showImageStatistics
//
//----------------------------------------------------------------------

void showImageStatistics(int npixels, USHORT *image_ptr)
{
	int nTotal;
	long minPixel, maxPixel;
	int i;
	double pixel, sumPixel;

	nTotal = 0;
	minPixel = 4095;
	maxPixel = 0;

	sumPixel = 0.0;

	for (i = 0; i < npixels; i++)
	{
		pixel = (double) image_ptr[i];
		sumPixel += pixel;
		if (image_ptr[i] > maxPixel)
			maxPixel = image_ptr[i];
		if (image_ptr[i] < minPixel)
			minPixel = image_ptr[i];
		nTotal++;
	}

	printf("Image: %d pixels, average=%9.2f min=%d max=%d\n",
		nTotal, sumPixel / nTotal, minPixel, maxPixel);
}


//----------------------------------------------------------------------
//
//  writeImageToFile
//
//----------------------------------------------------------------------

void writeImageToFile(void)
{
	int result;
	int mode_num = crntModeSelect;

	int x_size = modeInfo.ColsPerFrame;
	int y_size = modeInfo.LinesPerFrame;
	int npixels = x_size * y_size;

	USHORT *image_ptr = (USHORT *)malloc(npixels * sizeof(USHORT));

	result = vip_get_image(mode_num, VIP_CURRENT_IMAGE, x_size, y_size, image_ptr);

	if(result == HCP_NO_ERR)
	{
		char filename[32] = "newimage.raw";

		// file on the host computer for storing the image
		FILE *finput = fopen(filename, "wb");
		if (finput == NULL)
		{
			printf("Error opening image file to put file.");
			exit(-1);
		}

		fwrite(image_ptr, sizeof(USHORT), npixels, finput);
		fclose(finput);

		showImageStatistics(npixels, image_ptr);
	}
	else
	{
		printf("*** vip_get_image returned error %d\n", result);
	}

	free(image_ptr);
}

int  CheckRecLink()
{
	SCheckLink clk;
	memset(&clk, 0, sizeof(SCheckLink));
	clk.StructSize = sizeof(SCheckLink);
	int result = vip_check_link(&clk);
	while(result != HCP_NO_ERR)
	{
		 Sleep(1000);
		 result = vip_check_link(&clk);
	}
	return result;
}

//----------------------------------------------------------------------
//
//  performHwRadAcquisition
//
//----------------------------------------------------------------------

int  performHwRadAcquisition()
{
	int  result;

	{
		// just before enabling an acquisition, we want to make sure that 
		// we are calibrated and have correction files to do what's requested
		SCorrections corr;
		memset(&corr, 0, sizeof(SCorrections));
		corr.StructSize = sizeof(SCorrections);
		result = vip_get_correction_settings(&corr);
		// primarily interested that we get back HCP_NO_ERR..
		// HCP_NO_ERR - all requested corrections are available
		// HCP_OFST_ERR - no corrections are available
		// HCP_GAIN_ERR - of requested corrections only offset is available
		// HCP_DFCT_ERR - of requested corrections only offset and gain are available
		
		
		// Note that:--
		// --This call is made here to determine whether 
		// we have the requested correction capability. The return value 
		// MUST be HCP_NO_ERR before an acquisition is done.
		// --Currently requested corrections are those set in the receptor
		// configuration file or the last call to vip_set_correction_settings.

		// --The values returned in the SCorrections returned above reflect 
		// currently requested corrections. The return value reflects whether 
		// they are all available. 
		
		if (result != HCP_NO_ERR)
		{
			switch (result)
			{
			case HCP_OFST_ERR:
				printf("Requested corrections not available: offset file missing\n");
				break;
			case HCP_GAIN_ERR:
				printf("Requested corrections not available: gain file missing\n");
				break;
			case HCP_DFCT_ERR:
				printf("Requested corrections not available: defect file missing\n");
				break;
			}
			return result;
		}
	}

	result = CheckRecLink();
	if(result != HCP_NO_ERR)
	{
		printf("Receptor is NOT ready for rad acquisition\n");
		return -1;
	}

	printf("Enabling handswitch...");
	result = vip_io_enable(HS_ACTIVE);

	if (result == HCP_NO_ERR)
	{
		printf("OK\n");
		printf("Ready for rad acquisition\n");
	}
	else
	{
		printf("*** vip_io_enable returns error %d - HANDSWITCH NOT ENABLED\n", result);
		return result;
	}

	if (HCP_NO_ERR != (result = checkAcqStatus()))
		return result;

	while (!crntStatus.qpi.ReadyForPulse)
	{
		if (_kbhit())
			return -1;

		Sleep(50);
		result = checkAcqStatus();
		if (result != HCP_NO_ERR)
			return result;
	}

	while (!crntStatus.qpi.Complete)
	{
		if (_kbhit())
			return -1;

		Sleep(50);
		result = checkAcqStatus();
		if (result != HCP_NO_ERR)
			return result;
	}

	writeImageToFile();

	while (crntIoState != IO_STANDBY)
	{
		if (_kbhit())
			return -1;

		Sleep(50);
		result = checkAcqStatus();
		if (result != HCP_NO_ERR)
			return result;
	}

	return 0;
}

//----------------------------------------------------------------------
//
//  performSwRadAcquisition
//
//----------------------------------------------------------------------
int  performSwRadAcquisition()
{
	// This mode of operation may be used where no I/O card exists. Exact
	// synchronization with the xray generator may not be possible and this mode 
	// is only possible for relatively long integration times (probably several 
	// seconds at least).
	//
	// NOT SUITED FOR MEDICAL APPLICATION!
	//


	// STEP 1:
	// Before enabling an acquisition, we want to make sure that 
	// we are calibrated and have correction files to do what's requested
	printf("\nChecking correction capability...\n");
	SCorrections corr;
	memset(&corr, 0, sizeof(SCorrections));
	corr.StructSize = sizeof(SCorrections);
	int result = vip_get_correction_settings(&corr);
	// primarily interested that we get back HCP_NO_ERR..
	// HCP_NO_ERR - all requested corrections are available
	// HCP_OFST_ERR - no corrections are available
	// HCP_GAIN_ERR - of requested corrections only offset is available
	// HCP_DFCT_ERR - of requested corrections only offset and gain are available
	
	// Note that:--
	// --This call is made here to determine whether 
	// we have the requested correction capability. The return value 
	// MUST be HCP_NO_ERR before an acquisition is done.
	// --Currently requested corrections are those set in the receptor
	// configuration file or the last call to vip_set_correction_settings.

	// --The values returned in the SCorrections returned above reflect 
	// currently requested corrections. The return value reflects whether 
	// they are all available. 
	
	if (result != HCP_NO_ERR)
	{
		switch (result)
		{
		case HCP_OFST_ERR:
			printf("*** Requested corrections not available: offset file missing\n");
			break;
		case HCP_GAIN_ERR:
			printf("*** Requested corrections not available: gain file missing\n");
			break;
		case HCP_DFCT_ERR:
			printf("*** Requested corrections not available: defect file missing\n");
			break;
		}
	}
	else
	{
	// STEP 2:
	// Check we have valid acquisition mode set and get number of acq frames
		printf("Checking mode...\n");
		int modeType=-1, acqNum=-1;
		result = vip_get_mode_acq_type(crntModeSelect, &modeType, &acqNum);
		if(result == HCP_NO_ERR)
		{
			if(modeType != VIP_VALID_XRAYS_N_FRAMES || acqNum <= 0)
			{
				printf("*** Problem with mode settings; modeType=%d; acqNum=%d",
													modeType, acqNum);
				result = HCP_OTHER_ERR;
			}
		}
	}

	// STEP 3:
	// Enable software handshaking
	result = CheckRecLink();
	if(result != HCP_NO_ERR)
	{
		printf("Receptor is NOT ready for rad acquisition\n");
	}

	// STEP 4:
	// Enable software handshaking
	if (result == HCP_NO_ERR)
	{
		printf("Enabling software handshaking...\n");
		result = vip_enable_sw_handshaking(TRUE);

		if (result != HCP_NO_ERR)
		{
			printf("*** vip_enable_sw_handshaking returned error %d\n", result);
		}
	}
	
	// STEP 5:
	// Set frame rate (otherwise we can go with what's in the receptor config file
	// provided it will work.)
	if (result == HCP_NO_ERR)
	{
		printf("Setting frame rate...\n");
		result = vip_set_frame_rate(crntModeSelect, 0.1); // 10 second integration

		if (result == HCP_NO_ERR)
		{
			printf("\nOK, Ready for rad acquisition\nPress any key to START\n");
		}
		else
		{
			printf("*** vip_set_frame_rate returned error %d\n", result);
		}
	}

	// STEP 6:
	// Set PREPARE = TRUE
	if (result == HCP_NO_ERR)
	{
		while(!_kbhit()) Sleep (100);
		printf("\nSetting PREPARE=TRUE...\n");
		result = vip_sw_handshaking(VIP_SW_PREPARE, TRUE);

		if (result != HCP_NO_ERR)
		{
			printf("*** vip_sw_handshaking returned error %d\n", result);
		}
	}

	// SYNCHRONIZATION NOTE 1:
	// When PREPARE=TRUE is received the regular beat of frame reads is interrupted.
	// The next frame read is sent after a programmed delay which is normally
	// 1000ms. It is increased by any generator warmup delay set in the ini file (see 
	// 'VirtualCpInterface.pdf' section 6.3.1). 

	// STEP 7:
	// Wait for ReadyForPulse to go TRUE
	if (result == HCP_NO_ERR)
	{
		crntStatus.qpi.ReadyForPulse = FALSE;
		printf("Waiting for ReadyForPulse == TRUE...\n");
		while (result == HCP_NO_ERR)
		{
			result = queryProgress(true);
			if(crntStatus.qpi.ReadyForPulse == TRUE) break;
			Sleep(100);
		}

		if (result != HCP_NO_ERR)
		{
			printf("*** vip_query_prog_info returned error %d\n", result);
		}
	}

	// SYNCHRONIZATION NOTE 2:
	// ReadyForPulse is set to TRUE after the next frame read has begun.
	// The window for xray exposure may be assumed to begin approximately after
	// 300ms (expected maximum panel readout time) from ReadyForPulse == TRUE,
	// and end after the frame period has elapsed. 
	// Here the xray window is apprximately 300 - 10000 ms after ReadyForPulse == TRUE.

	// STEP 8:
	// Set XRAY_VALID = TRUE
	if (result == HCP_NO_ERR)
	{
		printf("Setting XRAY_VALID=TRUE...\n");
		result = vip_sw_handshaking(VIP_SW_VALID_XRAYS, TRUE);

		if (result != HCP_NO_ERR)
		{
			printf("*** vip_sw_handshaking returned error %d\n", result);
		}
	}

	// STEP 9:
	// <<<<<<<< READY FOR XRAYS >>>>>>>
	if (result == HCP_NO_ERR)
	{
		printf("\n\nREADY FOR XRAYS!!\n");

		crntStatus.qpi.Complete = FALSE;
		printf("Waiting for Complete == TRUE...\n");
		while (result == HCP_NO_ERR)
		{
			result = queryProgress(true);
			if(crntStatus.qpi.Complete == TRUE) break;
			MessageBeep(MB_OK);
			Sleep(1000);
		}

		if (result != HCP_NO_ERR)
		{
			printf("*** vip_query_prog_info returned error %d\n", result);
		}
	}

	// STEP 10:
	// be sure the grabber stops Set PREPARE = FALSE
	if (result == HCP_NO_ERR)
	{
		printf("\nSetting PREPARE=FALSE...\n");
		result = vip_sw_handshaking(VIP_SW_PREPARE, FALSE);

		if (result != HCP_NO_ERR)
		{
			printf("*** vip_sw_handshaking returned error %d\n", result);
		}
	}

	// STEP 11
	// retrieve the image
	if (result == HCP_NO_ERR)
	{
		writeImageToFile();
	}

	// no need to do this if not using Hw handshaking at all
	vip_enable_sw_handshaking(FALSE);
	return result;
}


//----------------------------------------------------------------------
//
//  main
//
//----------------------------------------------------------------------

void showMenu(char* currShake)
{
	printf("\n------------------------------\n");
	printf("Select operation:\n");
	printf("1 - Acquire image\n");
	printf("2 - Perform offset calibration\n");
	printf("3 - Perform gain calibration\n");
	printf("4 - Toggle Handshaking -- Currently %s\n", currShake);
	printf("0 - Exit\n");

}


//----------------------------------------------------------------------
//
//  run
//
//----------------------------------------------------------------------

int run(int argc, char* argv[])
{
	BOOL hwHandShake = TRUE;
	char currShake[4]="HW";
	char *path = default_path;
	int choice = 0;
	int result;
	SOpenReceptorLink  orl;
	memset(&orl, 0, sizeof(SOpenReceptorLink));

	printf("RadTest - Sample Code for Radiographic Image Acquisition\n\n");

	if (argc > 1)			// Check for receptor path on the command line
		path = argv[1];

	orl.StructSize = sizeof(SOpenReceptorLink);
	strcpy(orl.RecDirPath, path);

// if we want to turn debug on so that it flushes to a file ..
// or other settings see Virtual CP Communications Manual uncomment
// and modify the following line if required

//	orl.DebugMode = HCP_DBG_ON_FLSH;

	printf("Opening link to %s\n", orl.RecDirPath);
	result = vip_open_receptor_link(&orl);
	
	// set up the SCorrections structure we will need below
	SCorrections corr;
	memset(&corr, 0, sizeof(SCorrections));
	corr.StructSize = sizeof(SCorrections);

	if (result == HCP_OFST_ERR ||
		result == HCP_GAIN_ERR ||
		result == HCP_DFCT_ERR)
	{
		// this means not all correction files are available
		// here we will just turn corrections off but IN REAL APPLICATION
		// WE MUST BE SURE CORRECTIONS ARE ON AND THE RECEPTOR IS CALIBRATED
		result = vip_set_correction_settings(&corr);
		if(result == VIP_NO_ERR) printf("\n\nCORRECTIONS ARE OFF!!");
	}

	if (result == HCP_NO_ERR)
	{
		getSysInfo();
		getModeInfo();

		result = vip_select_mode(crntModeSelect);
		if (result == HCP_NO_ERR)
		{
			showMenu(currShake);
			for (bool running = true; running;)
			{
				if (_kbhit())
				{
					int keyCode = _getch();
					printf("%c\n", keyCode);
					switch (keyCode)
					{
					case '1':
						if(hwHandShake) performHwRadAcquisition();
						else performSwRadAcquisition();
						break;
					case '2':
						performOffsetCalibration();
						break;
					case '3':
						if(hwHandShake) performHwGainCalibration();
						else performSwGainCalibration();
						break;
					case '4':
						hwHandShake = !hwHandShake;
						if(hwHandShake) strncpy(currShake, "HW", 4);
						else strncpy(currShake, "SW", 4);
						break;
					case '0':
						running = false;
						break;
					}
					if (running)
						showMenu(currShake);
				}
			}
		}
		else
			printf("vip_select_mode(%d) returns error %d\n", crntModeSelect, result);

		vip_close_link();
	}
	else
		printf("vip_open_receptor_link returns error %d\n", result);

	printf("\n**Hit any key to exit");
	_getch();
	while(!_kbhit()) Sleep (100);

	return 0;
}
int OpenLink()
{
	//////////////////////////////////////////////////////////////////////////
		// **** STEP 1 -- Open the link --
	SOpenReceptorLink orl;
	memset(&orl, 0, sizeof(SOpenReceptorLink));
	orl.StructSize = sizeof(SOpenReceptorLink);
	strncpy(orl.RecDirPath, default_path, MAX_STR);
// if we want to turn debug on then uncomment the following line...
//	orl.DebugMode = HCP_DBG_ON;
	vip_set_debug(TRUE);
	int result = vip_open_receptor_link(&orl);
	// note that vip_open_receptor_link automatically generates calls
	// to:	-- vip_fluoro_init_sys(..) (this call should not be needed by user)
	//		-- vip_select_mode(0)
	//		-- vip_fluoro_init_mode(0, ..) (this call should not be needed by user)
	// It also automatically allocates two grab buffers which the 
	// frame grabber will write to directly (ping-pong fashion) 
	// when grabbing is active.
	// set up the SCorrections structure we will need below
	SCorrections corr;
	memset(&corr, 0, sizeof(SCorrections));
 	corr.StructSize = sizeof(SCorrections);
	if (result == HCP_OFST_ERR ||
		result == HCP_GAIN_ERR ||
		result == HCP_DFCT_ERR)
	{
		// this means not all correction files are available
		// here we will just turn corrections off but IN REAL APPLICATION
		// WE MUST BE SURE CORRECTIONS ARE ON AND THE RECEPTOR IS CALIBRATED
		result = vip_set_correction_settings(&corr);
		if(result == VIP_NO_ERR) printf("\n\nCORRECTIONS ARE OFF!!");
	}
	int N=0;
	// this sample code assumes that we are using mode 0 
	// if not it can be modified to select another mode..
	// N = ..
	result = vip_select_mode(N);
	//		(automatically generates vip_fluoro_init_mode(N, ..)
	//////////////////////////////////////////////////////////////////////////
	if(result != VIP_NO_ERR)
	{
		return(0);
	}
	else
	{
		return(1);
	}
}

//////////////////////////////////////////////////////////////////////////////
int run2()
{
	char errMsg[MAX_STR]="";
	int numBuf=0, numFrm=0;
	bool msgShown=false;
	int acqType=-1;

	printf("\n------------------------------------------------------------");
	printf("\n--------- VIRTUAL CP SAMPLE CODE FOR FLUORO MODES ----------");
	printf("\n------------------------------------------------------------");
	printf("\n\n**Hit any key to send vip_open_receptor_link()");
 	while(!_kbhit()) Sleep (100);
	printf("\nSending vip_open_receptor_link()..\n");

	//////////////////////////////////////////////////////////////////////////
		// **** STEP 1 -- Open the link --
	// Before we do (almost) anything we must call vip_open_receptor_link()..
	SOpenReceptorLink orl;
	memset(&orl, 0, sizeof(SOpenReceptorLink));
	orl.StructSize = sizeof(SOpenReceptorLink);
	strncpy(orl.RecDirPath, default_path, MAX_STR);

// if we want to turn debug on then uncomment the following line...
//	orl.DebugMode = HCP_DBG_ON;
	vip_set_debug(TRUE);
	int result = vip_open_receptor_link(&orl);

	// note that vip_open_receptor_link automatically generates calls
	// to:	-- vip_fluoro_init_sys(..) (this call should not be needed by user)
	//		-- vip_select_mode(0)
	//		-- vip_fluoro_init_mode(0, ..) (this call should not be needed by user)
	// It also automatically allocates two grab buffers which the 
	// frame grabber will write to directly (ping-pong fashion) 
	// when grabbing is active.

	// set up the SCorrections structure we will need below
	SCorrections corr;
	memset(&corr, 0, sizeof(SCorrections));
 	corr.StructSize = sizeof(SCorrections);

	if (result == HCP_OFST_ERR ||
		result == HCP_GAIN_ERR ||
		result == HCP_DFCT_ERR)
	{
		// this means not all correction files are available
		// here we will just turn corrections off but IN REAL APPLICATION
		// WE MUST BE SURE CORRECTIONS ARE ON AND THE RECEPTOR IS CALIBRATED
		result = vip_set_correction_settings(&corr);
		if(result == VIP_NO_ERR) printf("\n\nCORRECTIONS ARE OFF!!");
	}

	int N=1;
	// this sample code assumes that we are using mode 0 
	// if not it can be modified to select another mode..
	// N = ..
	result = vip_select_mode(N);
	//		(automatically generates vip_fluoro_init_mode(N, ..)


	//////////////////////////////////////////////////////////////////////////
	if(result != VIP_NO_ERR)
	{
		printf("\nvip_open_receptor_link() returned %d", result);
		msgShown = true;
	}
	else
	{
		// **** STEP 2 -- Get mode info --
		// Find out our mode info..
		SModeInfo modeInfo;
		memset(&modeInfo, 0, sizeof(SModeInfo));
		modeInfo.StructSize = sizeof(SModeInfo);

		result = vip_get_mode_info(N, &modeInfo);

		GImgX = modeInfo.ColsPerFrame;
		GImgY = modeInfo.LinesPerFrame;
		GImgSize = GImgX * GImgY * sizeof(WORD); // image size in bytes
		acqType = modeInfo.AcqType & VIP_ACQ_MASK;
	}
	
	if(result == VIP_NO_ERR)
	{
		// make sure we have a fluoro mode
		if(acqType != VIP_ACQ_TYPE_CONTINUOUS)
		{
			result = -2;
			msgShown = true;
			strncpy(errMsg, "\n\nError: Invalid mode type returned by "
							"vip_get_mode_info().", MAX_STR);
		}
	
		// make sure we have an image size that's plausible
		const int maxSize = 2048 * 1536 * sizeof(WORD); // 4030A
		if(GImgSize <= 0 || GImgSize > maxSize)
		{
			result = -2;
			msgShown = true;
			strncpy(errMsg, "\n\nError: Invalid image size returned by "
							"vip_get_mode_info().", MAX_STR);
		}
	}

	/////////////////////////////////////////////////////////////////////////
	if(result != VIP_NO_ERR)
	{
		if(!msgShown) printf("\nvip_get_mode_info() returned %d", result);
		msgShown = true;
	}
	else
	{
		// do calibration if required
		int doCal=0;
		printf("\nDo you want to do a calibration YES=1 or NO=0 ?: ");
		scanf("%d",&doCal);
		if(doCal == 1) result = DoCal(N);
	}

	/////////////////////////////////////////////////////////////////////////
	if(result != VIP_NO_ERR)
	{
		if(!msgShown) printf("\nDoCal() returned %d", result);
		msgShown = true;
	}
	else
	{
		// check the receptor link is OK
		result = CheckRecLink();
	}

	/////////////////////////////////////////////////////////////////////////
	if(result != VIP_NO_ERR)
	{
		if(!msgShown) printf("\nvip_check_link() returned %d", result);
		msgShown = true;
	}
	else
	{
		// Get some user input as to how many buffers to allocate
		// and how many frames to acquire. Note that zero is valid for 
		// number of frames  to acquire - it is interpreted as continuous 
		// acquisition, terminated  by vip_fluoro_record_stop() or 
		// vip_fluoro_grabber_stop().
		int i=0;
		while(numBuf <= 0 || numBuf > 64 || numFrm < 0 || numFrm > numBuf)
		{
			if(i++) printf("\n\nPlease select valid values. #frames <= #buffers");
			// get input from user re allocate buffers
			printf("\nHow many sequence buffers do you want to allocate? "
					"(0-64): ");
			scanf("%d",&numBuf);

			// get input from user re number of frames to acquire
			printf("\nHow many frames do you want to capture? "
					"(0=continuous): ");
			scanf("%d",&numFrm);
		}

		// **** STEP 3 -- Set sequence info --
		// Set the sequence parameter info..
		SSeqPrms seqPrms;
		memset(&seqPrms, 0, sizeof(SSeqPrms));
		seqPrms.StructSize = sizeof(SSeqPrms);
		seqPrms.NumBuffers = numBuf;
		seqPrms.StopAfterN = numFrm; // zero is interpreted as acquire continuous
								// (writing to buffers in circular fashion)
		result = vip_fluoro_set_prms(HCP_FLU_SEQ_PRMS, &seqPrms);
		// we may not get all we want so ...
		numBuf = seqPrms.NumBuffers;
		if(numFrm > numBuf) numFrm = numBuf;
		printf("\nNumber of buffers allocated = %d", numBuf);	
	}


	//////////////////////////////////////////////////////////////////////////
	if(result != VIP_NO_ERR)
	{
		if(!msgShown) printf("\nvip_fluoro_set_prms() returned %d", result);
		msgShown = true;
	}
	else
	{
		// the grabber can be started 'PAUSED' in which case it will not be writing 
		// to the grab buffers until vip_fluoro_record_start() is called
		// (not really significant here)
		int paused=0;
		printf("\nDo you want to start PAUSED (1) or regular (0)?: ");
		scanf("%d",&paused);
		if(paused != 1) paused = 0;

		// When the MarkPixels is set, a line of pixels is overwritten
		// in the grabber buffers. The line length is the same as the frame number.
		// This should NOT be set in normal useage but can be useful for 
		// debug and verification.
		int markPix=0;
		printf("\nDo you want to create 'walking pixel line' YES=1; NO=0 ?: ");
		scanf("%d",&markPix);
		if(markPix != 1) markPix = 0;

		// **** STEP 4 -- Start the grabber --
		SAcqPrms acqPrms;
		memset(&acqPrms, 0, sizeof(SAcqPrms));
		acqPrms.StructSize = sizeof(SAcqPrms);
		acqPrms.MarkPixels = markPix;
		acqPrms.StartUp = paused;
		
		acqPrms.CorrType = HCP_CORR_STD; // This will be the normal setting. 
					// Only other option currently
					// is HCP_CORR_NONE. Setting CorrType=HCP_CORR_NONE 
					// effectively acts as an override to the settings in SCorrections.
					// 
					// Note that the various calls, that may return  
					// HCP_OFST_ERR / HCP_GAIN_ERR / HCP_DFCT_ERR etc,
					// do not take account of the CorrType setting. i.e if it is set to
					// HCP_CORR_NONE, corrections will not be done but an error code
					// could be returned if corrections are otherwise 
					// requested but not available.

		// For clarity we also set the following but actually these settings
		// are the defaults done by the memset above.
		acqPrms.CorrFuncPtr = NULL; // This should essentially always be set to NULL. 
									// Only if the user is responsible for making his 
									// own corrections should the user set this.
									// Consult Varian Medical Systems Engineering staff.
									// The NULL value is interpreted as pointing
									// corrections into the Virtual CP integrated
									// corrections module.
		acqPrms.ReqType = 0;		// ReqType is for internal use only. 
									// Must be left at zero always.

		// just before starting the grabber we want to make sure that 
		// we are calibrated and have correction files to do what's requested
		result = vip_get_correction_settings(&corr);
		// primarily interested that we get back HCP_NO_ERR..
		// HCP_NO_ERR - all requested corrections are available
		// HCP_OFST_ERR - no corrections are available
		// HCP_GAIN_ERR - of requested corrections only offset is available
		// HCP_DFCT_ERR - of requested corrections only offset and gain are available

		// Note that:--
		// --This call is made here to determine whether 
		// we have the requested correction capability. The return value 
		// MUST be HCP_NO_ERR before an acquisition is done.
		// --Currently requested corrections are those set in the receptor
		// configuration file or the last call to vip_set_correction_settings.

		// --The values returned in the SCorrections returned above reflect 
		// currently requested corrections. The return value reflects whether 
		// they are all available. 
		
		if(result == HCP_NO_ERR)
		{
			printf("\nSending vip_fluoro_grabber_start()");
			result = vip_fluoro_grabber_start(&acqPrms);

			// A pointer to a SLivePrms structure is returned in the acqPrms
			//GLiveParams = (SLivePrms*)acqPrms.LivePrmsPtr;
		}
	}

	// At this point the grabber should be running. Frames are being captured
	// direct into the ping-pong buffers (unless started PAUSED). 
	// They are not yet being saved into the sequence buffers.


	//////////////////////////////////////////////////////////////////////////
	if(result != VIP_NO_ERR)
	{
		if(!msgShown) printf("\nvip_fluoro_grabber_start() returned %d", result);
		msgShown = true;
	}
	else
	{
		
		// **** STEP 5 -- Set up real-time image handling as needed --
		// We probably want to do something with the frames as they are being 
		// captured e.g. display them. We can get the names of events 
		// set up in the Virtual CP that will be helpful.
		result = vip_fluoro_get_event_name(HCP_FG_FRM_TO_DISP, GFrameReadyName);

		// (If Just-In-Time corrections are implemented we can ask for the
		// event name that we will use to signal we are ready for a frame.)

		if(result == VIP_NO_ERR)
		{
			// Start the worker thread that will handle real-time tasks.
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ImgDisplayThread, 
									NULL, 0, NULL);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	if(result != VIP_NO_ERR)
	{
		if(!msgShown) printf("\nvip_fluoro_grabber_start() returned %d", result);
		msgShown = true;
	}
	else
	{
		fflush(stdin);
		Sleep (100);
		printf("\n\n**Hit any key to send vip_fluoro_record_start()\n");
		while(!_kbhit()) Sleep (100);

		// ---------------------------------------------------------------
		// THIS IS A GOOD POINT TO WAIT FOR AN EVENT IN A REAL APPLICATION
		// WHEN THERE EXISTS A NEED TO SYNCHRONIZE THE BEGINNING OF 
		// CAPTURING FRAMES WITH FOR EXAMPLE THE START OF A CT SCAN.
		// ---------------------------------------------------------------

		// **** STEP 6 -- Start the recording --
		// Now we will save frames being written to the grab buffers into the 
		// sequence buffers.
		printf("\nSending  vip_fluoro_record_start()");

		// If we want we can set StopAfterN and startFromBufIdx at this point too.. 
		// result = vip_fluoro_record_start(numFrm, startFromBufIdx);
		// OR ..
		result = vip_fluoro_record_start();
		// START #1
	}


	//////////////////////////////////////////////////////////////////////////
	if(result != VIP_NO_ERR)
	{
		if(!msgShown) printf("\nvip_fluoro_record_start() returned %d", result);
		msgShown = true;
	}
	else
	{
		// **** STEP 7 -- End recording --
		if(numFrm) // wait for the requested number of frames to complete
		{
			int to = RECORD_TIMEOUT * 1000; //ms.
			int lpNum = 200;
			int qRate = to / lpNum;
			if(qRate < 1) qRate = 1;
			int i;
			for(i=0; i<lpNum; i++)
			{
				if(GNumFrames >= numFrm) break;
				Sleep(qRate);
			}

			if(i == lpNum)
			{
				result = -2;
				msgShown = true;
				strncpy(errMsg, "\n\nError: Timed out waiting for frames.",
								MAX_STR);
			}
		}
		else // user stops at any time
		{
			printf("\n\n**Hit any key to send vip_fluoro_record_stop()");
			_getch();
  			while(!_kbhit()) Sleep (100);
		}

		printf("\nSending  vip_fluoro_record_stop()");
		result = vip_fluoro_record_stop();
		// STOP #1
	}

	// if you want you can capture another segment
	int doMore=0;
	if(result == VIP_NO_ERR)
	{
		printf("\nDo you want to capture another segment YES=1 or NO=0 ?: ");
		scanf("%d",&doMore);
	}

	//////////////////////////////////////////////////////////////////////////
	if(doMore == 1)
	{
		printf("\nEnter TOTAL # of frames to acquire "
				"(include prior segment)?: ");
		scanf("%d", &numFrm);

		printf("\nEnter first buffer index for 2nd segment "
				"(-1 saves contiguous to 1st)?: ");
		int bufIdx=-1;
		scanf("%d", &bufIdx);

		fflush(stdin);
		printf("\n\n**Hit any key to send vip_fluoro_record_start()\n");
		while(!_kbhit()) Sleep (100);

		printf("\nSending  vip_fluoro_record_start()");

		result = vip_fluoro_record_start(numFrm, bufIdx);
		// START #2
	}


	//////////////////////////////////////////////////////////////////////////
	if(result != VIP_NO_ERR && doMore == 1)
	{
		if(!msgShown) printf("\nvip_fluoro_record_start() returned %d", result);
		msgShown = true;
	}
	else if(doMore == 1)
	{
		if(numFrm) // wait for the requested number of frames to complete
		{
			int to = RECORD_TIMEOUT * 1000; //ms.
			int lpNum = 200;
			int qRate = to / lpNum;
			if(qRate < 1) qRate = 1;
			int i;
			for(i=0; i<lpNum; i++)
			{
				if(GNumFrames >= numFrm) break;
				Sleep(qRate);
			}

			if(i == lpNum)
			{
				result = -2;
				msgShown = true;
				strncpy(errMsg, "\n\nError: Timed out waiting for frames.",
								MAX_STR);
			}
		}
		else // user stops at any time
		{
			printf("\n\n**Hit any key to send vip_fluoro_record_stop()");
			_getch();
  			while(!_kbhit()) Sleep (100);
		}

		printf("\nSending  vip_fluoro_record_stop()");
		result = vip_fluoro_record_stop();
		// STOP #2
	}


	//////////////////////////////////////////////////////////////////////////
	if(result != VIP_NO_ERR)
	{
		if(!msgShown) printf("\nvip_fluoro_record_stop() returned %d", result);
		// We can always just call grabber_stop - we don't have to call 
		// record_stop first.
		GGrabbingIsActive = false;
		vip_fluoro_grabber_stop();
		msgShown = true;
	}
	else
	{
		// **** STEP 8 -- End grabbing --
		// We're back to the state where we are grabbing only and frames 
		// are not being copied to sequence buffers.
		fflush(stdin);
		printf("\n\n**Hit any key to send vip_fluoro_grabber_stop()");
		while(!_kbhit()) Sleep (100);

		GGrabbingIsActive = false;

		Sleep(100);
		if(GImgDisplayThreadIsAlive)
		{
			printf("\nPlease wait -- processing images.");
			while(GImgDisplayThreadIsAlive) Sleep(100);
		}

		printf("\nSending  vip_fluoro_grabber_stop()");
		result = vip_fluoro_grabber_stop();
	}

	//////////////////////////////////////////////////////////////////////////
	if(result != VIP_NO_ERR)
	{
		if(!msgShown) printf("\nvip_fluoro_grabber_stop() returned %d", result);
		msgShown = true;
	}
	else
	{
		printf("\n\n**Hit any key to send Save images()");
		_getch();
		while(!_kbhit()) Sleep (100);

		// **** STEP 9 -- Save images --
		FILE* fHandle=NULL;
		char fName[MAX_STR]="";
		printf("\nSaving images..");
		if(doMore) numFrm = numBuf;
		if(numFrm > 8) numFrm = 8; // limit the number we write out
		for(int i=0; i<numFrm; i++)
		{
			_snprintf(fName, MAX_STR, "%s\\image_%02d.raw", default_path, i);
		    fHandle = fopen(fName, "wb");
		
			if(fHandle)
			{
				WORD* buf=NULL;
				result = vip_fluoro_get_buffer_ptr(&buf, i);
				int ret = fwrite(buf, 1, GImgSize, fHandle);
				fclose(fHandle);
			}
			else
			{
				result = -2; 
				strncpy(errMsg, "\n\nError: Problem opening file", MAX_STR);
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// get stats for acquisition
	if(result != VIP_NO_ERR)
	{
		if(!msgShown) printf("\nProblem saving images ");
		msgShown = true;
	}
	else 
	{
		SSeqStats seqStats;
		memset(&seqStats, 0, sizeof(SSeqStats));
		result = vip_fluoro_get_prms(HCP_FLU_STATS_PRMS, &seqStats);
		if(result == VIP_NO_ERR)
		{
			printf("\nReturned Seq Stats: SmplFrms=%d; HookFrms=%d; "
					"CaptFrms=%d; HookOverrun=%d; StartIdx=%d; "
					"EndIdx=%d; CaptRate=%.3f",
					seqStats.SmplFrms, seqStats.HookFrms, seqStats.CaptFrms,
					seqStats.HookOverrun, seqStats.StartIdx, 
					seqStats.EndIdx, seqStats.CaptRate);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	if(result != VIP_NO_ERR)
	{
		if(!msgShown) printf("\nProblem getting stats ");
		msgShown = true;
		vip_close_link(); 
	}
	else 
	{
		// **** STEP 10 -- Close the link --
		// Resources allocated will be cleaned up.
		printf("\n\n**Hit any key to send vip_close_link()");
		_getch();
		while(!_kbhit()) Sleep (100);

		printf("\nSending  vip_close_link()");
		result = vip_close_link();
		vip_set_debug(FALSE);
	}

	if(result != VIP_NO_ERR) 
	{
		if(!msgShown) printf("\nvip_close_link() returned %d", result);
		msgShown = true;

		if(!strcmp(errMsg, ""))
		{
			GetCPError(result, errMsg);
		}
		printf("\n\n%s", errMsg);

		if(result == 81)
		{
			printf("\n\nCheck that the default_path exists");
		}
	}
	else
	{
		printf("\n\n\nAcquisition complete");
	}
	
	printf("\n**Hit any key to exit");
	_getch();
	while(!_kbhit()) Sleep (100);

	return result;
}

//////////////////////////////////////////////////////////////////////////////
// This is the thread we launch to handle realtime image processing/display.
DWORD WINAPI ImgDisplayThread(LPVOID lpParameter)
{
	int lastCnt=-1;
	const int timeoutval = 1000;
	HANDLE	hFrameEvent = CreateEvent(NULL, FALSE, FALSE, GFrameReadyName);

	// If Just-In-Time corrections are implemented and in use, ask for
	// the first frame here.

	while(GGrabbingIsActive && hFrameEvent) //&& GLiveParams)
	{
		if(WaitForSingleObject(hFrameEvent, timeoutval) != WAIT_TIMEOUT)
		{
			WORD* imPtr ;//= (WORD*)GLiveParams->BufPtr;
			GNumFrames = 0;//GLiveParams->NumFrames;
			// display the image or do other processing
			if(GNumFrames && GNumFrames != lastCnt)
			{
				printf("\n--Image available. Ptr=0x%08x; "
						"Number of frames captured=%d.",
						imPtr, GNumFrames);
				lastCnt = GNumFrames;
			}


		// If Just-In-Time corrections are implemented and in use, ask for
		// the next frame here.
		}
	}

	if(hFrameEvent) CloseHandle(hFrameEvent);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
// do a cal
int DoCal(int mode)
{
	int doGain=0;
	printf("\nDo you want to do a ANALOG OFFSET=2 or GAIN=1 or OFFSET=0 ?: ");
	scanf("%d",&doGain);
	if(doGain == 2)
	{
		return FluoroAnalogOffsetCal(mode);
	}
	else if(doGain == 1)
	{
		return FluoroGainCal(mode);
	}
	else
	{
		return FluoroOffsetCal(mode);
	}
}

//////////////////////////////////////////////////////////////////////////////
// do an analog offset cal
int  FluoroAnalogOffsetCal(int mode)
{
	SAnalogOffsetParams aop;
	memset (&aop, 0, sizeof(SAnalogOffsetParams));
	aop.StructSize = sizeof(SAnalogOffsetParams);

	int result = vip_get_analog_offset_params(mode, &aop);
	// or just set them..
	//aop.TargetValue=1000;
	//aop.Tolerance=50;
	//aop.MedianPercent=50;
	//aop.FracIterDelta=0.33;
	//aop.NumIterations=10;
	if(result == VIP_NO_ERR)
	{
		printf ("\n\nAnalog offset params are:-\nTargetValue=%d; Tolerance=%d;"
			"\nMedianPercent=%d; FracIterationDelta=%.3f; NumberIterations=%d",
			aop.TargetValue, aop.Tolerance, 
			aop.MedianPercent, aop.FracIterDelta, aop.NumIterations);

		// start the analog offset cal
		result = vip_analog_offset_cal(mode);
	}

	if(result == VIP_NO_ERR)
	{
		UQueryProgInfo uq;
		memset(&uq, 0, sizeof(UQueryProgInfo));
		uq.qpi.StructSize = sizeof(SQueryProgInfo);

		printf("\n\nAnalog Offset in Progress");

		while(!uq.qpi.Complete && result == VIP_NO_ERR)
		{
			Sleep(1000);
			printf(".");
			result = vip_query_prog_info(HCP_U_QPI, &uq);
		}
	}

	if(result == VIP_NO_ERR)
	{
		printf("\n\nAnalog offset calibration completed successfully");
	}
	else
	{
		printf("\n\nError in analog offset calibration");
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////
// do a gain cal
int  FluoroGainCal(int mode)
{
	int numCalFrmSet=0;
	int result = vip_get_num_cal_frames(mode, &numCalFrmSet);

	// tell the system to prepare for a gain calibration
	if(result == VIP_NO_ERR)
	{
		result = vip_gain_cal_prepare(mode);
	}

	// send prepare = true
	if(result == VIP_NO_ERR)
	{
		result = vip_sw_handshaking(VIP_SW_PREPARE, TRUE);
	}

	SQueryProgInfo qpi;
	memset(&qpi, 0, sizeof(SQueryProgInfo));
	qpi.StructSize = sizeof(SQueryProgInfo);
	UQueryProgInfo* uqpi = (UQueryProgInfo*)&qpi;

	// wait for readyForPulse
	while(!qpi.ReadyForPulse && result == VIP_NO_ERR)
	{
		result = vip_query_prog_info(HCP_U_QPI, uqpi);
		Sleep(100);
	}
	
	fflush(stdin);
	printf("\n**Hit any key when x-rays are ON and ready for flat-field");
	while(!_kbhit()) Sleep (100);

	// send xrays = true - this signals the START of the FLAT-FIELD ACQUISITION
	if(result == VIP_NO_ERR)
	{
		result = vip_sw_handshaking(VIP_SW_VALID_XRAYS, TRUE);
	}

	qpi.NumFrames = 0;
	int maxFrms=0;
	while(qpi.NumFrames < numCalFrmSet && result == VIP_NO_ERR)
	{
		result = vip_query_prog_info(HCP_U_QPI, uqpi);
		printf("\nFlat-field frames accumulated = %d", qpi.NumFrames);
		Sleep(100);

		// just in case the number of frames resets to zero before we see the 
		// limit reached
		if(maxFrms > qpi.NumFrames) break;
		maxFrms = qpi.NumFrames;
	}
	
	printf("\n**Hit any key when x-rays are OFF and ready for dark-field");
	_getch();
	while(!_kbhit()) Sleep (100);

	// send xrays = false - this signals the START of the DARK-FIELD ACQUISITION
	if(result == VIP_NO_ERR)
	{
		result = vip_sw_handshaking(VIP_SW_VALID_XRAYS, 0);
	}

	// wait for the calibration to complete
	qpi.Complete = FALSE;
	while (!qpi.Complete && result == VIP_NO_ERR)
	{
		result = vip_query_prog_info(HCP_U_QPI, uqpi);
		if(qpi.NumFrames < numCalFrmSet)
		{
			printf("\nDark-field frames accumulated = %d", qpi.NumFrames);
		}
		Sleep(100);
	}

	if(result == VIP_NO_ERR)
	{
		printf("\n\nGain calibration completed successfully");
	}
	else
	{
		printf("\n\nError in gain calibration");
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////
// do offset cal
int  FluoroOffsetCal(int mode)
{
	fflush(stdin);
	printf("\n**Hit any key when x-rays are OFF and ready for offset cal");
	while(!_kbhit()) Sleep (100);

	// find how many calibration frames we have set
	int numCalFrmSet=0;
	int result = vip_get_num_cal_frames(mode, &numCalFrmSet);

	if(result == VIP_NO_ERR)
	{
		// start the offset cal
		result = vip_offset_cal(mode);
	}

	SQueryProgInfo qpi;
	memset(&qpi, 0, sizeof(SQueryProgInfo));
	qpi.StructSize = sizeof(SQueryProgInfo);
	UQueryProgInfo* uqpi = (UQueryProgInfo*)&qpi;

	// wait for the calibration to complete
	while (!qpi.Complete && result == VIP_NO_ERR)
	{
		int lastNum = 0;
		result = vip_query_prog_info(HCP_U_QPI, uqpi);
		if(qpi.NumFrames <= numCalFrmSet)
		{
			if(lastNum != qpi.NumFrames)
			{
				printf("\nOffset frames accumulated = %d; Complete = %d",
						qpi.NumFrames, qpi.Complete);
				lastNum = qpi.NumFrames;
			}
		}
		Sleep(100);
	}

	if(result == VIP_NO_ERR)
	{
		printf("\n\nOffset calibration completed successfully");
	}
	else
	{
		printf("\n\nError in offset calibration");
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////
// Find error string from error code.
void GetCPError(int errCode, char* errMsg)
{
	static int hcpErrCnt=0;
	if(!hcpErrCnt)
	{   int i=0;
		for(i=0; i<MAX_HCP_ERROR_CNT; i++)
		{
			if(!strncmp(HcpErrStrList[i], "UUU", MIN_STR)) break;
		}
		if(i < MAX_HCP_ERROR_CNT)
		{
			hcpErrCnt = i;
		}
		if(hcpErrCnt <= 0)
		{
			if(errMsg) strncpy(errMsg, "Error at Compile Time", MAX_STR);
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
		if(errMsg) strncpy(errMsg, HcpErrStrList[errCode], MAX_STR);
	}
	else
	{
		if(errMsg) strncpy(errMsg, "Unknown Error", MAX_STR);
	}
}
//////////////////////////////////////////////////////////////////////////////

/*
int  CheckRecLink()
{
	SCheckLink clk;
	memset(&clk, 0, sizeof(SCheckLink));
	clk.StructSize = sizeof(SCheckLink);
	int result = vip_check_link(&clk);
	int i=0;
	while(result != HCP_NO_ERR && i++ < 5)
	{
		 Sleep(1000);
		 result = vip_check_link(&clk);
	}

	if(result != HCP_NO_ERR)
	{
		// If check link fails it could be because of several things in addition
		// to the link itself not working correctly. It may be that the receptor
		// hasn't settled to its normal range for example. Give the option to move on
		// here but in a real application we should understand why.
		int ignore=0;
		printf("\nCheck link failed. Do you want to attempt to continue "
				"YES=1 or NO=0 ?: ");     
		scanf("%d",&ignore);
		if(ignore == 1) result = HCP_NO_ERR;
	}

	return result;
}*/




