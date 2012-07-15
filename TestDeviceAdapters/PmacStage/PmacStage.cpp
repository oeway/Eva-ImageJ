#include "PmacStage.h"
#include <atlstr.h>
#include <stdio.h>
#include <math.h>
#include ".\\PComm32\\DataDefine.h"
#include ".\\PComm32\\PCommAxis.h"
#include ".\\PComm32\\MCAxisBase.h"
#include ".\\PComm32\\PMACRuntime.h"
using namespace std;
#include <Windows.h>
#include <cstdarg>
#define BUFF_SIZE 4096
const char* g_Stage_name = "PmacXYStage";
static const char* g_Keyword_SetPosXmm = "PositionX";
static const char* g_Keyword_SetPosYmm = "PositionY";
static const char* g_Keyword_SetOriginHere = "Set origin here";
static const char* g_Keyword_Calibrate = "Calibrate";
static const char* g_Keyword_ReturnToOrigin = "Return to origin";
static const char* g_Keyword_PositionTypeAbsRel = "PositionType";
static const char* g_Keyword_SetPosXYmm = "Set position XY axis (mm) (X= Y=)";

static const char* g_Listword_No = "No";
static const char* g_Listword_Yes = "Yes";
static const char* g_Listword_AbsPos = "Absolute Position";
static const char* g_Listword_RelPos = "Relative Position";
double g_IntensityFactor_ = 1.0;


///////////////////////////////////////////////////////////////////////////////
// Exported MMDevice API
///////////////////////////////////////////////////////////////////////////////

/**
 * List all supported hardware devices here
 */
MODULE_API void InitializeModuleData()
{
   AddAvailableDeviceName(g_Stage_name, "Pmac Stage by OEway");
}
MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
   if (deviceName == 0)
      return 0;

   // decide which device class to create based on the deviceName parameter
   if (strcmp(deviceName, g_Stage_name) == 0)
   {
      // create camera
      return new PmacXYStage();
   }

   // ...supplied name not recognized
   return 0;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
   delete pDevice;
}
///////////////////////////////////////////////////////////////////////////////
// PmacXYStage implementation
// ~~~~~~~~~~~~~~~~~~~~~~~~~
const char* g_XYStageDeviceName = "PmacXYStage";

///////////////////////////////////////////////////////////////////////////////
// PmacXYStage implementation
// ~~~~~~~~~~~~~~~~~~~~~~~~~

PmacXYStage::PmacXYStage() : 
CXYStageBase<PmacXYStage>(),
	busy_(false),
	timeOutTimer_(0),
	initialized_(false),
	lowerLimit_(0.0),
	upperLimit_(20000.0)
{
   InitializeDefaultErrorMessages();
   //TODO:error message
   //定义错误代码
//#define ERRO_NOERR				0x0000
//#define ERRO_INIT_LINK			0x0001
//#define ERRO_INIT_OPEN			0x0002
//#define ERRO_RELEASE_CLOSE		0x0011
//#define ERRO_RELEASE_DLINK		0x0012
//#define ERRO_NORMAL_CMD			0x0100
   	// MCL error messages 
	////SetErrorText(MCL_GENERAL_ERROR, "MCL Error: General Error");
	////SetErrorText(MCL_DEV_ERROR, "MCL Error: Error transferring data to device");
	////SetErrorText(MCL_DEV_NOT_ATTACHED, "MCL Error: Device not attached");
	////SetErrorText(MCL_USAGE_ERROR, "MCL Error: Using a function from library device does not support");
	////SetErrorText(MCL_DEV_NOT_READY, "MCL Error: Device not ready");
	////SetErrorText(MCL_ARGUMENT_ERROR, "MCL Error: Argument out of range");
	////SetErrorText(MCL_INVALID_AXIS, "MCL Error: Invalid axis");
	////SetErrorText(MCL_INVALID_HANDLE, "MCL Error: Handle not valid");
	////SetErrorText(MCL_INVALID_DRIVER, "MCL Error: Invalid Driver");

}

PmacXYStage::~PmacXYStage()
{
	Shutdown();
}

void PmacXYStage::GetName(char* Name) const
{
	CDeviceUtils::CopyLimitedString(Name, g_XYStageDeviceName);
}

int PmacXYStage::Initialize()
{	
	if (initialized_)
		return DEVICE_OK;
	// set property list
	// -----------------
	stepSize_mm_ =0.001;

	int ret = CreatePmacXYProperties();
	if (ret != DEVICE_OK)
		return ret;
	initialized_ = true;
	ret = Init_Pmac();
	if (ret != DEVICE_OK)
		initialized_ = false;

	ret = SetProperty("PmacLink","OnLine");//open link
	if(ret != DEVICE_OK)
		initialized_ = false;
	maxVelocityX_ = m_PCommDev.m_Axis[xAxisID_].m_AxsInf.m_fSpdMax;
	maxVelocityY_ = m_PCommDev.m_Axis[yAxisID_].m_AxsInf.m_fSpdMax;

	ret = SetPropertyLimits(g_Keyword_SetPosXmm, positionMin_[xAxisID_], positionMax_[xAxisID_]);
	assert(ret == DEVICE_OK);

	ret = SetPropertyLimits(g_Keyword_SetPosYmm, positionMin_[yAxisID_], positionMax_[yAxisID_]);
	assert(ret == DEVICE_OK);

	ret = SetPropertyLimits("VelocityX", 0.001 ,maxVelocityX_);
	assert(ret == DEVICE_OK);

	ret = SetPropertyLimits("VelocityY", 0.001 ,maxVelocityY_);
	assert(ret == DEVICE_OK);
	GetCoreCallback()->SetDeviceProperty("Core","TimeoutMs","100000");
	ret = UpdateStatus();
	if (ret != DEVICE_OK)
		return ret;
	return ret;
}

