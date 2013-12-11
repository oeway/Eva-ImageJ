package plugins.oeway;
import icy.canvas.IcyCanvas;
import icy.canvas.IcyCanvasEvent;
import icy.canvas.IcyCanvasListener;
import icy.canvas.IcyCanvasEvent.IcyCanvasEventType;
import icy.gui.viewer.Viewer;
import icy.gui.viewer.ViewerEvent;
import icy.gui.viewer.ViewerListener;
import icy.image.IcyBufferedImage;
import icy.painter.Overlay;
import icy.roi.ROI2D;
import icy.roi.ROIEvent;
import icy.roi.ROIListener;
import plugins.kernel.roi.roi2d.ROI2DPoint;
import plugins.kernel.roi.roi2d.ROI2DLine;
import icy.roi.ROIUtil;
import plugins.kernel.roi.roi2d.ROI2DRectangle;
import plugins.kernel.roi.roi2d.ROI2DShape;
import icy.sequence.Sequence;
import icy.sequence.SequenceEvent;
import icy.sequence.SequenceListener;
import icy.type.collection.array.Array1DUtil;
import icy.type.point.Point5D;
import icy.util.ShapeUtil;
import icy.util.ShapeUtil.ShapeConsumer;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Polygon;
import java.awt.Shape;
import java.awt.event.MouseEvent;
import java.awt.geom.AffineTransform;
import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Random;

/**

 *           
 * @formatter:off
 *           
 *           This painter draws an intensity profile of the image over a line roi
 *           Every line roi works with a rectangle roi, the intensity prifile is showed with in the rectangle.
 *           
 * @author Will Ouyang, modified from IntensityOverRoi by Fabrice de Chaumont and Stephane Dallongeville
 */
public class IntensityInRectanglePainter extends Overlay
{
	public int nameIndex=0;
	public HashMap<ROI2D,IntensityPaint> roiPairDict = new HashMap<ROI2D,IntensityPaint>();
	public Point5D.Double cursorPos = new Point5D.Double();
	public Point lastPoint;
    public IntensityInRectanglePainter(String name) {
		super(name);
	}
    public Color getRandomColor()
    {
    	Random random = new Random();
    	final float hue = random.nextFloat();
    	final float saturation = (random.nextInt(2000) + 1000) / 5000f;
    	final float luminance = 0.9f;
    	final Color clr = Color.getHSBColor(hue, saturation, luminance);
    	return clr;
    }
    public class IntensityPaint implements ROIListener,SequenceListener,ViewerListener
    {

