///////////////////////////////////////////////////////////////////////////////
// FILE:          XYStage.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Thorlabs device adapters: BBD102 Controller
//
// COPYRIGHT:     Thorlabs Inc, 2011
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
//                http://nenad.amodaj.com
//

#ifndef _XYSTAGE_H_
#define _XYSTAGE_H_

#include "../../MMDevice/MMDevice.h"
#include "../../MMDevice/DeviceBase.h"



//////////////////////////////////////////////////////////////////////////////
// XYStage class
// (device adapter)
//////////////////////////////////////////////////////////////////////////////
class ThorlabsStage;

class XYStage : public CXYStageBase<XYStage>
{
public:
   XYStage();
   ~XYStage();

   // Device API
   // ----------
   int Initialize();
   int Shutdown();
  
   void GetName(char* pszName) const;
   bool Busy();

   // XYStage API
   // -----------
   int SetPositionSteps(long x, long y);
   int SetRelativePositionSteps(long x, long y);
   int GetPositionSteps(long& x, long& y);
   int Home();
   int Stop();
   int SetOrigin();
   int GetLimitsUm(double& xMin, double& xMax, double& yMin, double& yMax);
   int GetStepLimits(long& xMin, long& xMax, long& yMin, long& yMax);
   double GetStepSizeXUm();
   double GetStepSizeYUm();
   int IsXYStageSequenceable(bool& isSequenceable) const {isSequenceable = false; return DEVICE_OK;}

   // action interface
   // ----------------
   int OnStepSizeX(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnStepSizeY(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnMaxVelocity(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnAcceleration(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnMoveTimeout(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSyncStep(MM::PropertyBase* pProp, MM::ActionType eAct);   
   int SetPositionUm(double x, double y);
   int SetRelativePositionUm(double dx, double dy);
   int GetPositionUm(double& x, double& y);
private:
   
   enum Axis {X, Y};

   int MoveBlocking(long x, long y, bool relative = false);
   int SetCommand(const unsigned char* command, unsigned cmdLength);
   int GetCommand(unsigned char* answer, unsigned answerLength, double TimeoutMs);

   double syncStep_;
   class CommandThread;

   bool initialized_;            // true if the device is intitalized
   bool home_;                   // true if stage is homed
   double answerTimeoutMs_;      // max wait for the device to answer
   double moveTimeoutMs_;        // max wait for stage to finish moving
  
   enum MOVE_MODE {MOVE, MOVEREL, HOME};
   MOVE_MODE lastMode_;

   std::vector<double> *  parameters_;
   CommandThread* cmdThread_;    // thread used to execute move commands
};

#endif //_XYSTAGE_H_