int PmacXYStage::CreatePmacXYProperties()
{
	int err;
	char iToChar[255];

	vector<string> yesNoList;
    yesNoList.push_back(g_Listword_No);
    yesNoList.push_back(g_Listword_Yes);

	/// Read only properties
	
	CPropertyAction *pAct;
	// Name
	int ret = CreateProperty(MM::g_Keyword_Name, g_XYStageDeviceName, MM::String, true);
	if (DEVICE_OK != ret)
		return ret;

	// Description
	ret = CreateProperty(MM::g_Keyword_Description, "PMAC stage driver--By OEway", MM::String, true);
	if (DEVICE_OK != ret)
		return ret;

   //PmacLink
   const char* const v_Keyword_PmacLink    = "PmacLink";
   pAct = new CPropertyAction (this, &PmacXYStage::OnPmacLink);
   ret = CreateProperty(v_Keyword_PmacLink, "OffLine", MM::String, false,pAct);
   assert(ret == DEVICE_OK);
   vector<string> PmacLinkValues;
   PmacLinkValues.push_back("OffLine");
   PmacLinkValues.push_back("OnLine"); 
   ret = SetAllowedValues(v_Keyword_PmacLink, PmacLinkValues);
   assert(ret == DEVICE_OK);

	//DeviceID
	const char* const v_Keyword_DeviceID    = "DeviceID";
	pAct = new CPropertyAction (this, &PmacXYStage::OnDeviceID);
	ret = CreateProperty(v_Keyword_DeviceID, "0", MM::Integer, false,pAct);
	assert(ret == DEVICE_OK);
	ret = SetPropertyLimits(v_Keyword_DeviceID, 0,10);
	assert(ret == DEVICE_OK);


	//xAxisID
	const char* const v_Keyword_xAxisID    = "xAxisID";
	pAct = new CPropertyAction (this, &PmacXYStage::OnxAxisID);
	ret = CreateProperty(v_Keyword_xAxisID, "0", MM::Integer, false,pAct);
	assert(ret == DEVICE_OK);
	ret = SetPropertyLimits(v_Keyword_xAxisID, 0,10);
	assert(ret == DEVICE_OK);
	xAxisID_= 0;

	//yAxisID
	const char* const v_Keyword_yAxisID    = "yAxisID";
	pAct = new CPropertyAction (this, &PmacXYStage::OnyAxisID);
	ret = CreateProperty(v_Keyword_yAxisID, "0", MM::Integer, false,pAct);
	assert(ret == DEVICE_OK);
	ret = SetPropertyLimits(v_Keyword_yAxisID, 0,10);
	assert(ret == DEVICE_OK);
	yAxisID_ = 2;

	maxVelocityX_ =1.0;
	// Maximum velocity
	sprintf(iToChar, "%f", maxVelocityX_);
	err = CreateProperty("MaxVelocityX", iToChar, MM::Float, true);
	if (err != DEVICE_OK)
		return err;

	maxVelocityY_ =1.0;
	// Minumum velocity
	sprintf(iToChar, "%f", maxVelocityY_);
	err = CreateProperty("MaxVelocityY", iToChar, MM::Float, true);
	if (err != DEVICE_OK)
		return err;

	/// Action properties

	velocityX_ = 0.50;
	// Change velocityX
	sprintf(iToChar, "%f", maxVelocityX_);
	pAct = new CPropertyAction(this, &PmacXYStage::OnVelocityX);
	err = CreateProperty("VelocityX", iToChar, MM::Float, false, pAct);
	if (err != DEVICE_OK)
		return err;


	velocityY_ = 0.50;
	// Change velocity
	sprintf(iToChar, "%f", maxVelocityY_);
	pAct = new CPropertyAction(this, &PmacXYStage::OnVelocityY);
	err = CreateProperty("VelocityY", iToChar, MM::Float, false, pAct);
	if (err != DEVICE_OK)
		return err;

	// Change X position (mm)
	pAct = new CPropertyAction(this, &PmacXYStage::OnPositionXmm);
	err = CreateProperty(g_Keyword_SetPosXmm, "0", MM::Float, false, pAct);

	//SetPropertyLimits(g_Keyword_SetPosXmm, 0.0, 100.0);

	if (err != DEVICE_OK)
		return err;

	// Change Y position (mm)
	pAct = new CPropertyAction(this, &PmacXYStage::OnPositionYmm);
	err = CreateProperty(g_Keyword_SetPosYmm, "0", MM::Float, false, pAct);
	//SetPropertyLimits(g_Keyword_SetPosYmm, 0.0, 100.0);
	if (err != DEVICE_OK)
		return err;

	// Set origin at current position (reset encoders)
	pAct = new CPropertyAction(this, &PmacXYStage::OnSetOriginHere);
	err = CreateProperty(g_Keyword_SetOriginHere, "No", MM::String, false, pAct);
	if (err != DEVICE_OK)
		return err;
    err = SetAllowedValues(g_Keyword_SetOriginHere, yesNoList);
	if (err != DEVICE_OK)
		return err;

	// Calibrate
	pAct = new CPropertyAction(this, &PmacXYStage::OnCalibrate);
	err = CreateProperty(g_Keyword_Calibrate, "No", MM::String, false, pAct);
	if (err != DEVICE_OK)
		return err;
	err = SetAllowedValues(g_Keyword_Calibrate, yesNoList);
	if (err != DEVICE_OK)
		return err;

	// Return to origin
	pAct = new CPropertyAction(this, &PmacXYStage::OnReturnToOrigin);
	err = CreateProperty(g_Keyword_ReturnToOrigin, "No", MM::String, false, pAct);
	if (err != DEVICE_OK)
		return err;
	err = SetAllowedValues(g_Keyword_ReturnToOrigin, yesNoList);
	if (err != DEVICE_OK)
		return err;

	// Change x&y position at same time (mm)
	pAct = new CPropertyAction(this, &PmacXYStage::OnPositionXYmm);
	err = CreateProperty(g_Keyword_SetPosXYmm, "X=0.00 Y=0.00", MM::String, false, pAct);
	if (err != DEVICE_OK)
		return err;

	return DEVICE_OK;
}
int PmacXYStage::SetPositionUm(double x, double y)
{
	return SetPositionMm(x / 1000.0, y / 1000.0);
}

int PmacXYStage::GetPositionUm(double& x, double& y)
{
	if(!initialized_)
		return DEVICE_OK;
	int err = GetPositionMm(x, y);
	x *= 1000.0;
	y *= 1000.0;
	x=floor(x);//TODO:round
	y=floor(y);
	return err;
}
int PmacXYStage::OnAxsSetPara( int nAxsID, int nCmdTyp, int nPlsTyp,double value)
{
	CString csTmp;
	double dfPstA = 0, dfPlsA = 0;
	
	//((CEdit *)GetDlgItem(nPstDlgID))->GetWindowText(csTmp);
	dfPstA = value;//atof(csTmp);
	
	if (
		0 > nAxsID
		|| 8 < nAxsID
		) 
	{
		//AfxMessageBox("轴选择错误");
		return DEVICE_ERR;
	}

	//轴ID为数组下标加1
	m_PCommDev.m_Axis[nAxsID].m_AxsCtr.m_nIsSel = AXS_ISSEL_YES;
	m_PCommDev.m_Axis[nAxsID].m_AxsCtr.m_nCmdTyp = nCmdTyp;
	m_PCommDev.m_Axis[nAxsID].m_AxsCtr.m_nPlsTyp = nPlsTyp;
	
	CMCAxsGetPls(&(m_PCommDev.m_Axis[nAxsID].m_AxsInf), dfPstA,
		&(m_PCommDev.m_Axis[nAxsID].m_AxsDat.m_dfPlsAsk));	
	return DEVICE_OK;
}
int PmacXYStage::SetPositionMm(double x, double y)
{
	int err;

	///Calculate the absolute position.
	double xCurrent, yCurrent;

	err = GetPositionMm(xCurrent,yCurrent);
	if (err != DEVICE_OK)
	return err;

	double dx = x-xCurrent;
	double dy = y -yCurrent;

	err = SetRelativePositionMm(dx,dy);
	if (err != DEVICE_OK)
	return err;
	//PauseDevice();
	//if(err ==ERRO_NOERR)
		return DEVICE_OK;
	//else
	//	return DEVICE_ERR;
}

