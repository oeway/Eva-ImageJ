///////////////////////////////////////////////////////////////////////////////
// FILE:          EVA_NDE_Grbl.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   EVA_NDE_Grbl adapter.  Needs accompanying firmware
// COPYRIGHT:     University of California, San Francisco, 2008
// LICENSE:       LGPL
// 
// AUTHOR:        Nico Stuurman, nico@cmp.ucsf.edu 11/09/2008
//                automatic device detection by Karl Hoover
//
//

#include "EVA_NDE_Grbl.h"
#include "XYStage.h"
#include "../../MMDevice/ModuleInterface.h"
#include <sstream>
#include <cstdio>

#ifdef WIN32
   #define WIN32_LEAN_AND_MEAN
   #include <windows.h>
   #define snprintf _snprintf 
#endif

const char* g_DeviceNameEVA_NDE_GrblHub = "EVA_NDE_Grbl-Hub";
const char* g_DeviceNameEVA_NDE_GrblXYStage = "XYStage";
//const char* g_DeviceNameEVA_NDE_GrblZStage = "EVA_NDE_Grbl-ZStage";



// Global info about the state of the EVA_NDE_Grbl.  This should be folded into a class
unsigned g_switchState = 0;
unsigned g_shutterState = 0;
const int g_Min_MMVersion = 0;
const int g_Max_MMVersion = 10;
const char* g_versionProp = "Version";
const char* g_normalLogicString = "Normal";
const char* g_invertedLogicString = "Inverted";

// static lock
MMThreadLock CEVA_NDE_GrblHub::lock_;

///////////////////////////////////////////////////////////////////////////////
// Exported MMDevice API
///////////////////////////////////////////////////////////////////////////////
MODULE_API void InitializeModuleData()
{
   RegisterDevice(g_DeviceNameEVA_NDE_GrblHub, MM::HubDevice, "Hub (required)");
   RegisterDevice(g_DeviceNameEVA_NDE_GrblXYStage, MM::XYStageDevice, "XYStage");
   //AddAvailableDeviceName(g_DeviceNameEVA_NDE_GrblZStage, "ZStage");

}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
   if (deviceName == 0)
      return 0;

   if (strcmp(deviceName, g_DeviceNameEVA_NDE_GrblHub) == 0)
   {
      return new CEVA_NDE_GrblHub;
   }
   else if (strcmp(deviceName, g_DeviceNameEVA_NDE_GrblXYStage) == 0)
   {
      return new XYStage;
   }
   //else if (strcmp(deviceName, g_DeviceNameEVA_NDE_GrblZStage) == 0)
   //{
   //   return new CEVA_NDE_GrblZStage;
   //}
 
   return 0;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
   delete pDevice;
}
///////////////////////////////////////////////////////////////////////////////
// Misc
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

template <class Type>
Type stringToNum(const std::string& str)
{
	std::istringstream iss(str);
	Type num;
	iss >> num;
	return num;    
}

///////////////////////////////////////////////////////////////////////////////
// CEVA_NDE_GrblHUb implementation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
CEVA_NDE_GrblHub::CEVA_NDE_GrblHub() :
initialized_ (false)
{
   portAvailable_ = false;

   timedOutputActive_ = false;

   WPos[0] = 0.0;
   WPos[1] = 0.0;
   WPos[2] = 0.0;
   MPos[0] = 0.0;
   MPos[1] = 0.0;
   MPos[2] = 0.0;

   InitializeDefaultErrorMessages();

   SetErrorText(ERR_PORT_OPEN_FAILED, "Failed opening EVA_NDE_Grbl USB device");
   SetErrorText(ERR_BOARD_NOT_FOUND, "Did not find an EVA_NDE_Grbl board with the correct firmware.  Is the EVA_NDE_Grbl board connected to this serial port?");
   SetErrorText(ERR_NO_PORT_SET, "Hub Device not found.  The EVA_NDE_Grbl Hub device is needed to create this device");
   std::ostringstream errorText;
   errorText << "The firmware version on the EVA_NDE_Grbl is not compatible with this adapter.  Please use firmware version ";

   SetErrorText(ERR_VERSION_MISMATCH, errorText.str().c_str());

   CPropertyAction* pAct  = new CPropertyAction(this, &CEVA_NDE_GrblHub::OnPort);
   //CreateProperty("ComPort", "Undifined", MM::String, false, pAct);
   CreateProperty(MM::g_Keyword_Port, "Undefined", MM::String, false, pAct, true);

   pAct = new CPropertyAction(this, &CEVA_NDE_GrblHub::OnStatus);
   CreateProperty("Status", "-", MM::String, true, pAct);  //read only
}

CEVA_NDE_GrblHub::~CEVA_NDE_GrblHub()
{
   Shutdown();

}

