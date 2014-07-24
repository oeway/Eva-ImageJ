///////////////////////////////////////////////////////////////////////////////
// FILE:          EVA_NDE_PicoCamera.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   The example implementation of the EVA_NDE_Pico camera.
//                Simulates generic digital camera and associated automated
//                microscope devices and enables testing of the rest of the
//                system without the need to connect to the actual hardware. 
//                
// AUTHOR:        Nenad Amodaj, nenad@amodaj.com, 06/08/2005
//                
//                Karl Hoover (stuff such as programmable CCD size  & the various image processors)
//                Arther Edelstein ( equipment error simulation)
//
// COPYRIGHT:     University of California, San Francisco, 2006
//                100X Imaging Inc, 2008
//
// LICENSE:       This file is distributed under the BSD license.
//                License text is included with the source distribution.
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.
//
// CVS:           $Id: EVA_NDE_PicoCamera.h 10842 2013-04-24 01:21:05Z mark $
//

#ifndef _EVA_NDE_PicoCAMERA_H_
#define _EVA_NDE_PicoCAMERA_H_

#include "../../MMDevice/DeviceBase.h"
#include "../../MMDevice/ImgBuffer.h"
#include "../../MMDevice/DeviceThreads.h"
#include <string>
#include <map>
#include <algorithm>

#include "Pico/PS3000Acon.h"

//////////////////////////////////////////////////////////////////////////////
// Error codes
//
#define ERR_UNKNOWN_MODE         102
#define ERR_IN_SEQUENCE          104
#define ERR_SEQUENCE_INACTIVE    105
#define HUB_NOT_AVAILABLE        107

const char* NoHubError = "Parent Hub not defined.";

////////////////////////
// EVA_NDE_PicoHub
//////////////////////

class EVA_NDE_PicoHub : public HubBase<EVA_NDE_PicoHub>
{
public:
   EVA_NDE_PicoHub():initialized_(false), busy_(false), errorRate_(0.0), divideOneByMe_(1) {} ;
   ~EVA_NDE_PicoHub() {};

   // Device API
   // ---------
   int Initialize();
   int Shutdown() {return DEVICE_OK;};
   void GetName(char* pName) const; 
   bool Busy() { return busy_;} ;

   // HUB api
   int DetectInstalledDevices();
   MM::Device* CreatePeripheralDevice(const char* adapterName);

private:
   void GetPeripheralInventory();

   bool busy_;
   bool initialized_;
   std::vector<std::string> peripherals_;
   double errorRate_;
   long divideOneByMe_;
};


//////////////////////////////////////////////////////////////////////////////
// CEVA_NDE_PicoCamera class
// Simulation of the Camera device
//////////////////////////////////////////////////////////////////////////////

class MySequenceThread;

class CEVA_NDE_PicoCamera : public CCameraBase<CEVA_NDE_PicoCamera>  
{
public:
   CEVA_NDE_PicoCamera();
   ~CEVA_NDE_PicoCamera();
  
   // MMDevice API
   // ------------
   int Initialize();
   int Shutdown();
  
   void GetName(char* name) const;      
   