int PmacXYStage::SetRelativePositionUm(double dx, double dy){

	int err;
	err = SetRelativePositionMm(dx/1000, dy/1000);

	return err;
}

int PmacXYStage::SetRelativePositionMm(double x, double y){

	int err;
	//bool mirrorX, mirrorY;
	double lastX,lastY;
	bool openloop=false;
	//err = getAxisInfo(InfoType_status);
	//if (err != DEVICE_OK)
	//return err;
	//bool openloop =  (m_PCommDev.m_Axis[xAxisID_].m_AxsStt.m_nOpenLoop == 1) || (m_PCommDev.m_Axis[yAxisID_].m_AxsStt.m_nOpenLoop == 1);
	//if(openloop) //emergency button down!
	//	return 77;
	err = GetPositionMm(lastX,lastY);
	if (err != DEVICE_OK)
		return err;
  //  GetOrientation(mirrorX, mirrorY);
	
  //  if (mirrorX)
		//x = -x;
  //  if (mirrorY)
  //      y = -y;
	bool xarrived=false,yarrived=false;

	bool noXMovement = (fabs(x) < stepSize_mm_); 
	bool noYMovement = (fabs(y) < stepSize_mm_);

	if (noXMovement && noYMovement)
	{
		///No movement	
	}
	else if (noXMovement || XMoveBlocked(x))
	{ 
	OnAxsSetPara(yAxisID_, AXS_CMDTYPE_JOG_PST, PST_PLS_TYPE_REL,y);
	err = MCDevCmdSend(&m_PCommDev, m_chRes, m_nChSndLen, m_chSnd);

	}
	else if (noYMovement || YMoveBlocked(y))
	{
	OnAxsSetPara(xAxisID_, AXS_CMDTYPE_JOG_PST, PST_PLS_TYPE_REL,x);
	err = MCDevCmdSend(&m_PCommDev, m_chRes, m_nChSndLen, m_chSnd);
	}
	else 
	{
	OnAxsSetPara(xAxisID_, AXS_CMDTYPE_JOG_PST, PST_PLS_TYPE_REL,x);
	//err = MCDevCmdSend(&m_PCommDev, m_chRes, m_nChSndLen, m_chSnd);
	//Sleep(10);
	OnAxsSetPara(yAxisID_, AXS_CMDTYPE_JOG_PST, PST_PLS_TYPE_REL,y);
	err = MCDevCmdSend(&m_PCommDev, m_chRes, m_nChSndLen, m_chSnd);
	}
	//		///Calculate the absolute position.
	//double xCurrent, yCurrent;
	//MM::MMTime startTime = GetCurrentMMTime();
	//int runCount=0;
	//while((!xarrived || !yarrived))// && (|| !YMoveBlocked(x))) //&& ( GetCurrentMMTime() - startTime < MM::MMTime( 60000 ) ) 
	//{//TODO:


	//	//err = GetPositionMm(xCurrent,yCurrent);
	//	getAxisInfo(InfoType_status);
	//	//if (err != DEVICE_OK)
	//	//return err;
	//	xarrived = noXMovement ||  (m_PCommDev.m_Axis[xAxisID_].m_AxsStt.m_nInPst == 1);//(fabs(lastX+x-xCurrent) < stepSize_mm_); 
	//	yarrived = noYMovement || (m_PCommDev.m_Axis[yAxisID_].m_AxsStt.m_nInPst == 1);//(fabs(lastY+y-yCurrent) < stepSize_mm_);

	//	openloop =  (m_PCommDev.m_Axis[xAxisID_].m_AxsStt.m_nOpenLoop == 1) || (m_PCommDev.m_Axis[yAxisID_].m_AxsStt.m_nOpenLoop == 1);
	//	if(openloop)//emergency
	//		break;
	//	if(XMoveBlocked(x) )
	//		xarrived = true;
	//	if(YMoveBlocked(y) )
	//		yarrived = true;

	//	if(runCount++>250)
	//		break;
	//	Sleep(300);
	//}

	//Sleep(100);//wait to complete the stepsize

	//UpdateProperty(g_Keyword_SetPosXmm);
	//UpdateProperty(g_Keyword_SetPosYmm);
	//UpdateProperty(g_Keyword_SetPosXYmm);
	//PauseDevice();
	return DEVICE_OK;
}

int PmacXYStage::GetPositionMm(double& x, double& y)
{
	static double lastPosx,lastPosy;
	getAxisInfo(InfoType_velocity);
	if(getAxisInfo(InfoType_position) != DEVICE_OK)
	{
		x= lastPosx;
		y= lastPosy;
		return DEVICE_ERR;
	}
	CMCAxsGetPst(&m_PCommDev.m_Axis[xAxisID_].m_AxsInf, 
	m_PCommDev.m_Axis[xAxisID_].m_AxsDat.m_dfPlsCur, &x);

	CMCAxsGetPst(&m_PCommDev.m_Axis[yAxisID_].m_AxsInf, 
	m_PCommDev.m_Axis[yAxisID_].m_AxsDat.m_dfPlsCur, &y);
	lastPosx=x;
	lastPosy=y;
	return DEVICE_OK;
}

double PmacXYStage::GetStepSize()
{
	return stepSize_mm_;
}

int PmacXYStage::SetPositionSteps(long x, long y)
{
	return SetPositionMm(x * stepSize_mm_, y * stepSize_mm_);
}

int PmacXYStage::GetPositionSteps(long& x, long& y)
{
	int err;
	double getX, getY;

	err = GetPositionMm(getX, getY);

	x = (long) (getX / stepSize_mm_);
	y = (long) (getY / stepSize_mm_);

	return err;
}


int PmacXYStage::Home()
{
	return MoveToForwardLimits();
}

void PmacXYStage::PauseDevice()
{
	//int milliseconds;

	//milliseconds = MCL_MicroDriveWait(MCLhandle_);

	//MCL_DeviceAttached(milliseconds + 1, MCLhandle_);

}

int PmacXYStage::Stop()
{
	//int err = MCL_MicroDriveStop(NULL, MCLhandle_);
	//if (err != MCL_SUCCESS)
	//	return err;

	m_PCommDev.m_Axis[xAxisID_].m_AxsCtr.m_nIsSel = AXS_ISSEL_YES;
	m_PCommDev.m_Axis[xAxisID_].m_AxsCtr.m_nCmdTyp = AXS_CMDTYPE_JOG_STP;

	m_PCommDev.m_Axis[yAxisID_].m_AxsCtr.m_nIsSel = AXS_ISSEL_YES;
	m_PCommDev.m_Axis[yAxisID_].m_AxsCtr.m_nCmdTyp = AXS_CMDTYPE_JOG_STP;

	MCDevCmdSend(&m_PCommDev, m_chRes, m_nChSndLen, m_chSnd);
	return DEVICE_OK;
}

