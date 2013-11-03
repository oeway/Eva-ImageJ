package plugins.oeway;

/*
 * Copyright 2013 Will Ouyang

 */
import icy.gui.dialog.MessageDialog;
import icy.gui.frame.progress.AnnounceFrame;
import icy.painter.Overlay;
import icy.plugin.abstract_.PluginActionable;
import plugins.kernel.roi.roi2d.ROI2DLine;
import icy.sequence.Sequence;

/**
 * 
 * 
 * 
 * @author Will Ouyang, modified from IntensityOverRoi by Fabrice de Chaumont & Stephane Dallongeville
 */
public class IntensityInRectangle extends PluginActionable
{
    @Override
    public void run()
    {
        Sequence sequence = getActiveSequence();
        if (sequence == null)
        {
            MessageDialog.showDialog("Please open an image first.", MessageDialog.ERROR_MESSAGE);
            return;
        } 
        IntensityInRectanglePainter oldPt = null;
        for(Overlay p: getActiveSequence().getOverlays())
        {
        	if( p instanceof IntensityInRectanglePainter)
        	{
        		oldPt = (IntensityInRectanglePainter) p;
        		break;
        	}
        }
        	
        if(oldPt == null)
        {
        	getActiveSequence().addOverlay(new IntensityInRectanglePainter("IntensityInRectangleOverlay"));
            // creates a ROI2DPolyLine if no ROI exists
            if (sequence.getROIs().size() == 0)
            {
                ROI2DLine roi = new ROI2DLine(0,sequence.getHeight()/2,sequence.getWidth(),sequence.getHeight()/2);
                sequence.addROI(roi);
            }
        }
        else
        {
        	new AnnounceFrame("Do you want to kill intensityInRectangle plugin", "Exit", new Runnable()
            {
        		IntensityInRectanglePainter Pt;
                @Override
                public void run()
                {
                	try{
                		
                		getActiveSequence().removeOverlay(Pt);
                	}
                	finally
                	{
                		
                	}
                	new AnnounceFrame("Remove all ROIs?", "Remove", new Runnable()
                    {
                        @Override
                        public void run()
                        {
                        	getActiveSequence().removeAllROI();
                        }
                    }, 15);
                }
                public Runnable init(IntensityInRectanglePainter pstr) {
                    this.Pt=pstr;
                    return(this);
                }
            }.init(oldPt), 15);
        	
        }
        
    }

}