   // MMCamera API
   // ------------
   int SnapImage();
   const unsigned char* GetImageBuffer();
   unsigned GetImageWidth() const;
   unsigned GetImageHeight() const;
   unsigned GetImageBytesPerPixel() const;
   unsigned GetBitDepth() const;
   long GetImageBufferSize() const;
   double GetExposure() const;
   void SetExposure(double exp);
   int SetROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize); 
   int GetROI(unsigned& x, unsigned& y, unsigned& xSize, unsigned& ySize); 
   int ClearROI();
   int PrepareSequenceAcqusition()
   {
        return DEVICE_OK;
   }
   int StartSequenceAcquisition(double interval);
   int StartSequenceAcquisition(long numImages, double interval_ms, bool stopOnOverflow);
   int StopSequenceAcquisition();
   int InsertImage();
   int ThreadRun(MM::MMTime startTime);
   bool IsCapturing();
   void OnThreadExiting() throw(); 
   double GetNominalPixelSizeUm() const {return nominalPixelSizeUm_;}
   double GetPixelSizeUm() const {return nominalPixelSizeUm_ * GetBinning();}
   int GetBinning() const;
   int SetBinning(int bS);
   int IsExposureSequenceable(bool& isSequenceable) const 
   {
      isSequenceable = isSequenceable_;
      return DEVICE_OK;
   }
   int GetExposureSequenceMaxLength(long& nrEvents) 
   {
      nrEvents = sequenceMaxLength_;
      return DEVICE_OK;
   }
   int StartExposureSequence() 
   {
      // may need thread lock
      sequenceRunning_ = true;
      return DEVICE_OK;
   }
   int StopExposureSequence() 
   {
      // may need thread lock
      sequenceRunning_ = false; 
      sequenceIndex_ = 0;
      return DEVICE_OK;
   }
   // Remove all values in the sequence                                   
   int ClearExposureSequence();
   // Add one value to the sequence                                       
   int AddToExposureSequence(double exposureTime_ms);
   // Signal that we are done sending sequence values so that the adapter can send the whole sequence to the device
   int SendExposureSequence() const {
      return DEVICE_OK;
   }
   void GenerateEmptyImage(ImgBuffer& img);
   unsigned  GetNumberOfComponents() const { return nComponents_;};

   // action interface
   // ----------------
	// floating point read-only properties for testing

	//int OnSwitch(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnBinning(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnTriggerDevice(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnIsSequenceable(MM::PropertyBase* pProp, MM::ActionType eAct);
   //-----oe-------
   int OnSampleOffset(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSampleLength(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnTimebase(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnTimeInterval(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnRowCount(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnInputRange(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnTimeoutMs(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnChannelEnable(MM::PropertyBase* pProp, MM::ActionType eAct);

   int GetChannelName(unsigned /* channel */, char* name);
   unsigned GetNumberOfComponents();
   unsigned GetNumberOfChannels() ;
   int GetComponentName(unsigned channel, char* name);
private:
   int SetAllowedBinning();
   void TestResourceLocking(const bool);
   int ResizeImageBuffer();
   static const double nominalPixelSizeUm_;

   double dPhase_;
   ImgBuffer img_;
   bool busy_;
   bool stopOnOverFlow_;
   bool initialized_;
   long timeout_;
   unsigned roiX_;
   unsigned roiY_;
   MM::MMTime sequenceStartTime_;
   bool isSequenceable_;
   long sequenceMaxLength_;
   bool sequenceRunning_;
   unsigned long sequenceIndex_;
   double GetSequenceExposure();
   std::vector<double> exposureSequence_;
   long imageCounter_;
	long binSize_;
	long image_width;
	long image_height;
   double ccdT_;
	std::string triggerDevice_;

   bool stopOnOverflow_;

   MMThreadLock* pEVA_NDE_PicoResourceLock_;
   MMThreadLock imgPixelsLock_;
   int nComponents_;
   friend class MySequenceThread;
   MySequenceThread * thd_;

	char ch;
	PICO_STATUS status;
	UNIT unit;
	long sampleOffset_;


};

class MySequenceThread : public MMDeviceThreadBase
{
   friend class CEVA_NDE_PicoCamera;
   enum { default_numImages=1, default_intervalMS = 100 };
   public:
      MySequenceThread(CEVA_NDE_PicoCamera* pCam);
      ~MySequenceThread();
      void Stop();
      void Start(long numImages, double intervalMs);
      bool IsStopped();
      void Suspend();
      bool IsSuspended();
      void Resume();
      double GetIntervalMs(){return intervalMs_;}                               
      void SetLength(long images) {numImages_ = images;}                        
      long GetLength() const {return numImages_;}
      long GetImageCounter(){return imageCounter_;}                             
      MM::MMTime GetStartTime(){return startTime_;}                             
      MM::MMTime GetActualDuration(){return actualDuration_;}
   private:                                                                     
      int svc(void) throw();
      CEVA_NDE_PicoCamera* camera_;                                                     
      bool stop_;                                                               
      bool suspend_;                                                            
      long numImages_;                                                          
      long imageCounter_;                                                       
      double intervalMs_;                                                       
      MM::MMTime startTime_;                                                    
      MM::MMTime actualDuration_;                                               
      MM::MMTime lastFrameTime_;                                                
      MMThreadLock stopLock_;                                                   
      MMThreadLock suspendLock_;                                                
}; 

//////////////////////////////////////////////////////////////////////////////
// EVA_NDE_PicoShutter class
// Simulation of shutter device
//////////////////////////////////////////////////////////////////////////////
class EVA_NDE_PicoDA : public CSignalIOBase<EVA_NDE_PicoDA>
{
public:
   EVA_NDE_PicoDA ();
   ~EVA_NDE_PicoDA ();

   int Shutdown() {return DEVICE_OK;}
   void GetName(char* name) const;
   int SetGateOpen(bool open); 
   int GetGateOpen(bool& open);
   int SetSignal(double volts);
   int GetSignal(double& volts);
   int GetLimits(double& minVolts, double& maxVolts)
   {

      minVolts=0.0; maxVolts= 10.0; return DEVICE_OK;
   }
   bool Busy() {return false;}
   int Initialize();

   // Sequence functions
   int IsDASequenceable(bool& isSequenceable) const
   {

      isSequenceable = true;
      return DEVICE_OK;
   }
   int GetDASequenceMaxLength(long& nrEvents) const 
   {

      nrEvents = 256;
      return DEVICE_OK;
   }
   int StartDASequence() const
   {

      (const_cast<EVA_NDE_PicoDA *>(this))->SetSequenceStateOn();
      return DEVICE_OK;
   }
   int StopDASequence() const
   {
   
      (const_cast<EVA_NDE_PicoDA *>(this))->SetSequenceStateOff();
      return DEVICE_OK;
   }
   int SendDASequence();
   int ClearDASequence();
   int AddToDASequence(double voltage);

   int OnTrigger(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnVoltage(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnRealVoltage(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   double volt_;
   double gatedVolts_;
   bool open_;
   std::vector<double> nascentSequence_;
   std::vector<double> sentSequence_;
   unsigned long sequenceIndex_;
   bool sequenceRunning_;

   void SetSequenceStateOn() { sequenceRunning_ = true; }
   void SetSequenceStateOff() { sequenceRunning_ = false; sequenceIndex_ = 0; }

   void SetSentSequence();
};

//////////////////////////////////////////////////////////////////////////////
// MedianFilter class
// apply Median filter an image
// K.H.
//////////////////////////////////////////////////////////////////////////////
class MedianFilter : public CImageProcessorBase<MedianFilter>
{
public:
   MedianFilter () : busy_(false), performanceTiming_(0.),pSmoothedIm_(0), sizeOfSmoothedIm_(0)
   {
      // parent ID display
      CreateHubIDProperty();
   };
   ~MedianFilter () { if(0!=pSmoothedIm_) free(pSmoothedIm_); };

   int Shutdown() {return DEVICE_OK;}
   void GetName(char* name) const {strcpy(name,"MedianFilter");}

   int Initialize();
   bool Busy(void) { return busy_;};

   // NOTE: this utility MODIFIES the argument, make a copy yourself if you want the original data preserved
   template <class U> U FindMedian(std::vector<U>& values ) {
      std::sort(values.begin(), values.end());
      return values[(values.size())>>1];
   };


   template <typename PixelType> int Filter( PixelType* pI, unsigned int width, unsigned int height)
   {
      int ret = DEVICE_OK;
      int x[9];
      int y[9];

      const unsigned long thisSize = sizeof(*pI)*width*height;
      if( thisSize != sizeOfSmoothedIm_)
      {
         if(NULL!=pSmoothedIm_)
         {
            sizeOfSmoothedIm_ = 0;
            free(pSmoothedIm_);
         }
         // malloc is faster than new...
         pSmoothedIm_ = (PixelType*)malloc(thisSize);
         if(NULL!=pSmoothedIm_)
         {
            sizeOfSmoothedIm_ = thisSize;
         }
      }

      PixelType* pSmooth = (PixelType*) pSmoothedIm_;

      if(NULL != pSmooth)
      {
      /*Apply 3x3 median filter to reduce shot noise*/
      for (unsigned int i=0; i<width; i++) {
         for (unsigned int j=0; j<height; j++) {
            x[0]=i-1;
            y[0]=(j-1);
            x[1]=i;
            y[1]=(j-1);
            x[2]=i+1;
            y[2]=(j-1);
            x[3]=i-1;
            y[3]=(j);
            x[4]=i;
            y[4]=(j);
            x[5]=i+1;
            y[5]=(j);
            x[6]=i-1;
            y[6]=(j+1);
            x[7]=i;
            y[7]=(j+1);
            x[8]=i+1;
            y[8]=(j+1);
            // truncate the median filter window  -- duplicate edge points
            // this could be more efficient, we could fill in the interior image [1,w0-1]x[1,h0-1] then explicitly fill in the edge pixels.
            // also the temporary image could be as small as 2 rasters of the image
            for(int ij =0; ij < 9; ++ij)
            {
               if( x[ij] < 0)
                  x[ij] = 0;
               else if( int(width-1) < x[ij])
                  x[ij] = int(width-1);
               if( y[ij] < 0)
                  y[ij] = 0;
               else if( int(height-1) < y[ij])
                  y[ij] = (int)(height-1);
            }
            std::vector<PixelType> windo;
            for(int ij = 0; ij < 9; ++ij)
            {
               windo.push_back(pI[ x[ij] + width*y[ij]]);
            }
            pSmooth[i + j*width] = FindMedian(windo);
         }
      }

      memcpy( pI, pSmoothedIm_, thisSize);
      }
      else
         ret = DEVICE_ERR;

      return ret;
   }
   int Process(unsigned char* buffer, unsigned width, unsigned height, unsigned byteDepth);

   // action interface
   // ----------------
   int OnPerformanceTiming(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   bool busy_;
   MM::MMTime performanceTiming_;
   void*  pSmoothedIm_;
   unsigned long sizeOfSmoothedIm_;
   


};




//////////////////////////////////////////////////////////////////////////////
// EVA_NDE_PicoAutoFocus class
// Simulation of the auto-focusing module
//////////////////////////////////////////////////////////////////////////////
class EVA_NDE_PicoAutoFocus : public CAutoFocusBase<EVA_NDE_PicoAutoFocus>
{
public:
   EVA_NDE_PicoAutoFocus() : 
      running_(false), 
      busy_(false), 
      initialized_(false)  
      {
         CreateHubIDProperty();
      }

   ~EVA_NDE_PicoAutoFocus() {}
      
   // MMDevice API
   bool Busy() {return busy_;}
   void GetName(char* pszName) const;

   int Initialize();
   int Shutdown(){initialized_ = false; return DEVICE_OK;}

   // AutoFocus API
   virtual int SetContinuousFocusing(bool state)
   {

      running_ = state; return DEVICE_OK;
   }
   virtual int GetContinuousFocusing(bool& state)
   {

      state = running_; return DEVICE_OK;
   }
   virtual bool IsContinuousFocusLocked()
   {
      return running_;
   }
   virtual int FullFocus()
   {

      return DEVICE_OK;
   }
   virtual int IncrementalFocus()
   {

      return DEVICE_OK;
   }
   virtual int GetLastFocusScore(double& score)
   {

      score = 0.0;
      return DEVICE_OK;
   }
   virtual int GetCurrentFocusScore(double& score)
   {

      score = 1.0;
      return DEVICE_OK;
   }
   virtual int GetOffset(double& /*offset*/)
   {

      return DEVICE_OK;
   }
   virtual int SetOffset(double /*offset*/)
   {

      return DEVICE_OK;
   }

private:
   bool running_;
   bool busy_;
   bool initialized_;
};





#endif //_EVA_NDE_PicoCAMERA_H_