int PmacXYStage::SetOrigin()
{
	//int err = MCL_MicroDriveResetEncoders(NULL, MCLhandle_);
	//if (err != MCL_SUCCESS)
	//	return err;

	return DEVICE_UNSUPPORTED_COMMAND;
}

int PmacXYStage::GetLimits(double& /*lower*/, double& /*upper*/)
{
	return DEVICE_UNSUPPORTED_COMMAND;
}

int PmacXYStage::GetLimitsUm(double& /*xMin*/, double& /*xMax*/, double& /*yMin*/, double& /*yMax*/)
{
	return DEVICE_UNSUPPORTED_COMMAND;
}

int PmacXYStage::GetStepLimits(long& /*xMin*/, long& /*xMax*/, long& /*yMin*/, long& /*yMax*/)
{
	return DEVICE_UNSUPPORTED_COMMAND;
}

double PmacXYStage::GetStepSizeXUm()
{
	return stepSize_mm_*1000.0;
}

double PmacXYStage::GetStepSizeYUm()
{
	return stepSize_mm_*1000.0;
}

int PmacXYStage::Calibrate()
{
	//int err;
	//double xPosOrig;
	//double yPosOrig;
	//double xPosLimit;
	//double yPosLimit;

	//err = MCL_MicroDriveReadEncoders(&xPosOrig, &yPosOrig, MCLhandle_);
	//if (err != MCL_SUCCESS)
	//	return err;

	//err = MoveToForwardLimits();
	//if (err != DEVICE_OK)
	//	return err;

	//err = MCL_MicroDriveReadEncoders(&xPosLimit, &yPosLimit, MCLhandle_);
	//if (err != MCL_SUCCESS)
	//	return err;

	//err = SetOrigin();
	//if (err != DEVICE_OK)
	//	return err;

	//err = SetPositionMm((xPosOrig - xPosLimit), (yPosOrig - yPosLimit));
	//if (err != DEVICE_OK)
	//	return err;

	return DEVICE_UNSUPPORTED_COMMAND;
}

bool PmacXYStage::XMoveBlocked(double possNewPos)
{
	if(getAxisInfo(InfoType_status))
	{
		if (m_PCommDev.m_Axis[xAxisID_].m_AxsStt.m_nNgLmted && possNewPos < 0)
			return true;
		else if (m_PCommDev.m_Axis[xAxisID_].m_AxsStt.m_nPsLmted && possNewPos > 0)
			return true;
	}
	return false;
}

bool PmacXYStage::YMoveBlocked(double possNewPos)
{
	if(getAxisInfo(InfoType_status))
	{
		if (m_PCommDev.m_Axis[yAxisID_].m_AxsStt.m_nNgLmted && possNewPos < 0)
			return true;
		else if (m_PCommDev.m_Axis[yAxisID_].m_AxsStt.m_nPsLmted && possNewPos > 0)
			return true;
	}
	return false;
}

void PmacXYStage::GetOrientation(bool& mirrorX, bool& mirrorY) 
{
	char val[MM::MaxStrLength];
	int ret = this->GetProperty(MM::g_Keyword_Transpose_MirrorX, val);
	assert(ret == DEVICE_OK);
	mirrorX = strcmp(val, "1") == 0 ? true : false;

	ret = this->GetProperty(MM::g_Keyword_Transpose_MirrorY, val);
	assert(ret == DEVICE_OK);
	mirrorY = strcmp(val, "1") == 0 ? true : false;
}

int PmacXYStage::MoveToForwardLimits()
{
	//int err;
	unsigned char status = 0;

	//err = MCL_MicroDriveStatus(&status, MCLhandle_);
	//if(err != MCL_SUCCESS)	
	//	return err;

	//while ((status & BOTH_FORWARD_LIMITS) != 0)
	//{ 
	//	err = SetRelativePositionUm(4000, 4000);

	//	if (err != DEVICE_OK)
	//		return err;

	//	err = MCL_MicroDriveStatus(&status, MCLhandle_);
	//	if (err != MCL_SUCCESS)	
	//		return err;
	//}

	return DEVICE_UNSUPPORTED_COMMAND;
}

int PmacXYStage::ReturnToOrigin()
{
	int err;
	//double xPos;
	//double yPos;

	err = SetPositionMm(0.0, 0.0);
	if (err != DEVICE_OK)
		return err;

	return DEVICE_OK;
}
int PmacXYStage::OnxAxisID( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		long nm;
		pProp->Get(nm);
		xAxisID_ = (int)nm;
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set((long)xAxisID_);
	}
	return DEVICE_OK;
}
int PmacXYStage::OnyAxisID( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		long nm;
		pProp->Get(nm);
		yAxisID_ = (int)nm;
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set((long)yAxisID_);
	}
	return DEVICE_OK;
}
int PmacXYStage::OnPositionXmm(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	int err;
	double pos;
	if (eAct == MM::BeforeGet)
	{
		double x, y;
		GetPositionMm(x, y);
		pProp->Set(x);
	}
	else if (eAct == MM::AfterSet)
	{
		pProp->Get(pos);
		///Calculate the absolute position.
		double xCurrent, yCurrent;

		err = GetPositionMm(xCurrent,yCurrent);
		if (err != DEVICE_OK)
		return err;

		double dx = pos-xCurrent;
		double dy = 0;

		err = SetRelativePositionMm(dx,dy);
		if (err != DEVICE_OK)
		return err;

		//if (err != ERRO_NOERR)
		//	return err;
	}

	return DEVICE_OK;
}

int PmacXYStage::OnPositionYmm(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	int err;
	double pos;
	
	if (eAct == MM::BeforeGet)
	{
		double x, y;
		GetPositionMm(x, y);
		pProp->Set(y);

	}
	else if (eAct == MM::AfterSet)
	{
		pProp->Get(pos);
		///Calculate the absolute position.
		double xCurrent, yCurrent;

		err = GetPositionMm(xCurrent,yCurrent);
		if (err != DEVICE_OK)
		return err;

		double dx = 0;
		double dy = pos-yCurrent;

		err = SetRelativePositionMm(dx,dy);
		if (err != DEVICE_OK)
		return err;

		//if (err != ERRO_NOERR)
		//	return err;
	}

	return DEVICE_OK;
}

int PmacXYStage::OnSetOriginHere(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	int err;
	string message;

	if (eAct == MM::BeforeGet)
	{
		pProp->Set(g_Listword_No);
	}
	else if (eAct == MM::AfterSet)
	{
		pProp->Get(message);

		if (message.compare(g_Listword_Yes) == 0)
		{
			err = SetOrigin();
			if (err != DEVICE_OK)
			{
				return err;
			}
		}
	} 

	return DEVICE_OK;
}

int PmacXYStage::OnCalibrate(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	int err;
	string message;

	if (eAct == MM::BeforeGet)
	{
		pProp->Set(g_Listword_No);
	}
	else if (eAct == MM::AfterSet)
	{
		pProp->Get(message);

		if (message.compare(g_Listword_Yes) == 0)
		{
			err = Calibrate();
			if (err != DEVICE_OK)
				return err;
		}
	}

	return DEVICE_OK;
}

