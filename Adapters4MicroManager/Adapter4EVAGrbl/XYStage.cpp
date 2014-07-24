///////////////////////////////////////////////////////////////////////////////
// FILE:          XYStage.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Thorlabs device adapters: BBD102 Controller
//
// COPYRIGHT:     Thorlabs, 2011
//
// LICENSE:       This file is distributed under the BSD license.
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.
//
// AUTHOR:        Nenad Amodaj, nenad@amodaj.com, 2011
//

#include "../../MMDevice/ModuleInterface.h"
#include "XYStage.h"
#include <sstream>
#include "EVA_NDE_Grbl.h"

///////////
// properties
///////////

const char* g_XYStageDeviceName = "XYStage";

const char* g_SerialNumberProp = "SerialNumber";
const char* g_ModelNumberProp = "ModelNumber";
const char* g_SWVersionProp = "SoftwareVersion";

const char* g_StepSizeXProp = "StepSizeX";
const char* g_StepSizeYProp = "StepSizeY";
const char* g_MaxVelocityProp = "MaxVelocity";
const char* g_AccelProp = "Acceleration";
const char* g_MoveTimeoutProp = "MoveTimeoutMs";

const char* g_SyncStepProp = "SyncStep";
using namespace std;

///////////
// fixed stage parameters
///////////
const long xAxisMaxSteps = 2200000L;   // maximum number of steps in X
const long yAxisMaxSteps = 1500000L;   // maximum number of steps in Y
const double stepSizeUm = 0.05;        // step size in microns
const double accelScale = 13.7438;     // scaling factor for acceleration
const double velocityScale = 134218.0; // scaling factor for velocity

///////////////////////////////////////////////////////////////////////////////
// CommandThread class
// (for executing move commands)
///////////////////////////////////////////////////////////////////////////////

class XYStage::CommandThread : public MMDeviceThreadBase
{
   public:
      CommandThread(XYStage* stage) :
         stop_(false), moving_(false), stage_(stage), errCode_(DEVICE_OK) {}

      virtual ~CommandThread() {}
	  
      int svc()
      {
		CEVA_NDE_GrblHub* hub = static_cast<CEVA_NDE_GrblHub*>(stage_->GetParentHub());
		if (!hub || !hub->IsPortAvailable()) {
			return ERR_NO_PORT_SET;
		}
		while(1)
		{
			int ret = hub->GetStatus();
			if(ret != DEVICE_OK){
				moving_ = false;
				break;
			}
			char buf[30]="";
			ret = hub->GetProperty("Status",buf);
			if((strcmp(buf,"Idle") == 0 )){
				moving_ = false;
				break;
			}
			else
				moving_ = true;
			CDeviceUtils::SleepMs(10);
		}

         return 0;
      }
      void Stop() {stop_ = true;}
      bool GetStop() {return stop_;}
      int GetErrorCode() {return errCode_;}
      bool IsMoving()  {  return moving_;}

   private:
      void Reset() {stop_ = false; errCode_ = DEVICE_OK; moving_ = false;}
      enum Command {MOVE, MOVEREL, HOME};
      bool stop_;
      bool moving_;
      XYStage* stage_;
      long x_;
      long y_;
      Command cmd_;
      Command lastCmd_;
      int errCode_;
};

///////////////////////////////////////////////////////////////////////////////
// XYStage class
///////////////////////////////////////////////////////////////////////////////

XYStage::XYStage() :
   CXYStageBase<XYStage>(),
   initialized_(false),
   home_(false),
   answerTimeoutMs_(1000.0),
   moveTimeoutMs_(10000.0),
   cmdThread_(0)
{
   // set default error messages
   InitializeDefaultErrorMessages();


   // create pre-initialization properties
   // ------------------------------------

   // Name
   CreateProperty(MM::g_Keyword_Name, g_XYStageDeviceName, MM::String, true);

   // Description
   CreateProperty(MM::g_Keyword_Description, "XY stage adapter for EvaGrbl -- by Wei Ouyang", MM::String, true);


   cmdThread_ = new CommandThread(this);
}