    	public ROI2D guideRoi;
    	public ROI2D displayRectangle;
    	public double[] maxData;
    	public double[] minData;
    	public Sequence sequence;
    	public ArrayList<double[]> dataArr ;
    	public IcyCanvas canvas;
    	public int dataCount;
        public Line2D.Double cursor1 ;
        public Line2D.Double cursor2 ;
        public Polygon[] drawPolygon;
        
        
    	public IntensityPaint(ROI2D roi,Sequence seq, IcyCanvas canv)
    	{
    		sequence =seq;
    		guideRoi = roi;
    		displayRectangle = new ROI2DRectangle(lastPoint.getX(),lastPoint.getY(),Math.min(800,sequence.getWidth()),0);
			lastPoint.setLocation(lastPoint.getX()+20,lastPoint.getY()-20);
			displayRectangle.setName("("+guideRoi.getName()+")");
			displayRectangle.setColor(guideRoi.getColor());
			
			cursor1 = new Line2D.Double();
			cursor2 = new Line2D.Double();
			
     		maxData = new double[sequence.getSizeC()];
     		minData = new double[sequence.getSizeC()];
     		drawPolygon = new Polygon[sequence.getSizeC()];
     		canvas = canv;
     		
     		if(guideRoi.getClass().equals(ROI2DLine.class))
        	{
     			Line2D line = ((ROI2DLine) guideRoi).getLine();
     			dataCount = (int) line.getP1().distance(line.getP2());
        	}
     		else
     		{
     			dataCount = sequence.getSizeZ();
     		}
			dataArr = new ArrayList<double[]>();

     		for (int component = 0; component < sequence.getSizeC(); component++)
     		{
     			double[] data = new double[dataCount];
     			dataArr.add(data);
     		}
     		
     		
     		
     		computeData();
     		guideRoi.addListener(this);
     		canvas.getViewer().addListener(this);
     		
    	}
    	public void computeData()
    	{
    		try
    		{
	        	if(guideRoi.getClass().equals(ROI2DLine.class))
	        	{
	        	
	        		ShapeUtil.consumeShapeFromPath(((ROI2DShape)guideRoi).getPathIterator(null), new ShapeConsumer()
	    	        {
	    	            @Override
	    	            public boolean consume(Shape shape)
	    	            {
	    	                if (shape instanceof Line2D)
	    	                {
	
	    	                	Line2D line = (Line2D) shape;
			    	            for (int component = 0; component < sequence.getSizeC(); component++)
			        	        {
			        	            // create histo data
			        	            int distance = dataCount;
			
			        	            double vx = (line.getP2().getX() - line.getP1().getX()) / distance;
			        	            double vy = (line.getP2().getY() - line.getP1().getY()) / distance;
			
			        	            double[] data = dataArr.get(component);
				
			        	            double x = line.getP1().getX();
			        	            double y = line.getP1().getY();
			        	            IcyBufferedImage image = canvas.getCurrentImage();
	
			        	            if (image.isInside((int) x, (int) y))
			        	            {
			        	            	maxData[component] = Array1DUtil.getValue(image.getDataXY(component), image.getOffset((int) x, (int) y),
			        	                        image.isSignedDataType());
			        	            }
			        	            else
			        	            {
			        	            	maxData[component] = 0;
			        	            }
			        	            
			        	            Polygon polygon = new Polygon();
			        	            polygon.addPoint(0, 0);
			        	            for (int i = 0; i < dataCount; i++)
			        	            {
			        	                
			        	                if (image.isInside((int) x, (int) y))
			        	                {
			        	                    data[i] = Array1DUtil.getValue(image.getDataXY(component), image.getOffset((int) x, (int) y),
			        	                            image.isSignedDataType());
			        	                }
			        	                else
			        	                {
			        	                    data[i] = 0;
			        	                }
			        	                if(data[i]>maxData[component])
			        	                	maxData[component] = data[i];
			        	                if(data[i]<minData[component])
			        	                	minData[component] = data[i];
			        	                x += vx;
			        	                y += vy;
			        	                polygon.addPoint(i, (int) data[i]);
			        	                
			        	            }
			        	            polygon.addPoint( dataCount, 0);
			        	            drawPolygon[component] = polygon;
			        	        }	
			    	        }
			    	        return true; // continue
			    	     }
	    	        });
	        		 
	        	}
	        	else
	        	{
	
	                for (int component = 0; component < sequence.getSizeC(); component++)
	                {
	            		double[] data = dataArr.get(component);
	            		
	                	if(guideRoi.getClass().equals(ROI2DPoint.class))
	                	{
	                		Point p = guideRoi.getPosition();
	                		maxData[component] = sequence.getData(0, 0, component, p.y , p.x);
	                		
	    	                minData[component] = maxData[component];
	    	                for(int i=0;i<dataCount;i++)
	    	                {
	    	                	data[i] =  sequence.getData(0, i, component, p.y , p.x);
	    	                    if(data[i]>maxData[component])
	    	                    	maxData[component] = data[i];
	    	                    if(data[i]<minData[component])
	    	                    	minData[component] = data[i];
	    	                }
	                	}
	                	else
	                	{
	    	                maxData[component] = ROIUtil.getMeanIntensity(sequence, guideRoi,0,-1,component); ;
	    	                minData[component] = maxData[component];
	    		    		for(int i=0;i<dataCount;i++)
	    		    		{
	    		    			data[i] = ROIUtil.getMeanIntensity(sequence, guideRoi,i,-1,component);
	    		                if(data[i]>maxData[component])
	    		                	maxData[component] = data[i];
	    		                if(data[i]<minData[component])
	    		                	minData[component] = data[i];
	    		    		}
	                	}
    		            Polygon polygon = new Polygon();
    		            
    		            polygon.addPoint(0, 0);
    		            for (int i = 0; i < dataCount; i++)
    		                // pity polygon does not support this with double...
    		                polygon.addPoint(i, (int) data[i]);
    		            polygon.addPoint( dataCount, 0);
	                	drawPolygon[component] = polygon;
	                }
	        		
	        	}
    		}
    		catch(Exception e)
    		{
    			System.out.print(e);
    		}
    		
    	}
		@Override
		public void roiChanged(ROIEvent event) {
			computeData();
		}
		@Override
		public void sequenceChanged(SequenceEvent sequenceEvent) {
			computeData();
		}
		@Override
		public void sequenceClosed(Sequence sequence) {
			
		}

