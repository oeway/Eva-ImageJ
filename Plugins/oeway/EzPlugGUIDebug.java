package plugins.oeway;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;

import javax.swing.JButton;
import javax.swing.JPanel;

import plugins.adufour.ezplug.*;
 
/**
 * Tutorial on how to use the EzPlug library to write plugins fast and efficiently
 * 
 * @author Alexandre Dufour
 * 
 */
public class EzPlugGUIDebug extends EzPlug implements EzStoppable, ActionListener,EzVarListener<File>
{
	
	// declare here the variables you wish to use
	// in this tutorial I will use one of each
	
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
	// some other data
	boolean						stopFlag;
	static double count ;
	@Override
	protected void initialize()
	{
		count = 1000;
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
		
		EzGroup groupSettings = new EzGroup("Settings",targetFolder,note); 
		super.addEzComponent(groupSettings);
		
		EzGroup groupScanMap = new EzGroup("Scan Map",scanMapSeq,runBundlebox,stepSize, scanSpeed,generatePath);
		super.addEzComponent(groupScanMap);	
		
		JPanel dummyPanel = new JPanel();
		dummyPanel.add(getUI().getContentPane());
		getUI().setContentPane(dummyPanel);
	}
	@Override
	public void actionPerformed(ActionEvent e) {
		
		 if (((JButton)e.getSource()).getText().equals(getPos.name)) {
			 count *= 10;
			posX.setValue(count);
			posY.setValue(count*2);
	
		}
		
	}
	
	@Override
	protected void execute()
	{
	
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
	public void variableChanged(EzVar<File> source, File newValue) {
		// TODO Auto-generated method stub
		
	}


	
}