int PmacXYStage::OnReturnToOrigin(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	int err;
	string message;

	if (eAct == MM::BeforeGet)
	{
		pProp->Set(g_Listword_No);
	}
	else if (eAct == MM::AfterSet)
	{
		pProp->Get(message);

		if (message.compare(g_Listword_Yes) == 0)
		{
			err = ReturnToOrigin();
			if (err != DEVICE_OK)
				return err;
		}
	}

	return DEVICE_OK;
}

int PmacXYStage::OnPositionXYmm(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	int err;
	double x, y;
	string input;
	vector<string> tokenInput;
	char* pEnd;
	if (eAct == MM::BeforeGet)
	{ 
		double x, y;
		GetPositionMm(x, y);
		char iToChar[255];
		sprintf(iToChar,"X= %f Y= %f", x,y);
		pProp->Set(iToChar);

	}
	else if (eAct == MM::AfterSet)
	{
		pProp->Get(input);

		CDeviceUtils::Tokenize(input, tokenInput, "X=");

		x = strtod(tokenInput[0].c_str(), &pEnd);
	    y = strtod(tokenInput[1].c_str(), &pEnd);

		err = SetPositionMm(x, y);
		if (err != DEVICE_OK)
			return err;
	}

	return DEVICE_OK;
}

int PmacXYStage::OnVelocityX(MM::PropertyBase* pProp, MM::ActionType eAct){

	double vel;
	int err;
	if (eAct == MM::BeforeGet)
	{
		err = getAxisInfo(InfoType_velocity);
		//if(err != ERRO_NOERR)
		//	return DEVICE_ERR;
		double dfAxsVlc = m_PCommDev.m_Axis[xAxisID_].m_AxsInf.m_fVlcCur * 1000;
		CMCAxsGetPst(&m_PCommDev.m_Axis[xAxisID_].m_AxsInf, dfAxsVlc, &velocityX_);
		pProp->Set(velocityX_);
	}
	else if (eAct == MM::AfterSet)
	{
		pProp->Get(vel);

		double dfAxsVlc;
		CMCAxsGetPls(&m_PCommDev.m_Axis[xAxisID_].m_AxsInf, vel, &dfAxsVlc);
		m_PCommDev.m_Axis[xAxisID_].m_AxsInf.m_fVlcCur = dfAxsVlc/ 1000;

		int nAxsSel = DEV_AXS_SEL[xAxisID_];
		MCDevAxisSelSet(&m_PCommDev, nAxsSel, AXS_CMDTYPE_SET_VLC);
		err = MCDevCmdSend(&m_PCommDev, m_chRes, m_nChSndLen, m_chSnd);

		//if(err != ERRO_NOERR)
		//	return DEVICE_ERR;
		velocityX_ = vel;
	}
	
	return DEVICE_OK;
}
int PmacXYStage::OnVelocityY(MM::PropertyBase* pProp, MM::ActionType eAct){

	double vel;
	int err;
	if (eAct == MM::BeforeGet)
	{
		err = getAxisInfo(InfoType_velocity);
		//if(err != ERRO_NOERR)
		//	return DEVICE_ERR;
		double dfAxsVlc = m_PCommDev.m_Axis[yAxisID_].m_AxsInf.m_fVlcCur * 1000;
		CMCAxsGetPst(&m_PCommDev.m_Axis[yAxisID_].m_AxsInf, dfAxsVlc, &velocityY_);
		pProp->Set(velocityY_);
	}
	else if (eAct == MM::AfterSet)
	{
		pProp->Get(vel);
		double dfAxsVlc;
		CMCAxsGetPls(&m_PCommDev.m_Axis[yAxisID_].m_AxsInf, vel, &dfAxsVlc);
		m_PCommDev.m_Axis[yAxisID_].m_AxsInf.m_fVlcCur = dfAxsVlc/ 1000;

		int nAxsSel = DEV_AXS_SEL[yAxisID_];
		MCDevAxisSelSet(&m_PCommDev, nAxsSel, AXS_CMDTYPE_SET_VLC);
		err = MCDevCmdSend(&m_PCommDev, m_chRes, m_nChSndLen, m_chSnd);
		//if(err != ERRO_NOERR)
		//	return DEVICE_ERR;
		velocityY_ = vel;
	}
	
	return DEVICE_OK;
}
int PmacXYStage::IsXYStageSequenceable(bool& isSequenceable) const
{
	isSequenceable = false;
	return DEVICE_OK;
}
int PmacXYStage::OnDeviceID(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		long nm;
		pProp->Get(nm);
		deviceId_ = (int)nm;
		
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set((long)deviceId_);
	}
	return DEVICE_OK;
}
int PmacXYStage::OnPmacLink( MM::PropertyBase* pProp, MM::ActionType eAct )
{
	if (eAct == MM::AfterSet)
	{
		int ret = StatusChk_Pmac();
		std::string vcpl;
		pProp->Get(vcpl);
		if(ret != DEVICE_OK)
		{
			pmacLink_.assign("OffLine");
			return DEVICE_NOT_CONNECTED;
		}
		if(vcpl.compare("OnLine") == 0 && ret != DEVICE_OK)
		{
			int ret = Open_Pmac();
			if(ret == DEVICE_OK )
			{
				pmacLink_.assign("OnLine");
			}
			else
				return DEVICE_NOT_CONNECTED;
		}
		else if(vcpl.compare("OffLine") == 0 &&  ret == DEVICE_OK)
		{
			int ret = Close_Pmac();
			if(ret == DEVICE_OK)
				pmacLink_.assign("OffLine");
			else
				return DEVICE_ERR;
		}
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(pmacLink_.c_str());
	}
	return(DEVICE_OK);
}


int PmacXYStage::DeviceSelect_Pmac() 
{
	m_PCommDev.m_dwID = PmacDevSelect(NULL);
	//CString csTmp;
	//csTmp.Format("%d", m_PCommDev.m_dwID);
	//((CEdit *)GetDlgItem(IDC_EDT_DEV_ID))->SetWindowText(csTmp);
	OnFileWtInf();
	return DEVICE_OK;
}

int PmacXYStage::Shutdown()
{
	if (initialized_)
	{
		initialized_ = DEVICE_ERR;
	}
	return Close_Pmac();
}