void CEVA_NDE_GrblHub::GetName(char* name) const
{
   CDeviceUtils::CopyLimitedString(name, g_DeviceNameEVA_NDE_GrblHub);
}

bool CEVA_NDE_GrblHub::Busy()
{
   return false;
}

// private and expects caller to:
// 1. guard the port
// 2. purge the port
int CEVA_NDE_GrblHub::GetStatus()
{
   std::string cmd;
   cmd.assign("?"); // x step/mm
   std::string returnString;
   int ret = SendCommand(cmd,returnString);
   if (ret != DEVICE_OK)
   {
	 LogMessage("command send failed!");
    return ret;
   }
   std::vector<std::string> tokenInput;
	char* pEnd;
   	CDeviceUtils::Tokenize(returnString, tokenInput, "<>,:\r\n");
   //sample: <Idle,MPos:0.000,0.000,0.000,WPos:0.000,0.000,0.000>
	if(tokenInput.size() != 9)
	{
		LogMessage(returnString.c_str());
		LogMessage("echo error!");
		return DEVICE_ERR;
	}
	status.assign(tokenInput[0].c_str());
	MPos[0] = stringToNum<double>(tokenInput[2]);
	MPos[1] = stringToNum<double>(tokenInput[3]);
	MPos[2] = stringToNum<double>(tokenInput[4]);
	WPos[0] = stringToNum<double>(tokenInput[6]);
	WPos[1] = stringToNum<double>(tokenInput[7]);
	WPos[2] = stringToNum<double>(tokenInput[8]);
   return DEVICE_OK;

}
int CEVA_NDE_GrblHub::SetSync(int axis, double value ){
   std::string cmd;
   char buff[20];
   sprintf(buff, "M108P%.3fQ%d", value,axis);
   cmd.assign(buff); 
   std::string returnString;
   int ret = SendCommand(cmd,returnString);
   return ret;
}
int CEVA_NDE_GrblHub::SetParameter(int index, double value){
   std::string cmd;
   char buff[20];
   sprintf(buff, "$%d=%.3f", index,value);
   cmd.assign(buff); 
   std::string returnString;
   int ret = SendCommand(cmd,returnString);
   return ret;
}
//int CEVA_NDE_GrblHub::Reset(){
//	MMThreadGuard(this->executeLock_);
//   std::string cmd;
//   char buff[]={0x18,0x00};
//   cmd.assign(buff); 
//   std::string returnString;
//   SetAnswerTimeoutMs(2000.0);
//	PurgeComPortH();
//   int ret = SetCommandComPortH(cmd.c_str(),"\n");
//   if (ret != DEVICE_OK)
//    return ret;
//   std::string an;
//   ret = GetSerialAnswerComPortH(an,"]\r\n");
//   if (ret != DEVICE_OK)
//    return ret;
//   returnString = an;
//
//   std::vector<std::string> tokenInput;
//	char* pEnd;
//   	CDeviceUtils::Tokenize(returnString, tokenInput, "\r\n[");
//   if(tokenInput.size() != 2)
//	   return DEVICE_ERR;
//   version_ = tokenInput[0];
//   return ret;
//}

