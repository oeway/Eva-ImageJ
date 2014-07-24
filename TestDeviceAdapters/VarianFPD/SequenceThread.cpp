///////////////////////////////////////////////////////////////////////////////
// FILE:          SequenceThread.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters/VarianFPD
//-----------------------------------------------------------------------------
// DESCRIPTION:   Impelements sequence thread for rendering live video.
//                Part of the skeleton code for the micro-manager camera adapter.
//                Use it as starting point for writing custom device adapters.
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

#include "VarianFPD.h"
#include "../../MMCore/Error.h"


MySequenceThread::MySequenceThread(VarianFPD* pCam)
	:intervalMs_(default_intervalMS)
	,numImages_(default_numImages)
	,imageCounter_(0)
	,stop_(true)
	,suspend_(false)
	,camera_(pCam)
	,startTime_(0)
	,actualDuration_(0)
	,lastFrameTime_(0)
{};

MySequenceThread::~MySequenceThread() {};

void MySequenceThread::Stop() {
	MMThreadGuard(this->stopLock_);
	stop_=true;
}

void MySequenceThread::Start(long numImages, double intervalMs)
{
	MMThreadGuard(this->stopLock_);
	MMThreadGuard(this->suspendLock_);
	numImages_=numImages;
	intervalMs_=intervalMs;
	imageCounter_=0;
	stop_ = false;
	suspend_=false;
	activate();
	actualDuration_ = 0;
	startTime_= camera_->GetCurrentMMTime();
	lastFrameTime_ = 0;
}

bool MySequenceThread::IsStopped(){
	MMThreadGuard(this->stopLock_);
	return stop_;
}

void MySequenceThread::Suspend() {
	MMThreadGuard(this->suspendLock_);
	suspend_ = true;
}

bool MySequenceThread::IsSuspended() {
	MMThreadGuard(this->suspendLock_);
	return suspend_;
}

void MySequenceThread::Resume() {
	MMThreadGuard(this->suspendLock_);
	suspend_ = false;
}

int MySequenceThread::svc(void) throw()
{
	int ret=DEVICE_ERR;
	try 
	{
		do
		{  
			ret=camera_->ThreadRun();
			Sleep(10);
		} while (DEVICE_OK == ret && !IsStopped() && imageCounter_++ < numImages_-1);
		if (IsStopped())
			camera_->LogMessage("SeqAcquisition interrupted by the user\n");

	}catch( CMMError& e){
		camera_->LogMessage(e.getMsg(), false);
		ret = e.getCode();
	}catch(...){
		camera_->LogMessage(g_Msg_EXCEPTION_IN_THREAD, false);
	}
	stop_=true;
	actualDuration_ = camera_->GetCurrentMMTime() - startTime_;
	camera_->OnThreadExiting();
	return ret;
}

