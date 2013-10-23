package plugins.oeway;
import icy.gui.frame.progress.AnnounceFrame;
import icy.image.IcyBufferedImage;
import icy.main.Icy;
import icy.roi.ROI2D;
import icy.roi.ROI2DShape;
import icy.sequence.Sequence;
import icy.sequence.SequenceUtil;
import icy.type.collection.array.Array1DUtil;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;


import plugins.adufour.ezplug.*;
 
/**
 * Alignment pixels row by row according to the threshold
 * Originally used for surface tracking in ultrasound imaging
 * 
 * @author Will Ouyang / oeway007@gmail
 * 
 */
public class pixelRowAlign extends EzPlug implements EzStoppable, ActionListener
{
	


	EzVarSequence 				sequenceVar;
	EzVarDouble					thresholdVar;
	EzVarText					roiVar;
	EzVarBoolean 				inplaceVar;
	// some other data
	boolean						stopFlag;
	@Override
	protected void initialize()
	{
		// 1) variables must be initialized
		sequenceVar = new EzVarSequence("Target");
		thresholdVar = new EzVarDouble("Threshold");
		roiVar = new EzVarText("ROI", new String[] { "" }, 0, true);
		inplaceVar = new EzVarBoolean("In-Place",false);
	
		EzGroup groupInit= new EzGroup("", sequenceVar,roiVar,thresholdVar,inplaceVar);
		super.addEzComponent(groupInit);			

	}
	@Override
	public void actionPerformed(ActionEvent e) {

	}
	
	@Override
	protected void execute()
	{
		Sequence sequence = sequenceVar.getValue();

		double threshold = thresholdVar.getValue();
		ROI2DShape roi = null;

		for (ROI2D r : sequence.getROI2Ds())
		{	ROI2DShape r2 = null;
			if (r instanceof ROI2DShape){
				r2 = (ROI2DShape)r;
				if(r2.getName().trim() .equals(roiVar.getValue().trim()))
				{	
					roi = r2;
					break;
				}
			}
		}
		if(roi == null)
		{
			new AnnounceFrame("No roi named " + roiVar.getValue(),5);
			stopFlag = true;
			return;
		}
		
		if(!inplaceVar.getValue()){
			try
			{
				sequence = SequenceUtil.getCopy(sequenceVar.getValue());
	            Icy.getMainInterface().addSequence(sequence);
			}
			catch (Exception e) {
				new AnnounceFrame("Error when copying the sequnce undo will not available!");
			}
		}
		double minX = roi.getBounds().getMinX();
		double maxX = roi.getBounds().getMaxX();
		double val = 0.0;
		long cpt = 0;
		long totalImageCount = sequence.getSizeT()*sequence.getSizeZ()*sequence.getSizeC();
        for (int t = 0; t < sequence.getSizeT(); t++)
        {
        	for (int z = 0; z < sequence.getSizeZ(); z++)
        	{
				// Get the image at t=0 and z=0
				IcyBufferedImage image = sequence.getImage( t , z );		
		        for (int component = 0; component < sequence.getSizeC(); component++)
		        {
		
					// Get the data of the image for band 0 as a linear buffer, regardless of the type.
					Object imageData = image.getDataXY(component);
					
					// Get a copy of the data in double.
					double[] dataBuffer = Array1DUtil.arrayToDoubleArray( imageData , image.isSignedDataType() );
				    int i = 0;
					for(int y =0;y<image.getHeight();y++)
					{
						
						boolean detected = false;
						for(int x=(int) minX;x<maxX;x++)
						{
							if(detected)
							{
								i = x-(int)minX;
								break;
							}
							else
							{
								val = Array1DUtil.getValue(dataBuffer, image.getOffset((int) x, (int) y),
				                        image.isSignedDataType());
								if(val > threshold)
									detected = true;
							}
						}
						if(detected)
						{
							int length =image.getWidth()-i;
							for(int x=0;x<image.getWidth();x++)
							{
								if(x<length)
								{
									val = Array1DUtil.getValue(dataBuffer, image.getOffset((int)(x+i), (int)y),
					                        image.isSignedDataType());
									Array1DUtil.setValue(dataBuffer,image.getOffset((int)(x), (int)y),val);
								}
									else
										Array1DUtil.setValue(dataBuffer,image.getOffset((int)(x), (int)y),0.0);
									
							}
							
						}
						
					}
					cpt +=1;
					// Put the data back to the original image
					// Convert the double data automatically to the data type of the image. image.getDataXY(0) return a reference on the internal data of the image.
					Array1DUtil.doubleArrayToArray( dataBuffer , image.getDataXY( 0 ) ) ;
					// notify ICY the data has changed.
					image.dataChanged();	
			  		super.getUI().setProgressBarValue((double)cpt/totalImageCount);
			  		super.getUI().setProgressBarMessage(Long.toString(cpt)+"/"+ Long.toString(totalImageCount));
		        }
	        }
        }
        System.gc();
		stopFlag = true;
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