int CEVA_NDE_GrblHub::GetParameters()
{
	/*
	sample parameters:
	$0=250.000 (x, step/mm)
	$1=250.000 (y, step/mm)
	$2=250.000 (z, step/mm)
	$3=10 (step pulse, usec)
	$4=250.000 (default feed, mm/min)
	$5=500.000 (default seek, mm/min)
	$6=192 (step port invert mask, int:11000000)
	$7=25 (step idle delay, msec)
	$8=10.000 (acceleration, mm/sec^2)
	$9=0.050 (junction deviation, mm)
	$10=0.100 (arc, mm/segment)
	$11=25 (n-arc correction, int)
	$12=3 (n-decimals, int)
	$13=0 (report inches, bool)
	$14=1 (auto start, bool)
	$15=0 (invert step enable, bool)
	$16=0 (hard limits, bool)
	$17=0 (homing cycle, bool)
	$18=0 (homing dir invert mask, int:00000000)
	$19=25.000 (homing feed, mm/min)
	$20=250.000 (homing seek, mm/min)
	$21=100 (homing debounce, msec)
	$22=1.000 (homing pull-off, mm)
	*/
   if(!portAvailable_)
	   return ERR_NO_PORT_SET;
   std::string cmd;
   cmd.assign("$$"); // x step/mm
   std::string returnString;
   int ret = SendCommand(cmd,returnString);
   if (ret != DEVICE_OK)
    return ret;
   
   	std::vector<std::string> tokenInput;

	char* pEnd;
   	CDeviceUtils::Tokenize(returnString, tokenInput, "$=()\r\n");
   if(tokenInput.size() != PARAMETERS_COUNT*3)
	   return DEVICE_ERR;
   parameters.clear();
   std::vector<std::string>::iterator it;
   for (it=tokenInput.begin(); it!=tokenInput.end(); it+=3)
   {
	   std::string str = *(it+1);
	   parameters.push_back(stringToNum<double>(str));
   }
   return DEVICE_OK;

}
int CEVA_NDE_GrblHub::SendCommand(std::string command, std::string &returnString)
{
   if(!portAvailable_)
	   return ERR_NO_PORT_SET;
   // needs a lock because the other Thread will also use this function
   MMThreadGuard(this->executeLock_);
   PurgeComPortH();
   int ret = DEVICE_OK;
   	if(command.c_str()[0] == '$' && command.c_str()[1] == 'H') 
	{
		// Check that we have a controller:
		ret = GetStatus();
		if( DEVICE_OK != ret) 
		return ret;
		ret = GetParameters();
		if( DEVICE_OK != ret)
		return ret;
	   	SetAnswerTimeoutMs(60000.0);
	}
   else if(command.c_str()[0] == '$' || command.c_str()[0] == '?')
	  SetAnswerTimeoutMs(5000.0);//for long command
   else
	  SetAnswerTimeoutMs(300.0); //for normal command

   ret = SetCommandComPortH(command.c_str(),"\n");
   if (ret != DEVICE_OK)
   {
	    LogMessage(std::string("command write fail"));
	   return ret;
   }
   std::string an;
   if(command.c_str()[0] == 0x18 ){
        CDeviceUtils::SleepMs(600);
	    ret = GetParameters();
	    returnString.assign("ok");
		LogMessage(std::string("Reset!"));
		return DEVICE_OK;
   }
   else if(command.c_str()[0] == '$' || command.c_str()[0] == '?'){

		ret = GetSerialAnswerComPortH(an,"ok\r\n");
	   //ret = comPort->Read(answer,3,charsRead);
	   if (ret != DEVICE_OK)
	   {
		   LogMessage(std::string("answer get error!"));
		  return ret;
	   }
	   returnString.assign(an);
	   return DEVICE_OK;

   }
   else{ 
	   try
	   {

			ret = GetSerialAnswerComPortH(an,"\r\n");
		   //ret = comPort->Read(answer,3,charsRead);
		   if (ret != DEVICE_OK)
		   {
			   LogMessage(std::string("answer get error!_"));
			  return ret;
		   }
		   LogMessage(std::string(an),true);
		   //sample:>>? >><Idle,MPos:0.000,0.000,0.000,WPos:0.000,0.000,0.000>
		   if (an.length() <1)
			  return DEVICE_ERR;
		   returnString.assign(an);
		   if (returnString.find("ok") != std::string::npos)
			   return DEVICE_OK;
		   else
			   return DEVICE_ERR;
	   }
	   catch(...)
	   {
		  LogMessage("Exception in send command!");
		  return DEVICE_ERR;
	   }

   }
}

MM::DeviceDetectionStatus CEVA_NDE_GrblHub::DetectDevice(void)
{
  if (initialized_)
      return MM::CanCommunicate;

   // all conditions must be satisfied...
   MM::DeviceDetectionStatus result = MM::Misconfigured;
   char answerTO[MM::MaxStrLength];
   
   try
   {
      std::string portLowerCase = port_;
      for( std::string::iterator its = portLowerCase.begin(); its != portLowerCase.end(); ++its)
      {
         *its = (char)tolower(*its);
      }
      if( 0< portLowerCase.length() &&  0 != portLowerCase.compare("undefined")  && 0 != portLowerCase.compare("unknown") )
      {
         result = MM::CanNotCommunicate;
         // record the default answer time out
         GetCoreCallback()->GetDeviceProperty(port_.c_str(), "AnswerTimeout", answerTO);

         // device specific default communication parameters
         // for Arduino Duemilanova
         GetCoreCallback()->SetDeviceProperty(port_.c_str(), MM::g_Keyword_Handshaking, "Off");
         GetCoreCallback()->SetDeviceProperty(port_.c_str(), MM::g_Keyword_BaudRate, "9600" );
         GetCoreCallback()->SetDeviceProperty(port_.c_str(), MM::g_Keyword_StopBits, "1");
         // Arduino timed out in GetControllerVersion even if AnswerTimeout  = 300 ms
         GetCoreCallback()->SetDeviceProperty(port_.c_str(), "AnswerTimeout", "500.0");
         GetCoreCallback()->SetDeviceProperty(port_.c_str(), "DelayBetweenCharsMs", "0");
         MM::Device* pS = GetCoreCallback()->GetDevice(this, port_.c_str());
         pS->Initialize();
         // The first second or so after opening the serial port, the Arduino is waiting for firmwareupgrades.  Simply sleep 1 second.
         CDeviceUtils::SleepMs(2000);
         MMThreadGuard myLock(lock_);
         PurgeComPort(port_.c_str());
         int ret = GetStatus();
         // later, Initialize will explicitly check the version #
         if( DEVICE_OK != ret )
         {
            LogMessageCode(ret,true);
         }
         else
         {
            // to succeed must reach here....
            result = MM::CanCommunicate;
         }
         pS->Shutdown();
         // always restore the AnswerTimeout to the default
         GetCoreCallback()->SetDeviceProperty(port_.c_str(), "AnswerTimeout", answerTO);

      }
   }
   catch(...)
   {
      LogMessage("Exception in DetectDevice!",false);
   }

   return result;
}


