 //////////////////////////////////////////////////////////////////////////////
// FILE:          EVA_NDE_Grbl.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Adapter for EVA_NDE_Grbl board
//                Needs accompanying firmware to be installed on the board
// COPYRIGHT:     University of California, San Francisco, 2008
// LICENSE:       LGPL
//
// AUTHOR:        Nico Stuurman, nico@cmp.ucsf.edu, 11/09/2008
//                automatic device detection by Karl Hoover
//
//

#ifndef _EVA_NDE_Grbl_H_
#define _EVA_NDE_Grbl_H_

#include "../../MMDevice/MMDevice.h"
#include "../../MMDevice/DeviceBase.h"
#include <string>
#include <map>

//////////////////////////////////////////////////////////////////////////////
// Error codes
//
#define ERR_UNKNOWN_POSITION 101
#define ERR_INITIALIZE_FAILED 102
#define ERR_WRITE_FAILED 103
#define ERR_CLOSE_FAILED 104
#define ERR_BOARD_NOT_FOUND 105
#define ERR_PORT_OPEN_FAILED 106
#define ERR_COMMUNICATION 107
#define ERR_NO_PORT_SET 108
#define ERR_VERSION_MISMATCH 109

#define PARAMETERS_COUNT 23

extern std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);

class EVA_NDE_GrblInputMonitorThread;

class CEVA_NDE_GrblHub : public HubBase<CEVA_NDE_GrblHub>  
{
public:
   CEVA_NDE_GrblHub();
   ~CEVA_NDE_GrblHub();

   int Initialize();
   int Shutdown();
   void GetName(char* pszName) const;
   bool Busy();

   MM::DeviceDetectionStatus DetectDevice(void);
   int DetectInstalledDevices();

   // property handlers
   int OnPort(MM::PropertyBase* pPropt, MM::ActionType eAct);
   int OnVersion(MM::PropertyBase* pPropt, MM::ActionType eAct);
   int OnStatus(MM::PropertyBase* pProp, MM::ActionType pAct);
   int OnCommand(MM::PropertyBase* pProp, MM::ActionType pAct);
   // custom interface for child devices
   bool IsPortAvailable() {return portAvailable_;}
   bool IsTimedOutputActive() {return timedOutputActive_;}
   void SetTimedOutput(bool active) {timedOutputActive_ = active;}

   int PurgeComPortH() {return PurgeComPort(port_.c_str());}
   int WriteToComPortH(const unsigned char* command, unsigned len) {return WriteToComPort(port_.c_str(), command, len);}
   int ReadFromComPortH(unsigned char* answer, unsigned maxLen, unsigned long& bytesRead)
   {
      return ReadFromComPort(port_.c_str(), answer, maxLen, bytesRead);
   }
   int SetCommandComPortH(const char* command, const char* term)
   {
	   return SendSerialCommand(port_.c_str(),command,term);
   }
    int GetSerialAnswerComPortH (std::string& ans,  const char* term)
	{
		return GetSerialAnswer(port_.c_str(),term,ans);
	}
   static MMThreadLock& GetLock() {return lock_;}

   int SendCommand(std::string command, std::string &returnString);
   int SetAnswerTimeoutMs(double timout);
   int SetSync(int axis, double value );

   int GetParameters();
   int SetParameter(int index, double value);
   std::vector<double> parameters;
   //int Reset();
   double MPos[3];
   double WPos[3];
   int GetStatus(); 
   std::string status;
private:
   MMThreadLock executeLock_;

   std::string commandResult_;
   std::string port_;
   std::string version_;
   bool initialized_;
   bool portAvailable_;
   bool timedOutputActive_;
   static MMThreadLock lock_;

};

#endif //_EVA_NDE_Grbl_H_
