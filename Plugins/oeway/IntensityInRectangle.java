package plugins.oeway;

/*
 * Copyright 2013 Will Ouyang

 */
import icy.gui.dialog.MessageDialog;
import icy.gui.frame.progress.AnnounceFrame;
import icy.painter.Painter;
import icy.plugin.abstract_.PluginActionable;
import icy.roi.ROI2D;
import icy.roi.ROI2DLine;
import icy.roi.ROI2DPolyLine;
import icy.sequence.Sequence;

import java.awt.Polygon;

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

        Sequence sequence = getFocusedSequence();
        if (sequence == null)
        {
            MessageDialog.showDialog("Please open an image first.", MessageDialog.ERROR_MESSAGE);
            return;
        } 
        IntensityInRectanglePainter oldPt = null;
        for(Painter p: getFocusedSequence().getPainters())
        {
        	if( p instanceof IntensityInRectanglePainter)
        	{
        		oldPt = (IntensityInRectanglePainter) p;
        		break;
        	}
        }
        	
        if(oldPt == null)
        {
        	getFocusedSequence().addPainter(new IntensityInRectanglePainter("IntensityInRectangleOverlay"));
            // creates a ROI2DPolyLine if no ROI exists
            if (sequence.getROIs().size() == 0)
            {
                ROI2DLine roi = new ROI2DLine(0,sequence.getHeight()/2,sequence.getWidth(),sequence.getHeight()/2);
                sequence.addROI(roi);
            }
        }
        else
        {
        	new AnnounceFrame("IntensityInRectangle mode terminated. Remove all ROIs?", "Remove", new Runnable()
            {
                @Override
                public void run()
                {
                	getFocusedSequence().removeAllROI();
                }
            }, 15);
        	getFocusedSequence().removePainter(oldPt);
        }
        
    }

}