bool PmacXYStage::Busy()
{
	//if (timeOutTimer_ == 0)
	//	return false;
	//if (timeOutTimer_->expired(GetCurrentMMTime()))
	//{
	//	// delete(timeOutTimer_);
	//	return false;
	//}
	//return true;
		//err = GetPositionMm(xCurrent,yCurrent);
		int err = getAxisInfo(InfoType_status);
		if (err != DEVICE_OK)
		return false;
		bool xarrived =  (m_PCommDev.m_Axis[xAxisID_].m_AxsStt.m_nInPst == 1);//(fabs(lastX+x-xCurrent) < stepSize_mm_); 
		bool yarrived =(m_PCommDev.m_Axis[yAxisID_].m_AxsStt.m_nInPst == 1);//(fabs(lastY+y-yCurrent) < stepSize_mm_);

		bool openloop =  (m_PCommDev.m_Axis[xAxisID_].m_AxsStt.m_nOpenLoop == 1) || (m_PCommDev.m_Axis[yAxisID_].m_AxsStt.m_nOpenLoop == 1);

		if(openloop || (xarrived && yarrived))
			return false;
		else
			return true;

}
int PmacXYStage::getAxisInfo(InfoType infoT)//获取轴的状态信息
{
	// TODO: Add your message handler code here and/or call default
	int nAxsSel = DEV_AXS_SEL[xAxisID_] | DEV_AXS_SEL[yAxisID_];
	MCDevAxisSelSet(&m_PCommDev, nAxsSel, infoT);
	int nComStat = MCDevCmdSend(&m_PCommDev, m_chRes, m_nChSndLen, m_chSnd);
	if (nComStat)
	{
		m_PCommDev.m_nDevStt = 0x00;
		return DEVICE_ERR;
	}
	return DEVICE_OK;
}

int PmacXYStage::OnTimerSetView()//获取轴的状态信息
{
	// TODO: Add your message handler code here and/or call default
	string csTmp;
	//////////////////////////////////////////////////////////////////////////
	//显示各轴当前位置
	double dfAxsPst = 0.0f;
	//1
	//csTmp.Format("%1.2f",m_PCommDev.m_Axis[0].m_AxsDat.m_dfPlsCur);
	CMCAxsGetPst(&m_PCommDev.m_Axis[0].m_AxsInf, 
		m_PCommDev.m_Axis[0].m_AxsDat.m_dfPlsCur, &dfAxsPst);
	//csTmp.Format("%1.4f",dfAxsPst);
	//((CEdit *)GetDlgItem(IDC_EDT_PSTC_AX01))->SetWindowText(csTmp);
	//2
	//csTmp.Format("%1.2f",m_PCommDev.m_Axis[1].m_AxsDat.m_dfPlsCur);
	CMCAxsGetPst(&m_PCommDev.m_Axis[1].m_AxsInf, 
		m_PCommDev.m_Axis[1].m_AxsDat.m_dfPlsCur, &dfAxsPst);
	//csTmp.Format("%1.4f",dfAxsPst);
	//((CEdit *)GetDlgItem(IDC_EDT_PSTC_AX02))->SetWindowText(csTmp);
	//3
	//csTmp.Format("%1.2f",m_PCommDev.m_Axis[2].m_AxsDat.m_dfPlsCur);;
	CMCAxsGetPst(&m_PCommDev.m_Axis[2].m_AxsInf, 
		m_PCommDev.m_Axis[2].m_AxsDat.m_dfPlsCur, &dfAxsPst);
	//csTmp.Format("%1.4f",dfAxsPst);
	//((CEdit *)GetDlgItem(IDC_EDT_PSTC_AX03))->SetWindowText(csTmp);
	//4
	//csTmp.Format("%1.2f",m_PCommDev.m_Axis[3].m_AxsDat.m_dfPlsCur);;
	CMCAxsGetPst(&m_PCommDev.m_Axis[3].m_AxsInf, 
		m_PCommDev.m_Axis[3].m_AxsDat.m_dfPlsCur, &dfAxsPst);
	//csTmp.Format("%1.4f",dfAxsPst);
	//((CEdit *)GetDlgItem(IDC_EDT_PSTC_AX04))->SetWindowText(csTmp);

	return DEVICE_OK;
}


int PmacXYStage::Open_Pmac() 
{
	// TODO: Add your control notification handler code here
	//设置设备ID
	//是否已经打开
	if (0x01 == m_PCommDev.m_nDevStt) 
	{
		return DEVICE_OK;
	}

	//打开设备
	int nFlag =	MCDevOpen(&m_PCommDev);
	if (nFlag)
	{
		m_PCommDev.m_nDevStt = 0x01;
		//((CEdit *)GetDlgItem(IDC_EDT_DEV_STT))->SetWindowText("成功:连接");
		return DEVICE_OK;
	}
	else 
		return DEVICE_ERR;
}


int PmacXYStage::OnFileRdInf()//读参数设置
{
	// TODO: Add your message handler code here and/or call default
	char csTmp[MAXSIZE_CHAR_CNFG];
	sprintf(csTmp,FILENAME_INFO , PRJCT_NAME);
	if (!m_pfInf)
	{
		m_pfInf = fopen(csTmp, "rb+");
		if (!m_pfInf)
		{
			m_pfInf = fopen(csTmp, "wb+");
			OnFileWtInf();
			return DEVICE_ERR;
		}

		if (!m_pfInf)
		{
			return DEVICE_ERR;
		}
	}

	fseek(m_pfInf, 0L, SEEK_SET);
	fscanf(m_pfInf, "%X\n", &(m_PCommDev.m_dwID));

	fclose(m_pfInf);
	m_pfInf = NULL;

	return DEVICE_OK;
}

int PmacXYStage::OnFileWtInf()//写参数设置
{
	// TODO: Add your message handler code here and/or call default
	char csTmp[MAXSIZE_CHAR_CNFG];
	sprintf(csTmp,FILENAME_INFO , PRJCT_NAME);

	if (!m_pfInf)
	{
		m_pfInf = fopen(csTmp, "rb+");
		if (!m_pfInf)
		{
			m_pfInf = fopen(csTmp, "wb+");
		}

		if (!m_pfInf)
		{
			return DEVICE_ERR;
		}
	}

	fseek(m_pfInf, 0L, SEEK_SET);
	fprintf(m_pfInf, "%08X\n", m_PCommDev.m_dwID);

	fclose(m_pfInf);
	m_pfInf = NULL;

	return DEVICE_OK;
}

int PmacXYStage::OnFileRdDat()//读历史数据
{
	// TODO: Add your message handler code here and/or call default
	char csTmp[MAXSIZE_CHAR_CNFG];
	sprintf(csTmp,FILENAME_INFO , PRJCT_NAME);

	if (!m_pfDat)
	{
		m_pfDat = fopen(csTmp, "rb+");
		if (!m_pfDat)
		{
			m_pfDat = fopen(csTmp, "wb+");
			OnFileWtDat();
			return DEVICE_ERR;
		}

		if (!m_pfDat)
		{
			return DEVICE_ERR;
		}
	}

	double AxsDatPlsBgn[8] = {0};
	fseek(m_pfDat, 0L, SEEK_SET);
	fprintf(
		m_pfDat, "%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n",
		&AxsDatPlsBgn[0],
		&AxsDatPlsBgn[1],
		&AxsDatPlsBgn[2],
		&AxsDatPlsBgn[3],
		&AxsDatPlsBgn[4],
		&AxsDatPlsBgn[5],
		&AxsDatPlsBgn[6],
		&AxsDatPlsBgn[7]
	);

	for(int nAxsID = 0x00; nAxsID < 8; nAxsID++)
	{
		m_PCommDev.m_Axis[nAxsID].m_AxsDat.m_dfPlsCur = 
			m_PCommDev.m_Axis[nAxsID].m_AxsDat.m_dfPlsBgn = 
			AxsDatPlsBgn[nAxsID];
	}

	fclose(m_pfDat);
	m_pfDat = NULL;

	return DEVICE_OK;
}