XYStage::~XYStage()
{
   Shutdown();
}

///////////////////////////////////////////////////////////////////////////////
// XY Stage API
// required device interface implementation
///////////////////////////////////////////////////////////////////////////////
void XYStage::GetName(char* name) const
{
   CDeviceUtils::CopyLimitedString(name, g_XYStageDeviceName);
}

int XYStage::Initialize()
{
   CEVA_NDE_GrblHub* hub = static_cast<CEVA_NDE_GrblHub*>(GetParentHub());
   if (!hub || !hub->IsPortAvailable()) {
      return ERR_NO_PORT_SET;
   }
   char hubLabel[MM::MaxStrLength];
   hub->GetLabel(hubLabel);
   SetParentID(hubLabel); // for backward comp.
   parameters_ = &hub->parameters;
   int ret = DEVICE_ERR;

   // Step size
   CreateProperty(g_StepSizeXProp, CDeviceUtils::ConvertToString(stepSizeUm), MM::Float, true);
   CreateProperty(g_StepSizeYProp, CDeviceUtils::ConvertToString(stepSizeUm), MM::Float, true);

   // Max Speed
   CPropertyAction* pAct = new CPropertyAction (this, &XYStage::OnMaxVelocity);
   CreateProperty(g_MaxVelocityProp, "100.0", MM::Float, false, pAct);
   //SetPropertyLimits(g_MaxVelocityProp, 0.0, 31999.0);

   // Acceleration
   pAct = new CPropertyAction (this, &XYStage::OnAcceleration);
   CreateProperty(g_AccelProp, "100.0", MM::Float, false, pAct);
   //SetPropertyLimits("Acceleration", 0.0, 150);

   // Move timeout
   pAct = new CPropertyAction (this, &XYStage::OnMoveTimeout);
   CreateProperty(g_MoveTimeoutProp, "10000.0", MM::Float, false, pAct);
   //SetPropertyLimits("Acceleration", 0.0, 150);

   // Sync Step
   pAct = new CPropertyAction (this, &XYStage::OnSyncStep);
   CreateProperty(g_SyncStepProp, "1.0", MM::Float, false, pAct);
   //SetPropertyLimits("Acceleration", 0.0, 150);


   ret = UpdateStatus();
   if (ret != DEVICE_OK)
      return ret;

   initialized_ = true;
   return DEVICE_OK;
}

int XYStage::Shutdown()
{

   if (cmdThread_ && cmdThread_->IsMoving())
   {
      cmdThread_->Stop();
      cmdThread_->wait();
   }

   delete cmdThread_;
   cmdThread_ = 0;

   if (initialized_)
      initialized_ = false;

   return DEVICE_OK;
}

bool XYStage::Busy()
{
	return false;
}
 
double XYStage::GetStepSizeXUm()
{
	CEVA_NDE_GrblHub* hub = static_cast<CEVA_NDE_GrblHub*>(GetParentHub());
	if (!hub || !hub->IsPortAvailable()) {
		return ERR_NO_PORT_SET;
	}
   return 1000.0/(*parameters_)[0];
}

double XYStage::GetStepSizeYUm()
{
	CEVA_NDE_GrblHub* hub = static_cast<CEVA_NDE_GrblHub*>(GetParentHub());
	if (!hub || !hub->IsPortAvailable()) {
		return ERR_NO_PORT_SET;
	}
   return 1000.0/(*parameters_)[1];
}

int XYStage::SetPositionSteps(long x, long y)
{

   SetPositionUm(x*GetStepSizeXUm(), y*GetStepSizeYUm());
   CDeviceUtils::SleepMs(10); // to make sure that there is enough time for thread to get started

   return DEVICE_OK;   
}
 