int CEVA_NDE_GrblHub::Initialize()
{
	if(initialized_)
      return DEVICE_OK;
   // Name
   int ret = CreateProperty(MM::g_Keyword_Name, g_DeviceNameEVA_NDE_GrblHub, MM::String, true);
   if (DEVICE_OK != ret)
      return ret;

   MMThreadGuard myLock(lock_);

   CPropertyAction* pAct = new CPropertyAction(this, &CEVA_NDE_GrblHub::OnVersion);
   std::ostringstream sversion;
   sversion << version_;
   CreateProperty(g_versionProp, sversion.str().c_str(), MM::String, true, pAct);

   pAct = new CPropertyAction(this, &CEVA_NDE_GrblHub::OnCommand);
   ret = CreateProperty("Command","", MM::String, false, pAct);
   if (DEVICE_OK != ret)
      return ret;
   // turn off verbose serial debug messages
   GetCoreCallback()->SetDeviceProperty(port_.c_str(), "Verbose", "0");
   // synchronize all properties
   // --------------------------
   ret = UpdateStatus();
   if (ret != DEVICE_OK)
      return ret;

   initialized_ = true;
   return DEVICE_OK;
}

int CEVA_NDE_GrblHub::DetectInstalledDevices()
{
   if (MM::CanCommunicate == DetectDevice()) 
   {
      std::vector<std::string> peripherals; 
      peripherals.clear();
      peripherals.push_back(g_DeviceNameEVA_NDE_GrblXYStage);
      //peripherals.push_back(g_DeviceNameEVA_NDE_GrblZStage);

      for (size_t i=0; i < peripherals.size(); i++) 
      {
         MM::Device* pDev = ::CreateDevice(peripherals[i].c_str());
         if (pDev) 
         {
            AddInstalledDevice(pDev);
         }
      }
   }

   return DEVICE_OK;
}

int CEVA_NDE_GrblHub::SetAnswerTimeoutMs(double timeout)
{
      if(!portAvailable_)
	   return ERR_NO_PORT_SET;
     GetCoreCallback()->SetDeviceProperty(port_.c_str(), "AnswerTimeout",  CDeviceUtils::ConvertToString(timeout));
   return DEVICE_OK;
}


int CEVA_NDE_GrblHub::Shutdown()
{
   initialized_ = false;

   return DEVICE_OK;
}

int CEVA_NDE_GrblHub::OnPort(MM::PropertyBase* pProp, MM::ActionType pAct)
{
   if (pAct == MM::BeforeGet)
   {
      pProp->Set(port_.c_str());
   }
   else if (pAct == MM::AfterSet)
   {
      pProp->Get(port_);
      portAvailable_ = true;
   }
   return DEVICE_OK;
}
int CEVA_NDE_GrblHub::OnStatus(MM::PropertyBase* pProp, MM::ActionType pAct)
{
   if (pAct == MM::BeforeGet)
   {
	  int ret = GetStatus();
	  if(ret != DEVICE_OK)
		  pProp->Set("-");
	  else
		  pProp->Set(status.c_str());
   }
   return DEVICE_OK;
}
int CEVA_NDE_GrblHub::OnVersion(MM::PropertyBase* pProp, MM::ActionType pAct)
{
   if (pAct == MM::BeforeGet)
   {
	   pProp->Set(version_.c_str());
   }
   return DEVICE_OK;
}

int CEVA_NDE_GrblHub::OnCommand(MM::PropertyBase* pProp, MM::ActionType pAct)
{
   if (pAct == MM::BeforeGet)
   {
	   pProp->Set(commandResult_.c_str());
   }
   else if (pAct == MM::AfterSet)
   {
	   std::string cmd;
      pProp->Get(cmd);
	  if(cmd.compare(commandResult_) ==0)  // command result still there
		  return DEVICE_OK;
	  int ret = SendCommand(cmd,commandResult_);
	  if(DEVICE_OK != ret){
		  commandResult_.assign("Error!");
		  return DEVICE_ERR;
	  }
   }
   return DEVICE_OK;
}
