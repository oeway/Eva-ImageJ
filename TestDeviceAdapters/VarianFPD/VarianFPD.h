///////////////////////////////////////////////////////////////////////////////
// FILE:          VarianFPD.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Skeleton code for the micro-manager camera adapter. Use it as
//                starting point for writing custom device adapters
//                
// AUTHOR:        Nenad Amodaj, http://nenad.amodaj.com
//                
// COPYRIGHT:     University of California, San Francisco, 2011
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

#ifndef _VarianFPD_H_
#define _VarianFPD_H_

#include "DeviceBase.h"
#include "ImgBuffer.h"
#include "DeviceThreads.h"
#include "ImgBuffer.h"


#include "HcpErrors.h"
#include "HcpFuncDefs.h"
#include "iostatus.h"

//////////////////////////////////////////////////////////////////////////////
// Error codes
//
#define ERR_UNKNOWN_MODE         102

class MySequenceThread;

class VarianFPD : public CCameraBase<VarianFPD>  
{
public:
   VarianFPD();
   ~VarianFPD();
  
   // MMDevice API
   // ------------
   int Initialize();
   int Shutdown();
   int ThreadRun();

   void GetName(char* name) const;      
   
   // VarianFPD API
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
   int PrepareSequenceAcqusition();
   int StartSequenceAcquisition(double interval);
   int StartSequenceAcquisition(long numImages, double interval_ms, bool stopOnOverflow);
   int StopSequenceAcquisition();
   bool IsCapturing();
   int GetBinning() const;
   int SetBinning(int binSize);
   int IsExposureSequenceable(bool& seq) const {seq = false; return DEVICE_OK;}
   int SetCorrections();
   int GetCorrections();
   // action interface
   // ----------------
   int OnVirtualCPLink(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnConfigurationPath(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnBinning(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnPixelType(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnGain(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnAcquisitionType(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnFrameRate(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnExposure(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnAnalogGain(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnCalibrationMode(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnAutoRectify(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnResolution(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnMaxResolution(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnNumModes(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnReceptorType(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnMaxPixelValue(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSystemDescription(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnMode(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnOffsetCorrect(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnGainCorrect(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnDefectCorrect(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnRotate90(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnFlipX(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnFlipY(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnFrameNum4Cal(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnCalibrationProgress(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnOffsetCalibration(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnGainCalibrationStep1(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnGainCalibrationStep2(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnGainCalibrationStep3(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnGainCalibrationStep4(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnResetState(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnResetLink(MM::PropertyBase* pProp, MM::ActionType eAct);
   int onSOD( MM::PropertyBase* pProp, MM::ActionType eAct );
   int onSID( MM::PropertyBase* pProp, MM::ActionType eAct );
   int onTubeVoltage( MM::PropertyBase* pProp, MM::ActionType eAct );
   int onTubeCurrent( MM::PropertyBase* pProp, MM::ActionType eAct );
   int onOperator( MM::PropertyBase* pProp, MM::ActionType eAct );
   int onTag( MM::PropertyBase* pProp, MM::ActionType eAct );
   int OnFrameNum4Acq( MM::PropertyBase* pProp, MM::ActionType eAct );
private:
   int imageWidth_;
   int imageHeight_;
   int maxBitDepth_;
   long imageCounter_;
   MM::MMTime sequenceStartTime_;
   MMThreadLock imgPixelsLock_;

   MMThreadLock* pDemoResourceLock_;
   int nComponents_;
   friend class MySequenceThread;
   MySequenceThread * thd_;

   int binning_;
   int bytesPerPixel_;
   double gain_;
   bool initialized_;
   ImgBuffer img_;
   int roiX_, roiY_;


   std::string mode_;
   std::string acquisitionType_;
   float frameRate_;
   float exposure_;
   float analogGain_;
   std::string calibrationMode_;
   std::string autoRectify_;

   std::string resolution_;
   std::string maxResolution_;
   int numModes_;
   int receptorType_;
   int maxPixelValue_;
   int frameNum4Cal_;
   std::string systemDescription_;
   std::string configurationPath_;
   std::string virtualCPLink_;
   std::string pixelType_;

	std::string  ofstVcpSetting_;
	std::string   gainVcpSetting_;
	std::string   dfctVcpSetting_;
	std::string   rotaVcpSetting_;
	std::string  flpXVcpSetting_;
	std::string  flpYVcpSetting_;

	std::string  calibrationProgress_;
	std::string  OffsetCalibration_;
   std::string gainCalibrationStep1_;
   std::string gainCalibrationStep2_;
   std::string gainCalibrationStep3_;
   std::string gainCalibrationStep4_;

   std::string  resetState_;
   std::string  resetLink_;
      std::string operator_;
	  std::string tubeVoltage_;
	  std::string tubeCurrent_;
	  std::string sid_;
	  std::string sod_;
	  std::string tag_;
   enum modes{hi_fluoro=0,lo_fluoro=1,rad=2};
   enum calibrationmodes{normal=0,offset=1,gain=2};
   enum autorectifies{enable=1,disable=0};

   int ResizeImageBuffer();
   void GenerateImage();
   int InsertImage();
   //hardware related
   int getSysInfo();
   int openLink();
   int closeLink();
   int checkLink();
   int switchMode(std::string modeString);
   int fluoro_acqImage();
   int fluoro_init(int numFrm =0 ,int numBuf =10);
   int rad_init();
   int rad_acqImage();
   int get_image();
   int gain_calibration(int mode,int numCalFrmSet=-1);
   int offset_calibration(int mode,int numCalFrmSet=-1);
   int rad_gain_calibration(int mode,int numCalFrmSet=-1);
   void GenerateSyntheticImage(ImgBuffer& img, double exp);
   double GetNominalPixelSizeUm() const {return nominalPixelSizeUm_;}
   double GetPixelSizeUm() const {return nominalPixelSizeUm_ * GetBinning();}

    std::string getProgress();

    int fluoro_getReady4GainCal(int mode,int numCalFrmSet);
	int fluoro_startFlatFeild4GainCal();
	int fluoro_startDarkFeild4GainCal();
	int fluoro_endGainCal();

	int fluoro_getReady4OffsetCal(int mode,int numCalFrmSet);

	int rad_getReady4OffsetCal(int mode,int numCalFrmSet);

	int rad_getReady4GainCal(int mode,int numCalFrmSet);
	int rad_startDarkFeild4GainCal();
	int rad_startFlatFeild4GainCal();
	int rad_endGainCal();

   int queryProgress();
	UQueryProgInfo crntStatus;
	UQueryProgInfo prevStatus;  // So we can tell when crntStatus changes
	UQueryProgInfo* uqpi;
	SQueryProgInfo qpi;

   SLivePrms*	GLiveParams;
   char GFrameReadyName[256];
   SSysInfo sysInfo;
   SModeInfo modeInfo;
   int crntModeSelect;
   int lastBufIndex;
   double nominalPixelSizeUm_;
   int frameNum4Acq_;



};
class MySequenceThread : public MMDeviceThreadBase
{
	friend class VarianFPD;
	enum { default_numImages=1, default_intervalMS = 100 };
public:
	MySequenceThread(VarianFPD* pCam);
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
	VarianFPD* camera_;                                                     
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
	HANDLE hFrameEvent;
}; 
#endif //_VarianFPD_H_