int XYStage::SetRelativePositionSteps(long x, long y)
{
   SetRelativePositionUm(x*GetStepSizeXUm(), y*GetStepSizeYUm());
   return DEVICE_OK;
}
int XYStage::GetPositionUm(double& x, double& y){
   int ret;
   	CEVA_NDE_GrblHub* hub = static_cast<CEVA_NDE_GrblHub*>(GetParentHub());
	if (!hub || !hub->IsPortAvailable()) {
		return ERR_NO_PORT_SET;
	}
    ret = hub->GetStatus();
	if (ret != DEVICE_OK)
    return ret;
	x =  hub->MPos[0]*1000.0 ;
	y =   hub->MPos[1]*1000.0;
   ostringstream os;
   os << "GetPositionSteps(), X=" << x << ", Y=" << y;
   LogMessage(os.str().c_str(), true);
}
int XYStage::GetPositionSteps(long& x, long& y)
{
   int ret;
   	CEVA_NDE_GrblHub* hub = static_cast<CEVA_NDE_GrblHub*>(GetParentHub());
	if (!hub || !hub->IsPortAvailable()) {
		return ERR_NO_PORT_SET;
	}
    ret = hub->GetStatus();
	if (ret != DEVICE_OK)
    return ret;
	x =  hub->MPos[0]*1000 /GetStepSizeXUm();
	y =   hub->MPos[1]*1000 /GetStepSizeXUm();
   ostringstream os;
   os << "GetPositionSteps(), X=" << x << ", Y=" << y;
   LogMessage(os.str().c_str(), true);
   return DEVICE_OK;
}
int XYStage::SetPositionUm(double x, double y){
	 CEVA_NDE_GrblHub* hub = static_cast<CEVA_NDE_GrblHub*>(GetParentHub());
	if (!hub || !hub->IsPortAvailable()) {
		return ERR_NO_PORT_SET;
	}
	int errCode_ = DEVICE_ERR;
	if(lastMode_ != MOVE){
		std::string tmp("G90");
		errCode_ = hub->SendCommand(tmp,tmp);
		lastMode_ = MOVE;
	}
	char buff[100];
	sprintf(buff, "G00X%fY%f", x/1000.0,y/1000.0);
	std::string buffAsStdStr = buff;
	errCode_ = hub->SendCommand(buffAsStdStr,buffAsStdStr); //stage_->MoveBlocking(x_, y_);

	ostringstream os;
	os << "Move finished with error code: " << errCode_;
	LogMessage(os.str().c_str(), true);
	return errCode_;
}
int XYStage::SetRelativePositionUm(double dx, double dy){
	 CEVA_NDE_GrblHub* hub = static_cast<CEVA_NDE_GrblHub*>(GetParentHub());
	if (!hub || !hub->IsPortAvailable()) {
		return ERR_NO_PORT_SET;
	}
	int errCode_ = DEVICE_ERR;
	if(lastMode_ != MOVEREL){
		std::string tmp("G91");
		errCode_ = hub->SendCommand(tmp,tmp);
		lastMode_ = MOVEREL;
	}
	char buff[100];
	sprintf(buff, "G00X%fY%f", dx/1000.0,dy/1000.0);
	std::string buffAsStdStr = buff;
    errCode_ = hub->SendCommand(buffAsStdStr,buffAsStdStr);  // relative move

    ostringstream os;
    os << "Move finished with error code: " << errCode_;
    LogMessage(os.str().c_str(), true);
	return errCode_;
}


/**
 * Performs homing for both axes
 * (required after initialization)
 */
int XYStage::Home()
{
   	CEVA_NDE_GrblHub* hub = static_cast<CEVA_NDE_GrblHub*>(GetParentHub());
	if (!hub || !hub->IsPortAvailable()) {
		return ERR_NO_PORT_SET;
	}
	int ret;
	ret = hub->SetProperty("Command","$H");
	if (ret != DEVICE_OK)
	{
		LogMessage(std::string("Homing error!"));
		return ret;
	}
	else
		home_ = true; // successfully homed
   // check status
   return DEVICE_OK;
}

/**
 * Stops XY stage immediately. Blocks until done.
 */
