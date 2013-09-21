package plugins.oeway;
import plugins.tprovoost.Microscopy.MicroManagerForIcy.MicromanagerPlugin;
import plugins.tprovoost.Microscopy.MicroManagerForIcy.MicroscopeCore;
import plugins.tprovoost.Microscopy.MicroManagerForIcy.MicroscopeSequence;
import plugins.tprovoost.Microscopy.MicroManagerForIcy.Tools.ImageGetter;
import icy.file.Saver;
import icy.gui.frame.progress.AnnounceFrame;
import icy.image.IcyBufferedImage;
import icy.main.Icy;
import icy.plugin.PluginDescriptor;
import icy.plugin.PluginLauncher;
import icy.plugin.PluginLoader;
import icy.roi.ROI2D;
import icy.sequence.Sequence;
import icy.system.thread.ThreadUtil;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.nio.channels.FileChannel;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashMap;

import javax.swing.JButton;

import loci.formats.ome.OMEXMLMetadataImpl;

import plugins.adufour.ezplug.*;
import icy.gui.viewer.Viewer;

/**
 * Plugin for 
 * 
 * @author Wei Ouyang
 * 
 */
public class EvaScanner extends EzPlug implements EzStoppable, ActionListener,EzVarListener<File>
{
	

	
    /** CoreSingleton instance */
    MicroscopeCore core;
    Sequence currentSequence = null;
	EzButton 					markPos;
	EzButton 					getPos;	
	EzButton 					homing;	
	EzButton 					reset;		
	EzButton 					gotoPostion;
	EzButton 					runBundlebox;
	
	
	EzVarDouble					posX;
	EzVarDouble					posY;	
	EzButton 					generatePath;
	EzVarDouble					stepSize;
	EzVarSequence 				scanMapSeq;
	
	
	EzVarDouble					scanSpeed;
	EzVarFile					pathFile;
	EzVarText					note;
	EzVarFolder					targetFolder;

	String lastSeqName = "";
	String currentSeqName = "";	
	String xyStageLabel ="";
	String xyStageParentLabel ="";
	
	String picoCameraLabel ="";
	String picoCameraParentLabel ="";		
	long rowCount =100;
	long frameCount = 1;
	
	// some other data
	boolean						stopFlag;
	void startMicroManagerForIcy()
	{
		try {
	        // we get all the PluginDescriptor from the PluginManager
	        for (final PluginDescriptor pluginDescriptor : PluginLoader.getPlugins())
	        {
	        	//System.out.println(pluginDescriptor.getSimpleClassName()); 
	            // This part of the example check for a match in the name of the class
	            if (pluginDescriptor.isInstanceOf(MicromanagerPlugin.class))
	            {
	                // Create a new Runnable which contain the proper launcher
	                ThreadUtil.invokeLater(new Runnable()
	                {
	                    @Override
	                    public void run()
	                    {
	                        PluginLauncher.start(pluginDescriptor);
	                    }
	                });
	            }
	        }
		}catch (Exception e1) {
			new AnnounceFrame("Error When Start MicroManagerForIcy!",5);
			System.out.println("MicroManagerForIcy start failed...");

		} 
		
	}
	
