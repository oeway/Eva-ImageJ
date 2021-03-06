///////////////////////////////////////////////////////////////////////////////
//FILE:          LiveModeTimer.java
//PROJECT:       Micro-Manager
//SUBSYSTEM:     mmstudio
//-----------------------------------------------------------------------------
//
// AUTHOR:       Henry Pinkard, henry.pinkard@gmail.com, 2011
//
// COPYRIGHT:    University of California, San Francisco, 2011
//
// LICENSE:      This file is distributed under the BSD license.
//               License text is included with the source distribution.
//
//               This file is distributed in the hope that it will be useful,
//               but WITHOUT ANY WARRANTY; without even the implied warranty
//               of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//               IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//               CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//               INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.
//
package org.micromanager.acquisition;

import java.text.NumberFormat;
import java.util.Timer;
import java.util.TimerTask;
import mmcorej.CMMCore;
import mmcorej.TaggedImage;
import org.json.JSONException;
import org.micromanager.MMStudioMainFrame;
import org.micromanager.utils.ImageUtils;
import org.micromanager.utils.MDUtils;
import org.micromanager.utils.MMScriptException;
import org.micromanager.utils.ReportingUtils;

/**
 * This class extends the java swing timer.  It periodically retrieves images
 * from the core and displays them in the live window
 * @author Henry Pinkard
 */
public class LiveModeTimer {

   private static final String CCHANNELINDEX = "CameraChannelIndex";
   private static final String ACQ_NAME = MMStudioMainFrame.SIMPLE_ACQ;
   private VirtualAcquisitionDisplay win_;
   private CMMCore core_;
   private MMStudioMainFrame gui_;
   private long multiChannelCameraNrCh_;
   private boolean shutterOriginalState_;
   private boolean autoShutterOriginalState_;
   private long fpsTimer_;
   private long fpsCounter_;
   private long imageNumber_;
   private long fpsInterval_ = 5000;
   private final NumberFormat format_;
   private int delay_;
   private boolean running_ = false;
   private Timer timer_;
   private TimerTask task_;
   

   public LiveModeTimer(int delay) {
      delay_ = delay;
      gui_ = MMStudioMainFrame.getInstance();
      core_ = gui_.getCore();
      format_ = NumberFormat.getInstance();
      format_.setMaximumFractionDigits(0x1);
   }

   private void setInterval() {
      double interval = 33;
      try {
         interval = Math.max(core_.getExposure(), 33);
      } catch (Exception e) {
         ReportingUtils.logError("Unable to get exposure from core");
      }
      fpsInterval_ =  (long) (20 *  interval);
      
      delay_ = (int) interval;
   }

   private void setType() {
      multiChannelCameraNrCh_ = core_.getNumberOfCameraChannels();
      if (multiChannelCameraNrCh_ == 1) {
         task_ = singleCameraLiveTask();
      } else {
         task_ = multiCamLiveTask();
      }
   }

   public boolean isRunning() {
      return running_;
   }

   public void begin() throws Exception {
         if(running_) {
            return;
         }
         timer_ = new Timer("Live mode timer");
         
         manageShutter(true);
         core_.clearCircularBuffer();
            
         core_.startContinuousSequenceAcquisition(0);
         setType();
         setInterval();

         // Wait for first image to create ImageWindow, so that we can be sure about image size
         long start = System.currentTimeMillis();
         long now = start;
         long timeout = Math.min(10000, delay_ * 150);
         while (core_.getRemainingImageCount() == 0 && (now - start < timeout) ) {
            now = System.currentTimeMillis();
            Thread.sleep(5);
         }
         if (now - start >= timeout) {
            throw new Exception("Camera did not send image within a reasonable time");
         }
                    
         TaggedImage timg = core_.getLastTaggedImage();

         // With first image acquired, create the display
         gui_.checkSimpleAcquisition();
         win_ = MMStudioMainFrame.getSimpleDisplay();
         
         fpsCounter_ = 0;
         fpsTimer_ = System.currentTimeMillis();
         imageNumber_ = timg.tags.getLong("ImageNumber");

         timer_.schedule(task_, 0, delay_);
         win_.liveModeEnabled(true);
         
         win_.getImagePlus().getWindow().toFront();
         running_ = true;
   }

   
   public void stop() {
      stop(true);
   }
   
   private void stop(boolean firstAttempt) {
     
      timer_.cancel();
      try {
         core_.stopSequenceAcquisition();
         manageShutter(false);
         if (win_ != null) {
            win_.liveModeEnabled(false);
         }
         running_ = false;
      } catch (Exception ex) {
         try {
            manageShutter(false);
         } catch (Exception e) {
            ReportingUtils.showError("Error closing shutter");
         }
         ReportingUtils.showError(ex);
         //Wait 1 s and try to stop again
         if (firstAttempt) {
            final Timer delayStop = new Timer();
            delayStop.schedule( new TimerTask() {
               @Override
               public void run() {
                  stop(false);
               }},1000);   
         }
      } 
   }

   private void updateFPS(long imageNumber) {
      try {
         fpsCounter_++;
         long now = System.currentTimeMillis();
         long diff = now - fpsTimer_;
         if (diff > fpsInterval_) {
            double d = diff/ 1000.0;
            double fps = fpsCounter_ / d;
            double dfps = (imageNumber - imageNumber_) / d;
            ij.IJ.showStatus("fps: " + format_.format(dfps) +
                    ", display fps: " + format_.format(fps));
            fpsCounter_ = 0;
            fpsTimer_ = now;
            imageNumber_ = imageNumber;
         }
      } catch (Exception ex) {
         ReportingUtils.logError(ex);
      }
   }

