package plugins.oeway;
import icy.gui.dialog.MessageDialog;
import icy.gui.frame.progress.AnnounceFrame;
import icy.image.IcyBufferedImage;
import icy.main.Icy;
import icy.roi.ROI2D;
import plugins.kernel.roi.roi2d.ROI2DShape;
import icy.sequence.Sequence;
import icy.sequence.SequenceUtil;
import icy.type.DataType;
import icy.type.collection.array.Array1DUtil;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;


import plugins.adufour.ezplug.*;
 
/**
 * Align pixels row by row according to the threshold
 * Originally used for surface tracking in ultrasound imaging
 * 
 * @author Will Ouyang / oeway007@gmail
 * 
 */
public class pixelRowAlign extends EzPlug implements EzStoppable, ActionListener
{
	
	private enum thresholdModesEnum
	{
		GreaterThan,LessThan,EqualTo
	}

	EzVarSequence 				sequenceVar;
	EzVarDouble					thresholdVar;
	EzVarEnum<thresholdModesEnum>	thresholdModeVarEnum;
	EzVarBoolean 				inplaceVar;
	EzVarBoolean 				offsetMapVar;
	EzVarBoolean 				useLastVar;
	// some other data
	boolean						stopFlag;
	@Override
	protected void initialize()
	{
		// 1) variables must be initialized
		sequenceVar = new EzVarSequence("Target");
		thresholdVar = new EzVarDouble("Threshold");
		inplaceVar = new EzVarBoolean("In-Place",false);
		offsetMapVar = new EzVarBoolean("Offset Map",true);
		useLastVar = new EzVarBoolean("Remember Previous Value",false);
		thresholdModeVarEnum = new EzVarEnum<thresholdModesEnum>("Threshold Mode", thresholdModesEnum.values(), thresholdModesEnum.GreaterThan);
		EzGroup groupInit= new EzGroup("", sequenceVar,thresholdModeVarEnum,thresholdVar,inplaceVar,offsetMapVar,useLastVar);
		super.addEzComponent(groupInit);			

	}
	@Override
	public void actionPerformed(ActionEvent e) {

	}
	
	@Override
	protected void execute()
	{
		Sequence sequence = sequenceVar.getValue();
		IcyBufferedImage offsetMap = null;
		
		double threshold = thresholdVar.getValue();
		ROI2DShape roi = null;
		
		 ArrayList<ROI2D> rois = sequence.getROI2Ds();
		 try
		 {
	        int size = rois.size();

	        if (size == 0)
	        {
	            MessageDialog.showDialog("There is no ROI in the current sequence.\nAlign operation need a ROI.",
	                    MessageDialog.INFORMATION_MESSAGE);
	            return;
	        }
	        else if (size >= 1)
	        {
	            rois = sequence.getSelectedROI2Ds();
	            size = rois.size();
	            if (size == 1)
	            {
	            	if (rois.get(0) instanceof ROI2DShape)
	            		roi = (ROI2DShape)rois.get(0);
	            }
	            else if (size > 1)
	            {
	            	for (ROI2D r : rois)
	        		{	ROI2DShape r2 = null;
	        			if (r instanceof ROI2DShape && "ROI2DLine".equals(r.getSimpleClassName()))
	        			{
	        				r2 = (ROI2DShape)r;
	        					roi = r2;
	        					break;
	        			}
	        		}
	            }
	        }
			if(roi == null)
			{
				MessageDialog.showDialog("No ROI available, please select a ROI for alignment.",
	                    MessageDialog.INFORMATION_MESSAGE);
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
			offsetMap = new IcyBufferedImage(sequence.getSizeZ(),sequence.getSizeY(),sequence.getSizeT(),DataType.USHORT);	
			double minX = roi.getBounds().getMinX();
			double maxX = roi.getBounds().getMaxX();
			double val = 0.0;
			long cpt = 0;
			long totalImageCount = sequence.getSizeT()*sequence.getSizeZ()*sequence.getSizeC();
			boolean usePreviousVal = useLastVar.getValue();
			sequence.beginUpdate();
	        for (int t = 0; t < sequence.getSizeT(); t++)
	        {
	        	if(stopFlag)
	        		break;
	        	for (int z = 0; z < sequence.getSizeZ(); z++)
	        	{
		        	if(stopFlag)
		        		break;
					// Get the image at t=0 and z=0
					IcyBufferedImage image = sequence.getImage( t , z );		
			        for (int component = 0; component < sequence.getSizeC(); component++)
			        {
			        	if(stopFlag)
			        		break;
						// Get the data of the image for band 0 as a linear buffer, regardless of the type.
						Object imageData = image.getDataXY(component);
						
						// Get a copy of the data in double.
						double[] dataBuffer = Array1DUtil.arrayToDoubleArray( imageData , image.isSignedDataType() );
					    int i=0;
						for(int y =0;y<image.getHeight();y++)
						{
							if(!usePreviousVal)
								i = 0;
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
									if(thresholdModeVarEnum.getValue() == thresholdModesEnum.GreaterThan)
									{
										if(val > threshold)
											detected = true;
									}
									else if(thresholdModeVarEnum.getValue() == thresholdModesEnum.EqualTo)
									{
										if(val == threshold)
											detected = true;
									}
									else
									{
										if(val < threshold)
											detected = true;
									}
								}
							}
							offsetMap.setData(z, y, t, i);
							//if(detected) //use last i if can't detect
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
	        
	        if(offsetMapVar.getValue()){
		        Sequence offsetSeq=new Sequence();
				offsetSeq.setImage(0, 0, offsetMap);
				offsetSeq.setName("Offset Map of " + sequence.getName());
				Icy.getMainInterface().addSequence(offsetSeq);
	        }
		 }
		 catch(Exception e)
		 {
			 System.err.println(e.toString());
		 }
		 finally
		 {
			 sequence.endUpdate();
		 }
        System.gc();
		stopFlag = true;
		new AnnounceFrame("Alignment operation done!",5);
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