int XYStage::Stop()
{
   int ret;
   return DEVICE_OK;
}

/**
 * This is supposed to set the origin (0,0) at whatever is the current position.
 * Our stage does not support setting the origin (it is fixed). The base class
 * (XYStageBase) provides the default implementation SetAdapterOriginUm(double x, double y)
 * but we are not going to use since it would affect absolute coordinates during "Calibrate"
 * command in micro-manager.
 */
int XYStage::SetOrigin()
{
   // commnted oout since we do not really want to support setting the origin
   // int ret = SetAdapterOriginUm(0.0, 0.0);
   return DEVICE_OK; 
}
 
int XYStage::GetLimitsUm(double& xMin, double& xMax, double& yMin, double& yMax)
{
   xMin = 0.0;
   yMin = 0.0;
   xMax = xAxisMaxSteps * stepSizeUm;
   yMax = yAxisMaxSteps * stepSizeUm;

   return DEVICE_OK;
}

int XYStage::GetStepLimits(long& xMin, long& xMax, long& yMin, long& yMax)
{
   xMin = 0L;
   yMin = 0L;
   xMax = xAxisMaxSteps;
   yMax = yAxisMaxSteps;

   return DEVICE_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Action handlers
///////////////////////////////////////////////////////////////////////////////


/**
 * Gets and sets the maximum speed with which the stage travels
 */
int XYStage::OnMaxVelocity(MM::PropertyBase* pProp, MM::ActionType eAct) 
{
   if (eAct == MM::BeforeGet) 
   {
		pProp->Set(maxVelocity_);
   } 
   else if (eAct == MM::AfterSet) 
   {
	   pProp->Get(maxVelocity_);
   }

   return DEVICE_OK;
}

/**
 * Gets and sets the Acceleration of the stage travels
 */
int XYStage::OnAcceleration(MM::PropertyBase* pProp, MM::ActionType eAct) 
{
   if (eAct == MM::BeforeGet) 
   {
	    pProp->Set(acceleration_);
   } 
   else if (eAct == MM::AfterSet) 
   {
	   pProp->Get(acceleration_);

   }

   return DEVICE_OK;
}

/**
 * Gets and sets the Acceleration of the stage travels
 */
int XYStage::OnMoveTimeout(MM::PropertyBase* pProp, MM::ActionType eAct) 
{
   if (eAct == MM::BeforeGet) 
   {
      pProp->Set(moveTimeoutMs_);
   } 
   else if (eAct == MM::AfterSet) 
   {
      pProp->Get(moveTimeoutMs_);
   }

   return DEVICE_OK;
}
/**
 * Gets and sets the sync signal step
 */
int XYStage::OnSyncStep(MM::PropertyBase* pProp, MM::ActionType eAct) 
{
   if (eAct == MM::BeforeGet) 
   {

      pProp->Set(syncStep_);

   } 
   else if (eAct == MM::AfterSet) 
   {   
	   CEVA_NDE_GrblHub* hub = static_cast<CEVA_NDE_GrblHub*>(GetParentHub());
	   if (!hub || !hub->IsPortAvailable()) {
		  return ERR_NO_PORT_SET;
	   }

      pProp->Get(syncStep_);

	   int ret = hub->SetSync(0,syncStep_);
	   if(ret != DEVICE_OK)
		   return ret;
   }

   return DEVICE_OK;
}

///////////////////////////////////////////////////////////////////////////////
// private methods
///////////////////////////////////////////////////////////////////////////////


/**
 * Sends move command to both axes and waits for responses, blocking the calling thread.
 * If expected answers do not come within timeout interval, returns with error.
 */
int XYStage::MoveBlocking(long x, long y, bool relative)
{
   int ret;

   //if (!home_)
   //   return ERR_HOME_REQUIRED; 

   //// send command to X axis
   //ret = xstage_->MoveBlocking(x, relative);
   if (ret != DEVICE_OK)
      return ret;
   // send command to Y axis
   return ret;//ystage_->MoveBlocking(y, relative);
}