   private TimerTask singleCameraLiveTask() {
      return new TimerTask() {
         @Override
         public void run() {
            if (core_.getRemainingImageCount() == 0) {
               return;
            }
            if (win_.windowClosed()) //check is user closed window             
            {
               gui_.enableLiveMode(false);
            } else {
               try {
                  TaggedImage ti = core_.getLastTaggedImage();
                  addTags(ti, 0);
                  gui_.addImage(ACQ_NAME, ti, true, true);
                  gui_.updateLineProfile();
                  updateFPS(ti.tags.getLong("ImageNumber"));

               } catch (MMScriptException ex) {
                  ReportingUtils.showError(ex);
                  gui_.enableLiveMode(false);
               } catch (JSONException exc) {
                  ReportingUtils.showError("Problem with image tags");
                  gui_.enableLiveMode(false);
               } catch (Exception excp) {
                  ReportingUtils.showError("Couldn't get tagged image from core");
                  gui_.enableLiveMode(false);
               }
            }
         }
      };
   }

   private TimerTask multiCamLiveTask() {
      return new TimerTask() {
         @Override
         public void run() {
            if (core_.getRemainingImageCount() == 0) {
               return;
            }
            if (win_.windowClosed() || !gui_.acquisitionExists(MMStudioMainFrame.SIMPLE_ACQ)) {
               gui_.enableLiveMode(false);  //disable live if user closed window
            } else {
               try {
                  TaggedImage[] images = new TaggedImage[(int) multiChannelCameraNrCh_];
                  String camera = core_.getCameraDevice();

                  TaggedImage ti = core_.getLastTaggedImage();

                  int channel = ti.tags.getInt(camera + "-" + CCHANNELINDEX);
                  images[channel] = ti;
                  int numFound = 1;
                  int index = 1;
                  while (numFound < images.length && index <= 2 * images.length) {      //play with this number
                     try {
                        ti = core_.getNBeforeLastTaggedImage(index);
                     } catch (Exception ex) {
                        break;
                     }
                     channel = ti.tags.getInt(camera + "-" + CCHANNELINDEX);
                     if (images[channel] == null) {
                        numFound++;
                     }
                     images[channel] = ti;
                     index++;
                  }

                  if (numFound == images.length) {
                     for (channel = 0; channel < images.length; channel++) {
                        ti = images[channel];
                        ti.tags.put("Channel", core_.getCameraChannelName(channel));
                        addTags(ti, channel);
                     }
                     int lastChannelToAdd = win_.getHyperImage().getChannel() - 1;
                     for (int i = 0; i < images.length; i++) {
                        if (i != lastChannelToAdd) {
                           gui_.addImage(MMStudioMainFrame.SIMPLE_ACQ, images[i], false, true);
                        }
                     }
                     gui_.addImage(MMStudioMainFrame.SIMPLE_ACQ, images[lastChannelToAdd], true, true);
                     gui_.updateLineProfile();
                     updateFPS(ti.tags.getLong("ImageNumber"));
                  }

               } catch (MMScriptException ex) {
                  gui_.enableLiveMode(false);
                  ReportingUtils.showError(ex);
               } catch (JSONException exc) {
                  gui_.enableLiveMode(false);
                  ReportingUtils.showError("Problem with image tags");
               } catch (Exception excp) {
                  gui_.enableLiveMode(false);
                  ReportingUtils.showError("Couldn't get tagged image from core");
               }
            }
         }
      };
   }

   private void addTags(TaggedImage ti, int channel) throws JSONException {
      MDUtils.setChannelIndex(ti.tags, channel);
      MDUtils.setFrameIndex(ti.tags, 0);
      MDUtils.setPositionIndex(ti.tags, 0);
      MDUtils.setSliceIndex(ti.tags, 0);
      try {
         ti.tags.put("Summary", MMStudioMainFrame.getInstance().getAcquisition(ACQ_NAME).getSummaryMetadata());
      } catch (MMScriptException ex) {
         ReportingUtils.logError("Error adding summary metadata to tags");
      }
      gui_.addStagePositionToTags(ti);
   }
   
   private TaggedImage makeTaggedImage(Object pixels) throws JSONException, MMScriptException {
       TaggedImage ti = ImageUtils.makeTaggedImage(pixels,
                    0, 0, 0, 0,
                    gui_.getAcquisitionImageWidth(ACQ_NAME),
                    gui_.getAcquisitionImageHeight(ACQ_NAME),
                    gui_.getAcquisitionImageByteDepth(ACQ_NAME));
      try {
         ti.tags.put("Summary", gui_.getAcquisition(ACQ_NAME).getSummaryMetadata());

      } catch (MMScriptException ex) {
         ReportingUtils.logError("Error adding summary metadata to tags");
      }
      gui_.addStagePositionToTags(ti);
      return ti;
   }
   

   private void manageShutter(boolean enable) throws Exception {
      String shutterLabel = core_.getShutterDevice();
      if (shutterLabel.length() > 0) {
         if (enable) {
            shutterOriginalState_ = core_.getShutterOpen();
            autoShutterOriginalState_ = core_.getAutoShutter();
            core_.setAutoShutter(false);
            core_.setShutterOpen( shutterOriginalState_ || autoShutterOriginalState_);
         } else {
            core_.setShutterOpen(shutterOriginalState_);
            core_.setAutoShutter(autoShutterOriginalState_);
         }
      }
   }
}