int PmacXYStage::OnFileWtDat()//写历史数据
{
	// TODO: Add your message handler code here and/or call default
	char csTmp[MAXSIZE_CHAR_CNFG];
	sprintf(csTmp,FILENAME_DATA , PRJCT_NAME);
	if (!m_pfDat)
	{
		m_pfDat = fopen(csTmp, "rb+");
		if (!m_pfDat)
		{
			m_pfDat = fopen(csTmp, "wb+");
		}

		if (!m_pfDat)
		{
			return DEVICE_ERR;
		}
	}

	double AxsDatPlsBgn[8] = {0};	
	for(int nAxsID = 0x00; nAxsID < 8; nAxsID++)
	{
		AxsDatPlsBgn[nAxsID] = 
			m_PCommDev.m_Axis[nAxsID].m_AxsDat.m_dfPlsCur;
	}

	fseek(m_pfDat, 0L, SEEK_SET);
	fprintf(
		m_pfDat, "%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n",
		AxsDatPlsBgn[0],
		AxsDatPlsBgn[1],
		AxsDatPlsBgn[2],
		AxsDatPlsBgn[3],
		AxsDatPlsBgn[4],
		AxsDatPlsBgn[5],
		AxsDatPlsBgn[6],
		AxsDatPlsBgn[7]
	);

	return DEVICE_OK;
}

int PmacXYStage::Close_Pmac() 
{
	// TODO: Add your control notification handler code here
	//是否已经关闭
	if (0x00 == m_PCommDev.m_nDevStt) 
	{
		return DEVICE_OK;
	}

	int nFlag =	MCDevClose(&m_PCommDev);
	if (nFlag)
	{
		m_PCommDev.m_nDevStt = 0x00;
		//TODO:set the device status label 
		return DEVICE_OK;
	}
	else
		return DEVICE_ERR;
}

//读取设备配置文件
void PmacXYStage::OnGetCnfgAxs(CMCAxis * theAxs)
{
	//公共初值
	m_csFNameIni = _T(FILENAME_CNFG);
	wstring  widstr;
	char tmp[255];
	sprintf(tmp,"AXIS%02d",theAxs->m_AxsInf.m_nAxsID -1);
	string st = tmp;
	widstr = std::wstring(st.begin(), st.end());
	m_csSctName =(LPWSTR)widstr.c_str();
	//m_csSctName.Format("AXIS%02d",(theAxs->m_AxsInf.m_nAxsID));
	m_csStrDfut = _T("1.00");

	//独立项数值
	//类别
	m_csStrDfut = _T("2");
	m_csKeyName = _T("nType");
	::GetPrivateProfileString(
		m_csSctName,		// section name
		m_csKeyName,		// key name
		m_csStrDfut,		// default string
		(LPWSTR)tmp,				// destination buffer
		MAXSIZE_CHAR_CNFG,	// size of destination buffer
		m_csFNameIni		// initialization file name
		);
	wcstombs(tmp,(LPWSTR)tmp,MAXSIZE_CHAR_CNFG);
	theAxs->m_AxsInf.m_nAxsTyp = atoi(tmp);

	//导程
	m_csStrDfut = _T("1");
	m_csKeyName = _T("fLead");
	::GetPrivateProfileString(
		m_csSctName,		// section name
		m_csKeyName,		// key name
		m_csStrDfut,		// default string
		(LPWSTR)tmp,			// destination buffer
		MAXSIZE_CHAR_CNFG,	// size of destination buffer
		m_csFNameIni		// initialization file name
		);
	wcstombs(tmp,(LPWSTR)tmp,MAXSIZE_CHAR_CNFG);
	theAxs->m_AxsInf.m_fLead = atof(tmp);

	//步距角
	m_csStrDfut = _T("10000.0");
	m_csKeyName = _T("fStpnNg");
	::GetPrivateProfileString(
		m_csSctName,		// section name
		m_csKeyName,		// key name
		m_csStrDfut,		// default string
		(LPWSTR)tmp,				// destination buffer
		MAXSIZE_CHAR_CNFG,	// size of destination buffer
		m_csFNameIni		// initialization file name
		);
	wcstombs(tmp,(LPWSTR)tmp,MAXSIZE_CHAR_CNFG);
	theAxs->m_AxsInf.m_fStpnNg = atof(tmp);

	//细分
	m_csStrDfut = _T("1");
	m_csKeyName = _T("fSbdvsn");
	::GetPrivateProfileString(
		m_csSctName,		// section name
		m_csKeyName,		// key name
		m_csStrDfut,		// default string
		(LPWSTR)tmp,			// destination buffer
		MAXSIZE_CHAR_CNFG,	// size of destination buffer
		m_csFNameIni		// initialization file name
		);
	wcstombs(tmp,(LPWSTR)tmp,MAXSIZE_CHAR_CNFG);
	theAxs->m_AxsInf.m_fSbdvsn = atof(tmp);

	//减速比
	m_csStrDfut = _T("1");
	m_csKeyName = _T("fDecRate");
	::GetPrivateProfileString(
		m_csSctName,		// section name
		m_csKeyName,		// key name
		m_csStrDfut,		// default string
		(LPWSTR)tmp,			// destination buffer
		MAXSIZE_CHAR_CNFG,	// size of destination buffer
		m_csFNameIni		// initialization file name
		);
	wcstombs(tmp,(LPWSTR)tmp,MAXSIZE_CHAR_CNFG);
	theAxs->m_AxsInf.m_fDecRate = atof(tmp);

	//单位时间脉冲
	m_csStrDfut = _T("1");
	m_csKeyName = _T("fVlcCur");
	::GetPrivateProfileString(
		m_csSctName,		// section name
		m_csKeyName,		// key name
		m_csStrDfut,		// default string
		(LPWSTR)tmp,			// destination buffer
		MAXSIZE_CHAR_CNFG,	// size of destination buffer
		m_csFNameIni		// initialization file name
		);
	wcstombs(tmp,(LPWSTR)tmp,MAXSIZE_CHAR_CNFG);
	theAxs->m_AxsInf.m_fVlcCur = atof(tmp);

	//最大速度
	m_csStrDfut = _T("5.0");
	m_csKeyName = _T("fSpdMax");
	::GetPrivateProfileString(
		m_csSctName,		// section name
		m_csKeyName,		// key name
		m_csStrDfut,		// default string
		(LPWSTR)tmp,			// destination buffer
		MAXSIZE_CHAR_CNFG,	// size of destination buffer
		m_csFNameIni		// initialization file name
		);
	wcstombs(tmp,(LPWSTR)tmp,MAXSIZE_CHAR_CNFG);
	theAxs->m_AxsInf.m_fSpdMax = atof(tmp);

	//负极限位
	m_csStrDfut = _T("-100.0");
	m_csKeyName = _T("PositionMin");
	::GetPrivateProfileString(
		m_csSctName,		// section name
		m_csKeyName,		// key name
		m_csStrDfut,		// default string
		(LPWSTR)tmp,			// destination buffer
		MAXSIZE_CHAR_CNFG,	// size of destination buffer
		m_csFNameIni		// initialization file name
		);
	wcstombs(tmp,(LPWSTR)tmp,MAXSIZE_CHAR_CNFG);
	positionMin_[theAxs->m_AxsInf.m_nAxsID -1] = atof(tmp);

	//正极限位	
	m_csStrDfut = _T("100.0");
	m_csKeyName = _T("PositionMax");
	::GetPrivateProfileString(
		m_csSctName,		// section name
		m_csKeyName,		// key name
		m_csStrDfut,		// default string
		(LPWSTR)tmp,			// destination buffer
		MAXSIZE_CHAR_CNFG,	// size of destination buffer
		m_csFNameIni		// initialization file name
		);
	wcstombs(tmp,(LPWSTR)tmp,MAXSIZE_CHAR_CNFG);
	positionMax_[theAxs->m_AxsInf.m_nAxsID -1] = atof(tmp);
}

