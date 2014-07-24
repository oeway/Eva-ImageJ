

//////////////////////////////////////////////////////////////////////////////
// This code is intended as a simple example of using the Virtual CP.
// 
// Copyright:	Varian Medical Systems
//				All Rights Reserved
//				Varian Proprietary   
//////////////////////////////////////////////////////////////////////////////


#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include <windows.h>

#include "HcpFuncDefs.h"
#include "HcpErrors.h"

#ifndef VIP_NO_ERR
#define VIP_NO_ERR 0
#endif

#ifndef MAX_STR
#define MAX_STR 256
#endif

#define IMAGERS_DIR_PATH "C:\\IMAGERs\\9104-06"
#define RECORD_TIMEOUT 100
#define MAX_HCP_ERROR_CNT 65536

#define ESC_KEY   (0x1B)
#define ENTER_KEY (0x0D)
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
static	SLivePrms*	GLiveParams=NULL;
static	long	GNumFrames=0;
static	long	GImgDisplayThreadIsAlive=0;	

UQueryProgInfo crntStatus;
UQueryProgInfo prevStatus;  // So we can tell when crntStatus changes



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
		printf ("\n\n模拟偏移参数:-\n目标值=%d; 容差=%d;"
			"\nMedianPercent=%d; FracIterationDelta=%.3f; 迭代次数=%d",
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

		printf("\n\n模拟偏移校正进行中...");

		while(!uq.qpi.Complete && result == VIP_NO_ERR)
		{
			Sleep(1000);
			printf(".");
			result = vip_query_prog_info(HCP_U_QPI, &uq);
		}
	}

	if(result == VIP_NO_ERR)
	{
		printf("\n\n模拟偏移校正成功!");
	}
	else
	{
		printf("\n\n模拟偏移校正出错!!!");
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
	printf("\n**请打开射线源，按任意键继续本底扫描.");
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
		printf("\n本底帧数 = %d", qpi.NumFrames);
		Sleep(100);

		// just in case the number of frames resets to zero before we see the 
		// limit reached
		if(maxFrms > qpi.NumFrames) break;
		maxFrms = qpi.NumFrames;
	}
	
	printf("\n**请关闭射线源，按任意键继续暗场采集.");
	_getch();
	while(!_kbhit()) Sleep (100);

	// wait for readyForPulse
	while(!qpi.ReadyForPulse && result == VIP_NO_ERR)
	{
		result = vip_query_prog_info(HCP_U_QPI, uqpi);
		Sleep(100);
	}

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
			printf("\n暗场帧数 = %d", qpi.NumFrames);
		}
		Sleep(100);
	}

	if(result == VIP_NO_ERR)
	{
		printf("\n\n增益校正成功！");
	}
	else
	{
		printf("\n\n增益校正出错！！！");
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////
// do offset cal
int  FluoroOffsetCal(int mode)
{
	fflush(stdin);
	printf("\n**请关闭射线源，按任意键开始暗场采集.");
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
				printf("\n偏移校正帧数 = %d; 完成 = %d",
						qpi.NumFrames, qpi.Complete);
				lastNum = qpi.NumFrames;
			}
		}
		Sleep(100);
	}

	if(result == VIP_NO_ERR)
	{
		printf("\n\n偏移校正成功!");
	}
	else
	{
		printf("\n\n偏移校正出错！！！");
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////
// Find error string from error code.
void GetCPError(int errCode, char* errMsg)
{
	static int hcpErrCnt=0;
	if(!hcpErrCnt)
	{
		int i=0;
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
		if(errMsg) strncpy(errMsg, "位置错误", MAX_STR);
	}
}
//////////////////////////////////////////////////////////////////////////////


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
		printf("\n连接失败，是否继续  "
				"YES=1 or NO=0 ?: ");     
		scanf("%d",&ignore);
		if(ignore == 1) result = HCP_NO_ERR;
	}

	return result;
}


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
			printf("帧数=%d 完成=%d 脉冲=%d 准备=%d\n",
				crntStatus.qpi.NumFrames,
				crntStatus.qpi.Complete,
				crntStatus.qpi.NumPulses,
				crntStatus.qpi.ReadyForPulse);

			prevStatus.qpi = crntStatus.qpi;
		}
	}
	else
		printf("**** vip_query_prog_info返回错误： %d\n", result);

	return result;
}


