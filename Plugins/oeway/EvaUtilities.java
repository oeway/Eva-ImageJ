package plugins.oeway;

import icy.gui.frame.progress.AnnounceFrame;
import plugins.adufour.ezplug.*;
 
/**
 * Tutorial on how to use the EzPlug library to write plugins fast and efficiently
 * 
 * @author Alexandre Dufour
 * 
 */
public class EvaUtilities extends EzPlug implements EzStoppable
{


	EzVarSequence				varSequence;
	// some other data
	boolean						stopFlag;
	
	@Override
	protected void initialize()
	{
		// 1) variables must be initialized
		varSequence = new EzVarSequence("Input sequence");
		// let's add a description label
		EzLabel label = new EzLabel("Check above to show/hide a variable");
		EzGroup groupSequence = new EzGroup("Sequence group",  label, varSequence);
		super.addEzComponent(groupSequence);

	}
	
	@Override
	protected void execute()
	{
		// main plugin code goes here, and runs in a separate thread
		
		stopFlag = false;

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
	
}