		@Override
		public void viewerChanged(ViewerEvent event) {
			cursorPos.z=canvas.getPositionZ();
				computeData();
		}
		@Override
		public void viewerClosed(Viewer viewer) {
			// TODO Auto-generated method stub
			
		}
    	
    	
    }	
    @Override
    public void mouseMove(MouseEvent e, Point5D.Double imagePoint, IcyCanvas canvas)
    {
    	cursorPos = imagePoint;
    	painterChanged();
    }
	@Override
    public void paint(Graphics2D g, Sequence sequence, IcyCanvas canvas)
    {
		if(lastPoint == null)
			lastPoint = new Point(0,-Math.max(100,sequence.getHeight()));
        // create a graphics object so that we can then dispose it at the end of the paint to clean
        // all change performed in the paint,
        // like transform, color change, stroke (...).
		HashMap<ROI2D,IntensityPaint> roiPairTemp = new HashMap<ROI2D,IntensityPaint>();
        Graphics2D g2 = (Graphics2D) g.create();
        for (ROI2D roi : sequence.getROI2Ds())
        {
        	if(roi.getName().contains("[") ||roi.getName().contains("(") )
        		continue;
            if (roi instanceof ROI2DShape){
            	IntensityPaint rect;
        		if(roiPairDict.containsKey(roi))
        		{
        			rect = roiPairDict.get(roi);
//        			rect.displayRectangle.setName("["+roi.getName()+"]");
//        			Rectangle2D box2 = rect.displayRectangle.getBounds2D();
//       
//        			if(!sequence.getROI2Ds().contains(rect)){
//        				sequence.removeROI(rect.displayRectangle);
//        				rect.displayRectangle.remove();
//        				Rectangle2D box = roi.getBounds2D();
//            			rect.displayRectangle = new ROI2DRectangle(box.getMinX(),box2.getMinY(),Math.max(5,box.getMaxX()),Math.max(5,box2.getMaxY()));
//            			rect.displayRectangle.setColor(roi.getColor());
//            			sequence.addROI(rect.displayRectangle);
//        			}
        			
        		}
        		else
        		{
        			roi.setName(""+Integer.toString(nameIndex)+"#");
        			nameIndex +=1;
        			roi.setColor(getRandomColor());
        			//roi.setSelectedColor(getRandomColor());
        			
        		
        			lastPoint.setLocation(lastPoint.getX()+20,lastPoint.getY()-20);
        			rect = new IntensityPaint(roi,sequence,canvas);
        			sequence.addROI(rect.displayRectangle);

        		}
        		
        		
        		if(roi.isSelected())
        			rect.displayRectangle.setSelected(roi.isSelected());
        		else if(rect.displayRectangle.isSelected())
        			roi.setSelected(rect.displayRectangle.isSelected());
        		roiPairTemp.put(roi, rect);
        		
        		drawHisto((ROI2DShape) roi, g2, sequence, canvas);
        	}
        }
        for (ROI2D roi : roiPairDict.keySet())
        {
        	if(!roiPairTemp.containsKey(roi))
        		sequence.removeROI(roiPairDict.get(roi).displayRectangle);          		
        }
        roiPairDict.clear();
        roiPairDict = roiPairTemp;
        g2.dispose();
    }

