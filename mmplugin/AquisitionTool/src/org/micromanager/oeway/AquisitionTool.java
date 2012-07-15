package org.micromanager.oeway;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.Timer;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import mmcorej.CMMCore;
import mmcorej.StrVector;

import org.micromanager.oeway.AquisitionDialog;
import org.micromanager.MMStudioMainFrame;
import org.micromanager.api.AcquisitionEngine;
import org.micromanager.api.ScriptInterface;
import org.micromanager.utils.NumberUtils;
import org.micromanager.utils.ProgressBar;
import org.micromanager.utils.ReportingUtils;
/**
 *
 * @author oeway
 */
public class AquisitionTool  implements org.micromanager.api.MMPlugin,ActionListener, ChangeListener{
   public static String menuName = "DR采集工具";
   public static String tooltipDescription = "用于采集多帧DR图像";
   public static String CopyRight = "Wei Ouyang";
   public CMMCore core_;
   public MMStudioMainFrame app_;
   public AquisitionDialog dialog_;
   public int currentFrame;
   protected static Timer updateTimer;

    @Override
    public void dispose() {
      if (dialog_ != null) {
         dialog_.setVisible(false);
         dialog_.dispose();
         dialog_ = null;
      }
    }

    @Override
    public void setApp(ScriptInterface si) {
      app_ = (MMStudioMainFrame) si;
      core_ = si.getMMCore();
    }

    @Override
    public void show() {
        String cameraName=core_.getCameraDevice();
        if(!cameraName.equals("VarianFPD"))
        {
	        ReportingUtils.showMessage("Please load VarianFPD as the default camera!");
	        return;
        }
      if (dialog_ == null) {
         dialog_ = new AquisitionDialog();
      	try {
        	 StrVector v = core_.getAllowedPropertyValues("VarianFPD","Mode");
        	 for(int i=0;i<v.size();i++){
        		 dialog_.cobMode.addItem((Object)v.get(i));
        		 dialog_.cobMode.setSelectedItem("RAD");
        	 }
	 		} catch (Exception e) {
	 			// TODO Auto-generated catch block
	 			e.printStackTrace();
	 		}
         addEventHandler();
         dialog_.sldFrameNum.setMaximum(100);
         dialog_.sldFrameNum.setMinimum(1);
         dialog_.sldFrameNum.setValue(10);
         dialog_.setVisible(true);
      } else {
         //dialog_.setPlugin(this);
         dialog_.toFront();
         dialog_.setVisible(true);

      }
        //throw new UnsupportedOperationException("Not supported yet.");
    }

    @Override
    public void configurationChanged() {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    @Override
    public String getDescription() {
      return tooltipDescription;
    }

    @Override
    public String getInfo() {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    @Override
    public String getVersion() {
       return CopyRight;
    }

    @Override
    public String getCopyright() {
        throw new UnsupportedOperationException("Not supported yet.");
    }


    private void addEventHandler(){
        dialog_.btnStart.addActionListener(this);
        dialog_.cobMode.addActionListener(this);
        dialog_.sldFrameNum.addChangeListener(this);
        dialog_.jButton0.addActionListener(this);
        dialog_.jButton1.addActionListener(this);
    }
    
	@Override
	public void actionPerformed(ActionEvent e) {
		// TODO Auto-generated method stub
		if(e.getSource() == dialog_.jButton0){
			  boolean enable =app_.isLiveModeOn();
			  if(!enable){
					try {
						
						core_.setProperty("VarianFPD","Mode","Low-Res Fluoro");
						core_.setProperty("VarianFPD","Exposure","100");
						
						Thread.sleep(50);
					} catch (Exception e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}				  
			  }
				  app_.enableLiveMode(!enable);
	
		}
		else if(e.getSource() == dialog_.jButton1){
			boolean enable =app_.isLiveModeOn();
			if(enable)
			{
				
				app_.enableLiveMode(false);
				try {
					Thread.sleep(50);
				} catch (InterruptedException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
			}
			try {
				core_.setProperty("VarianFPD","Mode","RAD");
				core_.setProperty("VarianFPD","Exposure","1000");
				dialog_.jTextField0.setText("1000");
				
			} catch (Exception e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}
			app_.doSnap();
		}		
		else if (e.getSource()== updateTimer ){
			if(dialog_.btnStart.getText().equals("完成!"))
			{

			     Toolkit.getDefaultToolkit().beep();  

			}
			if(currentFrame >= dialog_.sldFrameNum.getValue() ){
				dialog_.btnStart.setText("完成!");
			}
			else{
				app_.snapAndAddToImage5D();
				currentFrame++;
				dialog_.ProgBar.setValue(currentFrame);
				int le =dialog_.sldFrameNum.getValue();
				dialog_.lblCount.setText( currentFrame + "/" + le);
			}
			try {
				Thread.sleep(20);
			} catch (InterruptedException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}
		}

		else if(e.getSource() == dialog_.btnStart){
			boolean enable =app_.isLiveModeOn();
			if(enable){
				app_.enableLiveMode(!enable);
			}
			if(dialog_.btnStart.getText().equals("完成!")){
				currentFrame =0;
				updateTimer.stop();
				dialog_.btnStart.setText("开始");
			}
			else if(dialog_.btnStart.getText().equals("停止")){
				dialog_.btnStart.setText("开始");
				currentFrame =0;
				updateTimer.stop();
			}else if(dialog_.btnStart.getText().equals("开始"))
			{
				ReportingUtils.showMessage("准备采集!");
				String mod = null;
				try {
					core_.setProperty("VarianFPD","Mode",mod);
				} catch (Exception e2) {
					// TODO Auto-generated catch block
					e2.printStackTrace();
				}
				try {
					core_.setProperty("VarianFPD","Exposure",dialog_.jTextField0.getText());
				} catch (Exception e2) {
					// TODO Auto-generated catch block
					e2.printStackTrace();
				}
				//if(!mod.equals((String)dialog_.cobMode.getSelectedItem()))
				try {
					core_.setProperty("VarianFPD","Mode",(String)dialog_.cobMode.getSelectedItem());
				} catch (Exception e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
				int num = dialog_.sldFrameNum.getValue();
				updateTimer = new Timer(100,this);
				updateTimer.setInitialDelay(100);
				updateTimer.start();
				dialog_.btnStart.setText("停止");
				
			}			
		}
		core_.updateSystemStateCache();
	}
	@Override
	public void stateChanged(ChangeEvent e) {
		// TODO Auto-generated method stub
		if(e.getSource() == dialog_.sldFrameNum){
			int le =dialog_.sldFrameNum.getValue();
			dialog_.lblCount.setText( currentFrame + "/" + le);
			dialog_.ProgBar.setMaximum(le);
		}
	}
    
}