int  RadOffsetCalibration(int crntModeSelect)
{
	int modeNum = crntModeSelect;

	int result = vip_reset_state();

	printf("开始RAD模式偏移校正\n");
	result = vip_offset_cal(modeNum);
	if (result != HCP_NO_ERR)
	{
		printf("*** 出错： %d\n", result);
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

	printf("偏移校正完成！\n");

	return 0;
}

//----------------------------------------------------------------------
//
//  performSwGainCalibration
//
//----------------------------------------------------------------------

int  RadGainCalibration(int crntModeSelect)
{
	int  numGainCalImages = 4;
	int  keyCode;

	printf("开始RAD模式增益校正\n");

	int  result = vip_reset_state();

// NOTE: for simplicity some error checking left out in the following
	int numOfstCal=2;
	vip_get_num_cal_frames(crntModeSelect, &numOfstCal);

	result = vip_gain_cal_prepare(crntModeSelect, false);
	if (result != HCP_NO_ERR)
	{
		printf("*** vip_gain_cal_prepare 返回错误： %d\n", result);
		return result;
	}
	result = vip_sw_handshaking(VIP_SW_PREPARE, 1);
	if (result != HCP_NO_ERR)
	{
		printf("*** vip_sw_handshaking(VIP_SW_PREPARE, 1) 返回错误 %d\n", result);
		return result;
	}
	printf("\n**请关闭射线源，按任意键开始暗场采集.");
	printf("正在进行暗场采集...\n");
	printf("按Esc可以中止采集并保留上次的校正结果\n");
	crntStatus.qpi.ReadyForPulse = FALSE;
	do
	{
		result = queryProgress();
		if (result != HCP_NO_ERR)
			return result;

		SleepEx(50, FALSE);
		if (_kbhit())
		{
			printf("用户取消，正在退出采集...\n");
			vip_reset_state();
			return -1;
		}
	} while (crntStatus.qpi.NumFrames < numOfstCal);
	printf("暗场采集完成！\n\n");

	int numPulses=0;
	
	vip_enable_sw_handshaking(TRUE);

	printf("\n请打开射线源，按任意键开始采集本底图像.\n");	
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

		printf("准备曝光...\n");

		crntStatus.qpi.NumPulses = numPulses;
		while(crntStatus.qpi.NumPulses == numPulses)
		{
			result = queryProgress();
			if(result != HCP_NO_ERR) return result;
			if(crntStatus.qpi.NumPulses != numPulses) break;
			SleepEx(100, FALSE);
		}
		numPulses = crntStatus.qpi.NumPulses;

		printf("计数 =%d", numPulses);
		if(numPulses == numGainCalImages) break;
	}
	
	printf("正在结束...\n");
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
				printf("按Esc强制结束校正过程！\n");
				vip_reset_state();
				return -1;
			}
		}
	}

	printf("增益校正成功！\n");

	// no need to do this if not using Hw handshaking at all
	vip_enable_sw_handshaking(FALSE);

	return 0;
}


//////////////////////////////////////////////////////////////////////////////
int main()
{
	char errMsg[MAX_STR]="";
	int numBuf=0, numFrm=0;
	bool msgShown=false;
	int acqType=-1;
	int input;
	int mode =0;
	START:
	printf("\n---------EVA校正程序----------");
	SOpenReceptorLink orl;
	memset(&orl, 0, sizeof(SOpenReceptorLink));
	orl.StructSize = sizeof(SOpenReceptorLink);
	strncpy(orl.RecDirPath, IMAGERS_DIR_PATH, MAX_STR);
	printf("\n正在打开连接...");
	int result = vip_open_receptor_link(&orl);
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
		if(result == VIP_NO_ERR) printf("\n\n校正关闭!!");
	}

	printf("\n请选择:");
	printf("\n1 RAD模式校正");
	printf("\n2 High-Res Fluoro模式校正");
	printf("\n3 Low-Res Fluoro模式校正");
	printf("\n0 退出");
	printf("\n");
	scanf("%d",&input);

	switch(input)
	{
	case 1:
		mode =0;
		printf("\n1 偏移校正 2 增益校正，请选择：");
		scanf("%d",&input);
		if(input ==1)
		    RadOffsetCalibration(mode);
		else if(input ==2)
			RadGainCalibration(mode);
		break;
	case 2:
		mode =0;
		printf("\n1 偏移校正 2 增益校正 3 模拟校正，请选择：");
		scanf("%d",&input);
		if(input ==1)
		    FluoroOffsetCal(mode);
		else if(input ==2)
			FluoroGainCal(mode);
		else if(input ==3)
			FluoroAnalogOffsetCal(mode);
		break;
	case 3:
		mode =1;
		printf("\n1 偏移校正 2 增益校正 3 模拟校正，请选择：");
		scanf("%d",&input);

		if(input ==1)
		    FluoroOffsetCal(mode);
		else if(input ==2)
			FluoroGainCal(mode);
		else if(input ==3)
			FluoroAnalogOffsetCal(mode);
		break;
	case 0:
		vip_close_link();
		return 0;

	}
	goto START;
	return 0;
}