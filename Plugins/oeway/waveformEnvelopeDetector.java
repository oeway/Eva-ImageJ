package plugins.oeway;

import icy.gui.frame.progress.AnnounceFrame;
import icy.image.IcyBufferedImage;
import icy.main.Icy;
import icy.sequence.Sequence;
import icy.sequence.SequenceUtil;
import icy.type.collection.array.Array1DUtil;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import biz.source_code.dsp.filter.FilterCharacteristicsType;
import biz.source_code.dsp.filter.FilterPassType;
import biz.source_code.dsp.filter.IirFilter;
import biz.source_code.dsp.filter.IirFilterCoefficients;
import biz.source_code.dsp.filter.IirFilterDesignFisher;
import biz.source_code.dsp.signal.EnvelopeDetector;
import plugins.adufour.ezplug.EzGroup;
import plugins.adufour.ezplug.EzPlug;
import plugins.adufour.ezplug.EzStoppable;
import plugins.adufour.ezplug.EzVarBoolean;
import plugins.adufour.ezplug.EzVarDouble;
import plugins.adufour.ezplug.EzVarInteger;
import plugins.adufour.ezplug.EzVarSequence;

/**
 * Detecting envelope of waveform
 * Originally used for surface tracking in ultrasound imaging
 * 
 * @author Will Ouyang / oeway007@gmail
 * 
 */

public class waveformEnvelopeDetector extends EzPlug implements EzStoppable, ActionListener
{

	EzVarSequence 				sequenceVar;
	EzVarInteger				sampleRateVar;
	EzVarDouble					lowerFilterCutoffFreqVar;
	EzVarDouble					upperFilterCutoffFreqVar;
	EzVarDouble					attackTimeVar;
	EzVarDouble					releaseTimeVar;
	EzVarInteger				filterOrderVar;
	EzVarDouble					filterRippleVar;
	EzVarBoolean 				inplaceVar;
	EzVarBoolean 				offsetMapVar;
	// some other data
	boolean						stopFlag;
	@Override
	protected void initialize()
	{
		// 1) variables must be initialized
		sequenceVar = new EzVarSequence("Target");
		sampleRateVar = new EzVarInteger("SampleRate");
		inplaceVar = new EzVarBoolean("In-Place",false);

		Double[] freqs={10.0,100.0,1000.0,10000.0,100000.0,1000000.0,10000000.0,100000000.0,};
		

		releaseTimeVar = new EzVarDouble("releaseTime");
		attackTimeVar = new EzVarDouble("attackTime");
		lowerFilterCutoffFreqVar = new EzVarDouble("lowerFilterCutoffFreq",freqs,true);
		upperFilterCutoffFreqVar = new EzVarDouble("upperFilterCutoffFreq",freqs,true);
		filterOrderVar = new EzVarInteger("filterOrder",1,20,1);
		filterRippleVar = new EzVarDouble("filterRipple",Double.MIN_VALUE,Double.MAX_VALUE,0.1);
		
		/**
		* Constructs an envelope detector.
		*    Sampling rate in Hz.
		*    Attack time of the envelope detector in seconds (time for 1/e convergence).
		*    Release time of the envelope detector in seconds (time for 1/e convergence).
		*    Filter for pre-processing the signal. May be null to bypass filtering.
		*/
		//set default values
		sampleRateVar.setValue((int) 1E4);
		attackTimeVar.setValue(0.0015);
		releaseTimeVar.setValue(0.03);
		lowerFilterCutoffFreqVar.setValue(1E2);
		upperFilterCutoffFreqVar.setValue(1E3);
		filterOrderVar.setValue(4);
		filterRippleVar.setValue(-0.5);
		
		EzGroup groupInit= new EzGroup("", 
				sequenceVar,sampleRateVar,
				releaseTimeVar,attackTimeVar,
				lowerFilterCutoffFreqVar,upperFilterCutoffFreqVar,
				filterOrderVar,filterRippleVar,
				inplaceVar);
		super.addEzComponent(groupInit);			

	}
	@Override
	public void actionPerformed(ActionEvent e) {

	}
	
	@Override
	protected void execute()
	{
		Sequence sequence = sequenceVar.getValue();
		sequence = SequenceUtil.getCopy(sequenceVar.getValue());
        Icy.getMainInterface().addSequence(sequence);

        double val;
		long cpt = 0;
		long totalImageCount = sequence.getSizeT()*sequence.getSizeZ()*sequence.getSizeC();
		try
		{
			
		   double attackTime =attackTimeVar.getValue();
		   double releaseTime = releaseTimeVar.getValue();
		   double lowerFilterCutoffFreq = lowerFilterCutoffFreqVar.getValue();
		   double upperFilterCutoffFreq = upperFilterCutoffFreqVar.getValue();
		   int filterOrder = filterOrderVar.getValue();                                    // higher bandpass filter orders would be instable because of the small lower cutoff frequency
		   double filterRipple = filterRippleVar.getValue();
		   double fcf1Rel = lowerFilterCutoffFreq / sampleRateVar.getValue();
		   double fcf2Rel = upperFilterCutoffFreq / sampleRateVar.getValue();
		   IirFilterCoefficients coeffs = IirFilterDesignFisher.design(FilterPassType.bandpass, FilterCharacteristicsType.chebyshev, filterOrder, filterRipple, fcf1Rel, fcf2Rel);
		  
			
			
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
						 IirFilter iirFilter = new IirFilter(coeffs);
						EnvelopeDetector envelopeDetector = new EnvelopeDetector(sampleRateVar.getValue(), attackTime, releaseTime, iirFilter);
						for(int y =0;y<image.getHeight();y++)
						{
							for(int x = 0;x<image.getWidth();x++)
							{
								val = Array1DUtil.getValue(dataBuffer, image.getOffset((int) x, (int) y),
				                        image.isSignedDataType());
							    val = envelopeDetector.step(val);
							    Array1DUtil.setValue(dataBuffer,image.getOffset((int)(x), (int)y),val);
	
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
		}
	     catch(Exception e)
	     {
	    	 System.err.println(e.toString());
	     }

        System.gc();
		stopFlag = true;
		new AnnounceFrame("Envelope detection done!",5);
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