	@Override
	protected void initialize()
	{
		startMicroManagerForIcy();
		// 1) variables must be initialized
		scanMapSeq = new EzVarSequence("Scan Map Sequence");
		stepSize = new EzVarDouble("Step Size");
		markPos = new EzButton("Mark Position", this);
		gotoPostion = new EzButton("Goto Position", this);
		reset = new EzButton("Reset", this);
		homing = new EzButton("Homing", this);
		generatePath = new EzButton("Generate Path", this);
		runBundlebox = new EzButton("Run Bundle Box",this);
		getPos = new EzButton("Get Position", this);		
		posX = new EzVarDouble("X");
		posY = new EzVarDouble("Y");		
		
		scanSpeed = new EzVarDouble("Scan Speed");
		pathFile = new EzVarFile("Path File", null);
		
		note = new EzVarText("Scan Note", new String[] { "Test" }, 0, true);
		
		targetFolder = new EzVarFolder("Target Folder", null);
		targetFolder.addVarChangeListener(this);
		
		// 2) and added to the interface in the desired order
		
		// let's group other variables per type
		stepSize.setValue(1.0);
		scanSpeed.setValue(1000.0);
		
		EzGroup groupInit= new EzGroup("Initialization", homing,reset);
		super.addEzComponent(groupInit);			
		
		EzGroup groupMark = new EzGroup("Mark", getPos,posX,posY,gotoPostion,markPos);
		super.addEzComponent(groupMark);	
		
		EzGroup groupSettings = new EzGroup("Settings",targetFolder,note); //TODO:add targetFolder
		super.addEzComponent(groupSettings);
		
		EzGroup groupScanMap = new EzGroup("Scan Map",scanMapSeq,runBundlebox,stepSize, scanSpeed,generatePath);
		super.addEzComponent(groupScanMap);	
		
		core = MicroscopeCore.getCore();

		
	}	
	protected boolean waitUntilComplete()
	{
		if(xyStageLabel.equals("")){
			 xyStageLabel = core.getXYStageDevice();
	    try {
		   xyStageParentLabel = core.getParentLabel(xyStageLabel);
		} catch (Exception e1) {
			new AnnounceFrame("XY Stage Error!",5);
			System.out.println("XY Stage Error...");
			return false;
		} 
		}
  		String status = "";
  		//wait until movement complete
  		System.out.println("waiting for the stage...");
  		int retry= 0;
  		while(!stopFlag){
  			try {
				status = core.getProperty(xyStageParentLabel, "Status");
			} catch (Exception e) {
				if(retry++<100)
				{
					e.printStackTrace();
					try{
						  Thread.currentThread();
						Thread.sleep(100);//sleep for 1000 ms
						  
						}
						catch(Exception ie){
						
						}
					
				}
				else
				return false;
			}
  			if(status.equals("Idle"))
  				break;
  			else if(!status.equals("Run"))
  			{
  				System.out.println("status error:"+status);
  				break;
  			}
  			Thread.yield();
  		}
  		if(!status.equals( "Idle") && !stopFlag) // may be error occured
  		{
  			System.out.println("Stage error");
  			new AnnounceFrame("XY stage status error!",5);
  			return false;
  		}
  		System.out.println("stage ok!");
  		return true;
		
	}	
	protected boolean createAndAdd(IcyBufferedImage capturedImage)
	{
		try
		{
			if(currentSequence !=null){
				try {
					if(targetFolder.getValue() != null){
						File f = new File(targetFolder.getValue(),currentSequence.getName()+".tiff");
						Saver.save(currentSequence,f,false,true);
						new AnnounceFrame(currentSequence.getName() + " saved!",10);
					}
				} catch (Exception e) {
					e.printStackTrace();
					new AnnounceFrame("File haven't save!",10);
				}
				try {
					copyFile(pathFile.getValue(),targetFolder.getValue(),currentSequence.getName()+"_gcode.txt");
				} catch (Exception e) {
					e.printStackTrace();
					new AnnounceFrame("Gcode can not be copied!",10);
				}
			    //Close the input stream
			    currentSequence = null;				
			}

	        MicroscopeSequence s = new MicroscopeSequence(capturedImage);
	        Calendar calendar = Calendar.getInstance();
	        Icy.getMainInterface().addSequence(s);
	        s.setName(currentSeqName + "__" + calendar.get(Calendar.MONTH) + "_" + calendar.get(Calendar.DAY_OF_MONTH) + "_"
	                + calendar.get(Calendar.YEAR) + "-" + calendar.get(Calendar.HOUR_OF_DAY) + "_"
	                + calendar.get(Calendar.MINUTE) + "_" + calendar.get(Calendar.SECOND));
	        currentSequence = s;
	        lastSeqName = currentSeqName;
	        
	        currentSequence.setTimeInterval(1e-12); //1G Hz Sample Rate
	        currentSequence.setPixelSizeX(stepSize.getValue()*1000.0);
	        currentSequence.setPixelSizeY(stepSize.getValue()*1000.0);
	        currentSequence.setPixelSizeZ(stepSize.getValue()*1000.0);
	
	        
	        OMEXMLMetadataImpl md = currentSequence.getMetadata();
	        md.setImageDescription(note.getValue(), 0);
	        new AnnounceFrame("New Sequence created:"+currentSeqName,5);

		}
		catch(Exception e)
		{
			new AnnounceFrame("Error when create new sequence!",20);
			return false;
		}
        return true;
	}
	protected boolean snap2Sequence()
	{
        IcyBufferedImage capturedImage;
        if (core.isSequenceRunning())
        {
        	new AnnounceFrame("Sequence is running, close it before start!",10);
        	return false;//capturedImage = ImageGetter.getImageFromLive(core);
        }
        else
            capturedImage = ImageGetter.snapImage(core);
        
        if (capturedImage == null)
        {
            new AnnounceFrame("No image was captured",30);
            return false;
        }
        
        if(!lastSeqName.equals( currentSeqName) || currentSequence == null )
        {
        	createAndAdd(capturedImage);
        }
        else
        {
            try
            {
            	currentSequence.addImage(((Viewer) currentSequence.getViewers().get(0)).getT(), capturedImage);
            }
            catch (IllegalArgumentException e)
            {
                String toAdd = "";
                if (currentSequence.getSizeC() > 0)
                    toAdd = toAdd
                            + ": impossible to capture images with a colored sequence. Only Snap C are possible.";
                new AnnounceFrame("This sequence is not compatible" + toAdd,30);
                return false;
            }
            catch(IndexOutOfBoundsException e2)
            {
            	createAndAdd(capturedImage);
            	new AnnounceFrame("IndexOutOfBoundsException,create new sequence instead!",5);
            }
        }
        return true;
		
	}
	@Override
	protected void execute()
	{
		core = MicroscopeCore.getCore();
        if (core.isSequenceRunning())
        {
        	new AnnounceFrame("Sequence is running, close it before start!",10);
        	return ;//capturedImage = ImageGetter.getImageFromLive(core);
        }
		if(targetFolder.getValue() == null){
			stopFlag = true;
			new AnnounceFrame("Please select a target folder to store data!",5);
			return;
		}
		long cpt = 0;
		stopFlag = false;
		lastSeqName = "";
		// main plugin code goes here, and runs in a separate thread
		if(pathFile.getValue() == null){
			new AnnounceFrame("Please select a path file!",5);
			return;
		}
		
	   xyStageLabel = core.getXYStageDevice();
	   
	   try {
		   xyStageParentLabel = core.getParentLabel(xyStageLabel);
		} catch (Exception e1) {
			new AnnounceFrame("Please select 'EVA_NDE_Grbl' as the default XY Stage!",5);
			return;
		} 
	   
		try {
			if(!core.hasProperty(xyStageParentLabel,"Command"))
			  {
				  new AnnounceFrame("Please select 'EVA_NDE_Grbl' as the default XY Stage!",5);
				  return;
			  }
		} catch (Exception e1) {
			  new AnnounceFrame("XY Stage Error!",5);
			  return;
		}
		
		picoCameraLabel = core.getCameraDevice();
	   try {
		   picoCameraParentLabel = core.getParentLabel(picoCameraLabel);
		} catch (Exception e1) {
			new AnnounceFrame("Please select 'picoCam' as the default camera device!",5);
			return;
		} 
	   
		try {
			if(!core.hasProperty(picoCameraLabel,"RowCount"))
			  {
				new AnnounceFrame("Please select 'picoCam' as the default camera device!",5);
				  return;
			  }
		} catch (Exception e1) {
			  new AnnounceFrame("Camera Error!",5);
			  return;
		}		
		
//		if(targetFolder.getValue() == null){
//			new AnnounceFrame("Please select a target folder to store data!");
//			return;
//		}
	
		String oldRowCount="1";
		try {
			oldRowCount = core.getProperty(picoCameraLabel, "RowCount");
		} catch (Exception e1) {
			  new AnnounceFrame("Camera Error!",5);
			  return;
		}
		System.out.println(scanSpeed.name + " = " + scanSpeed.getValue());
		System.out.println(pathFile.name + " = " + pathFile.getValue());
		System.out.println(targetFolder.name + " = " + targetFolder.getValue());
		System.out.println(note.name + " = " + note.getValue());


		try{
		  // Open the file that is the first 
		  // command line parameter
		  FileInputStream fstream = new FileInputStream(pathFile.getValue());
		  // Get the object of DataInputStream
		  DataInputStream in = new DataInputStream(fstream);
		  BufferedReader br = new BufferedReader(new InputStreamReader(in));
		  String strLine;
		  //Read File Line By Line
		  HashMap<String , String> settings = new HashMap<String , String>();  
		  

		  int maxRetryCount = 3;
		  String lastG00="";
		  super.getUI().setProgressBarMessage("Action...");
		  while ((strLine = br.readLine()) != null && !stopFlag) {
			  	// Print the content on the console
			  	strLine = strLine.trim();
			  	System.out.println (strLine);
			  	if(strLine.startsWith("(") && strLine.endsWith(")") && strLine.contains("=")){  //comment
			  		strLine = strLine.replace("(", ""); 
			  		strLine = strLine.replace(")", "");
			  		String tmp[] = strLine.split("=");
			  		tmp[0] = tmp[0].trim().toLowerCase();
			  		tmp[1] = tmp[1].trim().toLowerCase();
			  		settings.put(tmp[0],tmp[1]);
			  		
			  	   try{
				  		if(tmp[0].equals("newsequence") ){
				  			currentSeqName = tmp[1];
				  			cpt =0;
				  			
				  		}
				  		else if(tmp[0].equals("width")){
				  			rowCount = Integer.parseInt(tmp[1]);
				  			core.setProperty(picoCameraLabel, "RowCount",tmp[1]);
				  		}
				  		else if(tmp[0].equals("height")){
				  			frameCount = Integer.parseInt(tmp[1]);
				  		}
				  		else if(tmp[0].equals("sampleoffset")){
				  			core.setProperty(picoCameraLabel, "SampleOffset",tmp[1]);
				  		}
				  		else if(tmp[0].equals("samplelength")){
				  			core.setProperty(picoCameraLabel, "SampleLength",tmp[1]);
				  		}
				  		else if(tmp[0].equals("reset")){
				  			if(tmp[1].equals("1"))
				  			{
				  				String a = String.valueOf(Character.toChars(18));
				  				core.setProperty(xyStageParentLabel, "Command",a);
				  			}
				  		}
				  		else{
				  			new AnnounceFrame(tmp[0]+":"+tmp[1],5);
				  		}
			  		}
					catch (Exception e){//Catch exception if any
						new AnnounceFrame("Error when parsing line:"+strLine);
					}
			  		
			  	}
			  	else if (strLine.startsWith("G01")){
			  		boolean success = false;
			  		int retryCount = 0;

			  		while(retryCount<maxRetryCount && !success){
			  			core.setProperty(xyStageParentLabel, "Command",strLine);			  			
			  			retryCount++;
			  			try
			  			{
			  				System.out.println("snapping");
					  		//excute command
					  		if(snap2Sequence())
					  			success = true;
					  		else
					  			success = false;

			  			}
			  			catch(Exception e4)
			  			{
			  				new AnnounceFrame("Error when snapping image!",5);
			  				System.out.println("error when snape image:");
			  				e4.printStackTrace();
			  			}
				  		if(!waitUntilComplete())
				  			success = false;
				  		    
				  		if(success)
				  			break;
				  		
				  		//if not success, then redo
				  		core.setProperty(xyStageParentLabel, "Command",lastG00);
				  		if(! waitUntilComplete()){
				  			super.getUI().setProgressBarMessage("error!");
							  try {
									core.setProperty(picoCameraLabel, "RowCount",oldRowCount);
								} catch (Exception e1) {
									// TODO Auto-generated catch block
									e1.printStackTrace();
								}
							  System.out.println("Error when waiting for the stage to complete");
				  			break;
				  		}
			  			
			  		}
			  		if(!success){
			  			new AnnounceFrame("Error when snapping image!");
			  			break; //exit current progress!
			  		}
			  		cpt++;
			  		super.getUI().setProgressBarValue((double)cpt/frameCount);
			  		super.getUI().setProgressBarMessage(Long.toString(cpt)+"/"+ Long.toString(frameCount));
			  	}
			  				  	
			  	else{
			  	     if (strLine.startsWith("G00"))
			   		lastG00 = strLine;		
			  	     try
			  	     {
			  	    	 core.setProperty(xyStageParentLabel, "Command",strLine);
			  	     }
			  		catch (Exception e)//Catch exception if any
			  			{
			  				e.printStackTrace();
			  			}
			  		
			  		if(! waitUntilComplete()){
			  			super.getUI().setProgressBarMessage("error!");
						  try {
								core.setProperty(picoCameraLabel, "RowCount",oldRowCount);
							} catch (Exception e1) {
								// TODO Auto-generated catch block
								e1.printStackTrace();
							}
			  			break;
			  		}
			  	}
			  }
		  lastG00 ="";
		  in.close();

		}
		catch (Exception e){//Catch exception if any
			  super.getUI().setProgressBarMessage("error!");
			  System.err.println("Error: " );
			  e.printStackTrace();
		}
		finally{
			if(currentSequence !=null){
				try {
					if(targetFolder.getValue() != null){
						File f = new File(targetFolder.getValue(),currentSequence.getName()+".tiff");
						Saver.save(currentSequence,f,false);
						new AnnounceFrame(currentSequence.getName() + " saved!",10);
					}
				} catch (Exception e) {
					e.printStackTrace();
					new AnnounceFrame("File haven't save!",10);
				}
				try {
					copyFile(pathFile.getValue(),targetFolder.getValue(),currentSequence.getName()+"_gcode.txt");
				} catch (Exception e) {
					e.printStackTrace();
					new AnnounceFrame("Gcode can not be copied!",10);
				}
				//Close the input stream
			    currentSequence = null;				
			}


			try {
				core.setProperty(picoCameraLabel, "RowCount",oldRowCount);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		new AnnounceFrame("Task Over!",20);

	}
	
	@Override
	public void clean()
	{
		// use this method to clean local variables or input streams (if any) to avoid memory leaks
		
	}
	
	@Override
	public void stopExecution()
	{
		// this method is from the EzStoppable interface
		// if this interface is implemented, a "stop" button is displayed
		// and this method is called when the user hits the "stop" button
		stopFlag = true;
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		core = MicroscopeCore.getCore();
		if (((JButton)e.getSource()).getText().equals(markPos.name)) {
					
		   xyStageLabel = core.getXYStageDevice();
			try {
				double[] x_stage = {0.0};
				double[] y_stage = {0.0};
				core.getXYPosition(xyStageLabel, x_stage, y_stage);
				posX.setValue(x_stage[0]/1000.0);
				posY.setValue(y_stage[0]/1000.0);
				if(scanMapSeq != null)
				{
					IcyBufferedImage image = scanMapSeq.getValue().getImage( 0 , 0, 0 );
					image.setData((int)x_stage[0]/1000,(int)y_stage[0]/1000,0,100);
				}
				else
				{
					  new AnnounceFrame("No sequence selected!",10);
					  return;
				}
			} catch (Exception e1) {
				  new AnnounceFrame("Marking on sequence failed!",10);
				  e1.printStackTrace();
				  return;
			}
			
		}
		else if (((JButton)e.getSource()).getText().equals(gotoPostion.name)) {
			 
				try {
					xyStageLabel = core.getXYStageDevice();
					core.setXYPosition(xyStageLabel, posX.getValue()*1000.0, posY.getValue()*1000.0);
					//new AnnounceFrame("Goto position...!",5);
				} catch (Exception e1) {
					  new AnnounceFrame("Goto position failed!",10);
					  return;
				}
		}
		else if (((JButton)e.getSource()).getText().equals(getPos.name)) {
			   xyStageLabel = core.getXYStageDevice();
				try {
					double[] x_stage = {0.0};
					double[] y_stage = {0.0};
					core.getXYPosition(xyStageLabel, x_stage, y_stage);
					posX.setValue(x_stage[0]/1000.0);
					posY.setValue(y_stage[0]/1000.0);
				} catch (Exception e1) {
					  new AnnounceFrame("Get position failed!",10);
					  return;
				}
		}
		else if (((JButton)e.getSource()).getText().equals(runBundlebox.name)) {
		    	System.out.println("Run bundle box ...");
		    	class MyRunner implements Runnable{
		    		  public void run(){
					      try {
								if(scanMapSeq != null)
								{
									   xyStageLabel = core.getXYStageDevice();
									   
									   try {
										   xyStageParentLabel = core.getParentLabel(xyStageLabel);
										} catch (Exception e1) {
											new AnnounceFrame("Please select 'EVA_NDE_Grbl' as the default XY Stage!",20);
											return;
										} 
									   
										try {
											if(!core.hasProperty(xyStageParentLabel,"Command"))
											  {
												  new AnnounceFrame("Please select 'EVA_NDE_Grbl' as the default XY Stage!",20);
												  return;
											  }
										} catch (Exception e1) {
											  new AnnounceFrame("XY Stage Error!",10);
											  return;
										}
										
			
									
									ArrayList<ROI2D> rois;	
								
									try
									{
										rois= scanMapSeq.getValue().getROI2Ds();
									}
									catch (Exception e1)
									{
										new AnnounceFrame("Please add at least one 2d roi in the scan map sequence!",10);
										return;	
									}
									  if(rois.size()<=0)
									    {
									    	  new AnnounceFrame("No roi found!",5);
											  return;
									    }
								    core.setProperty(xyStageParentLabel, "Command","G90");
									for(int i=0;i<rois.size();i++) 
									{
										ROI2D roi = rois.get(i);
										double x0 = roi.getBounds().getMinX();
										double y0 = roi.getBounds().getMinY();
										double x1 = roi.getBounds().getMaxX();
										double y1 = roi.getBounds().getMaxY();
										
										
										core.setProperty(xyStageParentLabel, "Command","G00 X" + Double.toString(x0)+" Y" + Double.toString(y0));
										
										core.setProperty(xyStageParentLabel, "Command","G00 X" + Double.toString(x1)+" Y" + Double.toString(y0));
										
										core.setProperty(xyStageParentLabel, "Command","G00 X" + Double.toString(x1)+" Y" + Double.toString(y1));
										
										core.setProperty(xyStageParentLabel, "Command","G00 X" + Double.toString(x0)+" Y" + Double.toString(y1));
										
										core.setProperty(xyStageParentLabel, "Command","G00 X" + Double.toString(x0)+" Y" + Double.toString(y0));
										waitUntilComplete();
									}	
				
			
								}
								 new AnnounceFrame("Bundle box complete!",5);
							} catch (Exception e1) {
								  new AnnounceFrame("Error when run bundle box!",10);
								  return;
							}
		    		  }
		    		}
		    	     MyRunner myRunner = new MyRunner(); 
		    	     Thread myThread = new Thread(myRunner);
		    	   
		    	     myThread.start();
		    	    
		    	    	
		}
		else if (((JButton)e.getSource()).getText().equals(reset.name)) {	
	    	try {
	    		 xyStageLabel = core.getXYStageDevice();
				   try {
					   xyStageParentLabel = core.getParentLabel(xyStageLabel);
					} catch (Exception e1) {
						new AnnounceFrame("Please select 'EVA_NDE_Grbl' as the default XY Stage!",20);
						return;
					} 
				core.setProperty(xyStageParentLabel, "Command",String.valueOf((char)0x18));
			} catch (Exception e1) {
				 new AnnounceFrame("Reset failed!",10);
			}
	    }
	    else if (((JButton)e.getSource()).getText().equals(homing.name)) {	
	    	
	    	class MyRunner implements Runnable{
	    		  public void run(){
		    		    	try {
		    		    		 xyStageLabel = core.getXYStageDevice();
		    					   try {
		    						   xyStageParentLabel = core.getParentLabel(xyStageLabel);
		    						} catch (Exception e1) {
		    							new AnnounceFrame("Please select 'EVA_NDE_Grbl' as the default XY Stage!",20);
		    							return;
		    						} 
		    					core.setProperty(xyStageParentLabel, "Command","$H");
		    					new AnnounceFrame("Homing completed!",5);
		    				} catch (Exception e1) {
		    					 new AnnounceFrame("Homing error,try to restart controller!",10);
		    				}
	    		  		}
	    		}
	    	     MyRunner myRunner = new MyRunner(); 
	    	     Thread myThread = new Thread(myRunner);
	    	     myThread.start();

	    }
	    else if (((JButton)e.getSource()).getText().equals(generatePath.name)) {		
			System.out.println("Generate Path ...");
			
			try {
				if(scanMapSeq != null)
				{
					if(stepSize.getValue()<=0.0)
					{
						  new AnnounceFrame("Step size error!",20);
						  return;
					}
					
					ArrayList<ROI2D> rois;	
					PrintWriter pw ;
					try
					{
						rois= scanMapSeq.getValue().getROI2Ds();
					}
					catch (Exception e1)
					{
						new AnnounceFrame("Please add at least one 2d roi in the scan map sequence!",5);
						return;	
					}
					  if(rois.size()<=0)
					    {
					    	  new AnnounceFrame("No roi found!",5);
							  return;
					    }
					if(pathFile.getValue() != null)
					{
						pw = new PrintWriter(new FileWriter(pathFile.getValue()));
					}
					else
					{
						  new AnnounceFrame("Please select the 'Target Folder'!",5);
						  return;
					}
				  
					for(int i=0;i<rois.size();i++) 
					{
						ROI2D roi = rois.get(i);
						double x0 = roi.getBounds().getMinX();
						double y0 = roi.getBounds().getMinY();
						double x1 = roi.getBounds().getMaxX();
						double y1 = roi.getBounds().getMaxY();
						//for(double a=x0;a<=x1;a+=stepSize.getValue())
						pw.printf("(newSequence=%s-%d)\n",roi.getName(),i);
						pw.printf("(location=%d,%d)\n",(int)x0,(int)y0);
						pw.printf("(width=%d)\n",(int)((double)(x1-x0)/stepSize.getValue()-0.5));	
						pw.printf("(height=%d)\n",(int)((double)(y1-y0)/stepSize.getValue()+0.5));	
						pw.printf("(reset=1)\n");	
						pw.printf("G90\n");		
						pw.printf("M108 P%f Q%d\n",stepSize.getValue(),0);
						
						for(double b=y0;b<=y1;b+=stepSize.getValue())	
						{
							pw.printf("G00 X%f Y%f\n",x0,b);
							pw.printf("G01 X%f Y%f F%f\n",x1,b,scanSpeed.getValue());
						}
					}	
					//pw.printf("G00 X0 Y0\n");
					pw.close();	
					
					File old = pathFile.getValue();
					pathFile.setValue(pathFile.getValue()); //set the path as the default value.
					pathFile.valueChanged(null,old, pathFile.getValue());
					new AnnounceFrame("Generated successfully!",5);
					

				}
			} catch (Exception e1) {
				  new AnnounceFrame("Error when generate path file!",20);
				  return;
			}
			
		}
	}
	@Override
	public void variableChanged(EzVar<File> source, File newValue) {
		if(newValue != null)
		{
			try{
				File f = new File(newValue.getPath(),"gcode.txt");
				pathFile.setValue(f);
			}catch(Exception e){
				 new AnnounceFrame("Error path",20);
			}
		}
	}
	
	
	public static long copyFile(File srcFile, File destDir, String newFileName) {
		long copySizes = 0;
		if (!srcFile.exists()) {
			System.out.println("File does not exist!");
			copySizes = -1;
		} else if (!destDir.exists()) {
			System.out.println("Target folder does not exist");
			copySizes = -1;
		} else if (newFileName == null) {
			System.out.println("File name is null");
			copySizes = -1;
		} else {
			try {
				FileChannel fcin = new FileInputStream(srcFile).getChannel();
				FileChannel fcout = new FileOutputStream(new File(destDir,
						newFileName)).getChannel();
				long size = fcin.size();
				fcin.transferTo(0, fcin.size(), fcout);
				fcin.close();
				fcout.close();
				copySizes = size;
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		return copySizes;
	}

	
}