    void drawHisto(ROI2DShape roi, Graphics2D g, Sequence sequence, final IcyCanvas canvas)
    {
    	if(!roi.getClass().equals(ROI2DLine.class) && sequence.getSizeZ()<2)
    	{
    		return;
    	}
    	if(!roiPairDict.containsKey(roi))
    		return;
    	
        for (int component = 0; component < sequence.getSizeC(); component++)
        {
        	String currentValue = "#";
        	IntensityPaint ip = roiPairDict.get(roi);

            AffineTransform originalTransform = g.getTransform(); 
           
            g.setColor(new Color(236,10,170));
            if (sequence.getSizeC() != 1)
            {
                if (component == 0)
                    g.setColor(Color.red);
                if (component == 1)
                    g.setColor(Color.green);
                if (component == 2)
                    g.setColor(Color.blue);
            }
            Rectangle2D rectBox = ((ROI2DRectangle) ip.displayRectangle).getRectangle();
            Rectangle2D polyBox = ip.drawPolygon[component].getBounds2D();
            try
            {
	           
	            g.translate(rectBox.getMinX(), rectBox.getMaxY());
	            double sx = rectBox.getWidth()/polyBox.getWidth();
	            double sy = rectBox.getHeight()/polyBox.getHeight();
	            g.scale(sx, -sy);
	            g.draw(ip.drawPolygon[component]);
	            
	            
	            if(roi.getClass().equals(ROI2DLine.class))
	        	{
	            	Line2D line = ((ROI2DLine) roi).getLine();
	            	int pos;
	            	if(Math.min(line.getX1(),line.getX2()) > cursorPos.x)
	            		pos = 0;
	            	else if(Math.max(line.getX1(),line.getX2()) < cursorPos.x)
	            		pos = ip.dataCount;
	            	else
	            	{
	            		pos = (int)( (cursorPos.x-Math.min(line.getX1(),line.getX2()))/ line.getP1().distance(line.getP2())*ip.dataCount);
	            		try
	            		{
	            			currentValue =String .format("%.3f",ip.dataArr.get(component)[pos]);
	            		}
	            		catch(Exception e2)
	            		{
	            			
	            		}
	            	}
	            	
	            	ip.cursor1.setLine(pos, 0, pos, ip.maxData[component]);
	        	}
	            else
	            {
	            	int pos = (int) cursorPos.z;
	            	ip.cursor1.setLine(pos, 0, pos, ip.maxData[component]);
	            	try
            		{
	            		currentValue = String .format("%.3f", ip.dataArr.get(component)[pos]);
            		}
            		catch(Exception e2)
            		{
            			
            		}	
	            }
	            g.draw(ip.cursor1);
	            
            }
            finally
            {
            	g.setTransform(originalTransform);
//            	 // transform to put the painter at the right place
//	            char[] c = Integer.toString((int)line.getP1().getX()).toCharArray();
//	            //x1
//        	    g.drawChars(c, 0, c.length ,(int)rectBox.getMinX(),(int)rectBox.getMaxY()+15);
//        	    
//	            c = Integer.toString((int)line.getP2().getX()).toCharArray();
//	            //x2
//        	    g.drawChars(c, 0, c.length ,(int)rectBox.getMaxX(),(int)rectBox.getMaxY()+15);
        	    
            	
            	
        	    
	            //min,max
        	    
	            if(roi.getClass().equals(ROI2DLine.class))
	        	{
	            	Line2D line = ((ROI2DLine) roi).getLine();
	            	int pos;
	            	if(Math.min(line.getX1(),line.getX2()) > cursorPos.x)
	            		pos = (int) Math.min(line.getX1(),line.getX2());
	            	else if(Math.max(line.getX1(),line.getX2()) < cursorPos.x)
	            		pos = (int) Math.max(line.getX1(),line.getX2());
	            	else
	            		pos = (int)cursorPos.x;
	            	
	            	double yp = (cursorPos.x-Math.min(line.getX1(),line.getX2()))/line.getP1().distance(line.getP2()) * (line.getY2()-line.getY1()) +line.getY1();
	            	
	            	
	            	ip.cursor2.setLine(pos, yp+10 , pos, yp-10);
	            	g.draw(ip.cursor2);
	            	
	            	
	        	}

	            char[] c = ("max:"+Integer.toString((int)ip.maxData[component])+" min:"+Integer.toString((int)ip.minData[component])).toCharArray();
        	    
                if (sequence.getSizeC() != 1)
                {
	        	    if(component == 1)
	        	    g.drawChars(c, 0, c.length ,(int)rectBox.getMinX(),(int)rectBox.getMinY()-8);
	        	    if(component == 2)
	        	    g.drawChars(c, 0, c.length ,(int)rectBox.getCenterX(),(int)rectBox.getMinY()-8);
	        	    if(component == 3)
	        	    g.drawChars(c, 0, c.length ,(int)rectBox.getMaxX(),(int)rectBox.getMinY()-8);
                }
                else
                	g.drawChars(c, 0, c.length ,(int)rectBox.getCenterX(),(int)rectBox.getMinY()-8);
                
                c = currentValue.toCharArray();
        	    
                if (sequence.getSizeC() != 1)
                {
                	//TODO: handle multi-channel
//	        	    if(component == 1)
//	        	    g.drawChars(c, 0, c.length ,(int)rectBox.getMinX(),(int)rectBox.getMinY()-8);
//	        	    if(component == 2)
//	        	    g.drawChars(c, 0, c.length ,(int)rectBox.getCenterX(),(int)rectBox.getMinY()-8);
//	        	    if(component == 3)
//	        	    g.drawChars(c, 0, c.length ,(int)rectBox.getMaxX(),(int)rectBox.getMinY()-8);
                }
                else
                	 g.drawChars(c, 0, c.length,(int) (rectBox.getMinX()+(ip.cursor1.x1/ip.dataCount)*rectBox.getWidth()) -10 ,(int)rectBox.getMaxY()+ 15 );
                
                
        	    c = (ip.displayRectangle.getName()).toCharArray();
//	            //ROI Name of line ROI
//        	    if(line.getP1().getX()> line.getP2().getX())
//        	    	g.drawChars(c, 0, c.length ,(int) line.getP1().getX()+10,(int)line.getP1().getY());
//        	    else
//        	    	g.drawChars(c, 0, c.length ,(int) line.getP2().getX()+10,(int)line.getP2().getY());
        	    
        	    c = (ip.displayRectangle.getName()).toCharArray();
	            //ROI Name of line ROI
        	    g.drawChars(c, 0, c.length, (int)rectBox.getCenterX(),(int)rectBox.getMaxY()-7 );
        	    
            }
            

        }

    }

}