package plugins.oeway.viewers;

import java.awt.Component;
import java.awt.image.BufferedImage;

import icy.canvas.IcyCanvas;
import icy.canvas.IcyCanvasEvent;
import icy.gui.viewer.Viewer;
import icy.plugin.abstract_.PluginActionable;
import icy.plugin.interface_.PluginCanvas;
import icy.sequence.DimensionId;
import icy.sequence.Sequence;
import icy.sequence.SequenceEvent;
import icy.sequence.SequenceListener;
import icy.sequence.SequenceEvent.SequenceEventType;

/**
 * Class providing a plug-in and a viewer for 1D signal stored as a 1-row image
 * 
 * 
 * 
 * @author  Wei Ouyang
 * 
 */
public class Chart1DCanvas extends PluginActionable implements PluginCanvas, SequenceListener
{
    /**
     * 
     * 
     * 
     * @param 
     *           
     * @return 
     * @return a jfreechart to show the intensity profile of one row or column
     */
	private ChartCanvas ChartCanvas;

    
    @Override
    public void run()
    {
        final Sequence input = getFocusedSequence();
        if (input == null) return;

        
    }
    
    @Override
    public String getCanvasClassName()
    {
        return ChartCanvas.class.getName();
    }
    
    @Override
    public IcyCanvas createCanvas(Viewer viewer)
    {
    	ChartCanvas = new ChartCanvas(viewer);
    	Sequence  sequence = viewer.getSequence();
    	sequence.addListener(this);
        return ChartCanvas;
    }
    

	@Override
	public void sequenceChanged(SequenceEvent sequenceEvent) {
		
        switch (sequenceEvent.getSourceType())
        {
            case SEQUENCE_DATA:
            	if(sequenceEvent.getType() != SequenceEventType.CHANGED)
            	{
            		ChartCanvas.iprofile.updateXYNav();
            	}
            	ChartCanvas.iprofile .updateChart();
            	break;
		default:
			break;
        }
	}

	@Override
	public void sequenceClosed(Sequence sequence) {
		// TODO Auto-generated method stub
		
	}
	
	
	
    public class ChartCanvas extends IcyCanvas 
    {
        private static final long serialVersionUID = 1L;
        
        private Sequence  sequence;

        private IntensityProfile iprofile;
        private Viewer currentViewer;
        public ChartCanvas(Viewer viewer)
        {
        	super(viewer);
            // set position to first Y, Z and Z
            posZ = 0;
            posT = 0;
            posY = 0;
            // all channel visible at once ?
            posC = -1;
            
        	currentViewer = viewer;
            sequence= getSequence();
            iprofile = new IntensityProfile(this,getSequence());


            // mouse infos panel setting: we want to see values for X only (1D view)
            mouseInfPanel.setInfoXVisible(true);
            // Y, Z and T values are already visible in row, Z and T navigator bar
            mouseInfPanel.setInfoYVisible(true);
            
            mouseInfPanel.setInfoZVisible(false);
            mouseInfPanel.setInfoTVisible(false);
            // no C navigation with this canvas (all channels visible)
            mouseInfPanel.setInfoCVisible(false);
            // data information visible
            mouseInfPanel.setInfoDataVisible(true);
            // no color information visible
            mouseInfPanel.setInfoColorVisible(false);

            // refresh Z and T navigation bar state depending
            updateZNav();
            updateTNav();

        }
		@Override
		public void refresh() {
			// TODO Auto-generated method stub
			iprofile.updateChart();
		}
		@Override
		public Component getViewComponent() {
			// TODO Auto-generated method stub
			return null;
		}
		@Override
		public BufferedImage getRenderedImage(int t, int z, int c,
				boolean canvasView) {
			// TODO Auto-generated method stub
			return null;
		}
      
	    @Override
	    public void changed(IcyCanvasEvent event)
	    {
	        super.changed(event);

	        switch (event.getType())
	        {
	            case POSITION_CHANGED:
	            	DimensionId i = event.getDim();
	            	if (i == DimensionId.T || i == DimensionId.Z )
	            		iprofile.updateChart();
	            	break;
			default:
				break;
	        }
	    }
	    @Override
	    public double getMouseImagePosX()
	    {
	        // can be called before constructor ended
	        if (iprofile == null)
	            return 0d;

	        return iprofile.posX;

	    }

	    @Override
	    public double getMouseImagePosY()
	    {
	        // can be called before constructor ended
	        if (iprofile == null)
	            return 0d;

	        return iprofile.posY;
	    }
 
    }



}