int PmacXYStage::StatusChk_Pmac()
{
	string csTmp;
	if (!(m_PCommDev.m_nDevStt))
	{
		//csTmp.Format("%s\n%s\n%s\n%s\n%s\n%s\n"
		//	,_T("控制器通讯失败")
		//	,_T("1：设备是否供电正常？")
		//	,_T("2：软件是否正确安装？")
		//	,_T("3：通讯线是否连接正确？")
		//	,_T("4：通讯端口选择是否正确？")
		//	,_T("如要重新配置设备通讯，请单击“设备设置”进行配置"));

		//AfxMessageBox(csTmp);
		return DEVICE_ERR;
	}
	return DEVICE_OK;
}

int PmacXYStage::Init_Pmac()
{
	int ret= DEVICE_OK;
	string csTmp;
	//////////////////////////////////////////////////////////////////////////
	//变量初始化
	m_chRes[0] = m_chSnd[0] = '\0';
	m_nChResLen = m_nChSndLen = 512;
	m_pfInf = m_pfDat = NULL;

	////状态信息请求序列
	//m_nInfPckCmd[0] = AXS_CMDTYPE_GET_PST;//信息指令包
	//m_nInfPckCmd[1] = AXS_CMDTYPE_GET_STT;//信息指令包
	//m_nInfPckCmd[2] = AXS_CMDTYPE_UNDEF;//信息指令包
	//m_nInfPckIndex = 0;//信息指令包指令序号
	DEV_AXS_SEL[0] = DEV_AXS_SEL_01;
	DEV_AXS_SEL[1] = DEV_AXS_SEL_02;
	DEV_AXS_SEL[2] = DEV_AXS_SEL_03;
	DEV_AXS_SEL[3] = DEV_AXS_SEL_04;
	DEV_AXS_SEL[4] = DEV_AXS_SEL_05;
	DEV_AXS_SEL[5] = DEV_AXS_SEL_06;
	DEV_AXS_SEL[6] = DEV_AXS_SEL_07;
	DEV_AXS_SEL[7] = DEV_AXS_SEL_08;

	//系统初始化
	m_PCommDev.m_nRWStep = DEV_RWSTP_RD;
	m_PCommDev.m_nSRType = DEV_SRTYP_SD;

	//初始化运动控制设备信息
	m_PCommDev.m_dwID = 0;

	//读历史及参数数据
	OnFileRdDat();
	OnFileRdInf();
	
	//显示动态库加载信息
	//csTmp.Format("%d", m_PCommDev.m_dwID);
	//((CEdit *)GetDlgItem(IDC_EDT_DEV_ID))->SetWindowText(csTmp);
	int nFlag = MCSysInit(&m_PCommDev);
	if (ERRO_NOERR != nFlag) 
	{
		switch(nFlag)
		{
		case ERRO_INIT_LINK:			
			//csTmp.Format("%s", _T("错误:加载库"));
			break;
		case ERRO_INIT_OPEN:
			//csTmp.Format("%s", _T("错误:设备ID"));
			break;
		default:
			;
			//csTmp.Format("%s", _T("错误:未知"));
		}
		//((CEdit *)GetDlgItem(IDC_EDT_DEV_STT))->SetWindowText(csTmp);
		ret = DEVICE_ERR;
	}
	else
	{
		//((CEdit *)GetDlgItem(IDC_EDT_DEV_STT))->SetWindowText("成功:连接");

		//SetTimer(TIMER_DEVCM_EVENT, TIMER_DEVCM_TIME, NULL);
		//SetTimer(TIMER_VEWUD_EVENT, TIMER_VEWUD_TIME, NULL);
		ret = DEVICE_OK;		

	}

	//修改运动轴参数:伺服
	for(int nIndex = 0; nIndex < 4; nIndex++)
	{
		m_PCommDev.m_Axis[nIndex].m_AxsInf.m_nAxsID = nIndex + 1;
		OnGetCnfgAxs(&(m_PCommDev.m_Axis[nIndex]));
	}

	//修改运动轴控制方式:R轴
	m_PCommDev.m_Axis[3].m_AxsCtr.m_nCtrTyp = PST_CTR_TYPE_OLOOP;

	//设置默认值
	//csTmp.Format("%s", _T("0.0000"));
	//((CEdit *)GetDlgItem(IDC_EDT_PSTA_AX01))->SetWindowText(csTmp);
	//((CEdit *)GetDlgItem(IDC_EDT_PSTA_AX02))->SetWindowText(csTmp);
	//((CEdit *)GetDlgItem(IDC_EDT_PSTA_AX03))->SetWindowText(csTmp);
	//((CEdit *)GetDlgItem(IDC_EDT_PSTA_AX04))->SetWindowText(csTmp);

	//使能所有设备
	StatusChk_Pmac();

	//更新一次数据
	//OnTimerSetView();

	//////////////////////////////////////////////////////////////////////////

	return ret;  // return DEVICE_OK  unless you set the focus to a control
}

int PmacXYStage::EnableAll_Pmac() 
{
	// TODO: Add your control notification handler code here
	int nIndex = 0;

	if (StatusChk_Pmac()!=DEVICE_OK)
	{
		return DEVICE_ERR;
	}

	char chBuf[12] = { '\0'	};
	m_chRes[0] = '\0';
	m_chSnd[0] = '\0';

	for(nIndex = 1; nIndex <= 4; nIndex++)
	{
		sprintf(chBuf,"%d00=1 ",nIndex);
		strcat(m_chSnd, chBuf);
	}

	for(nIndex = 1; nIndex <= 4; nIndex++)
	{
		sprintf(chBuf,"#%dj/ ",nIndex);
		strcat(m_chSnd, chBuf);
	}

	int n = PmacGetResponce(m_PCommDev.m_dwID, m_chRes, m_nChResLen, m_chSnd);	
	return DEVICE_OK;
}