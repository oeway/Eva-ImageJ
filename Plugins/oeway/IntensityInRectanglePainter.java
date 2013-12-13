package plugins.oeway;
import icy.canvas.IcyCanvas;
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
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
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
public class IntensityInRectanglePainter extends Overlay implements ViewerListener,KeyListener
{
	public int nameIndex=0;
	public HashMap<ROI2D,IntensityPaint> roiPairDict = new HashMap<ROI2D,IntensityPaint>();
	public Point5D.Double cursorPos = new Point5D.Double();
	public Point lastPoint;       
	public boolean enableAddRoi = true;
	public IcyCanvas canvas;
	public boolean skipSelect = false;
	static  enum PaintMode
    {
        	line,
        	point,
        	area
        	
    }
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
        public PaintMode paintMode;

    	public IntensityPaint(ROI2D roi,Sequence seq, IcyCanvas canv)
    	{
    		canvas = canv;
    		sequence =seq;
    		guideRoi = roi;
    		displayRectangle = new ROI2DRectangle(lastPoint.getX(),lastPoint.getY(),lastPoint.getX()+Math.min(800,sequence.getWidth()/4),lastPoint.getY()+Math.min(800,sequence.getHeight()/4));
			displayRectangle.setName("("+guideRoi.getName()+")");
			displayRectangle.setColor(guideRoi.getColor());
			displayRectangle.setOpacity((float) 0.2);
			
			cursor1 = new Line2D.Double();
			cursor2 = new Line2D.Double();
			
			if(guideRoi.getClass().equals(ROI2DLine.class))
     			paintMode = PaintMode.line;
     		else if(guideRoi.getClass().equals(ROI2DPoint.class))
     			paintMode = PaintMode.point;
     		else
     			paintMode = PaintMode.area;

     		computeData();
     		
     		sequence.addListener(this);
     		guideRoi.addListener(this);
     		if(paintMode == PaintMode.line)
     			canvas.getViewer().addListener(this);
     		
    	}
    	
    	public void dispose()
    	{
    		try
    		{
    			sequence.removeListener(this);
     			guideRoi.removeListener(this);
     			canvas.getViewer().removeListener(this);
    		}
    		catch(Exception e)
    		{
    			
    		}
    		
    	}
    	public void computeData()
    	{
    		try
    		{
         		maxData = new double[sequence.getSizeC()];
         		minData = new double[sequence.getSizeC()];
         		drawPolygon = new Polygon[sequence.getSizeC()];

         		if(paintMode == PaintMode.line)
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
         			
	        	if(paintMode == PaintMode.line)
	        	{
	        		Line2D line = ((ROI2DLine) guideRoi).getLine();
	     			dataCount = (int) line.getP1().distance(line.getP2());
	        	
	        		ShapeUtil.consumeShapeFromPath(((ROI2DShape)guideRoi).getPathIterator(null), new ShapeConsumer()
	    	        {
	    	            @Override
	    	            public boolean consume(Shape shape)
	    	            {
	    	                if (shape instanceof Line2D)
	    	                {
	
	    	                	Line2D line = (Line2D) shape;
	    		            	Point2D Lp;
	    		            	Point2D Rp;
	    		            	if(line.getX2()>line.getX1())
	    		            	{
	    		            		Lp = line.getP1();
	    		            		Rp = line.getP2();
	    		            	}
	    		            	else
	    		            	{
	    		            		Lp = line.getP2();
	    		            		Rp = line.getP1();
	    		            	}
	    		            	
			    	            for (int component = 0; component < sequence.getSizeC(); component++)
			        	        {
			        	            // create histo data
			        	            int distance = dataCount;
			
			        	            double vx = (Rp.getX() - Lp.getX()) / distance;
			        	            double vy = (Rp.getY() - Lp.getY()) / distance;
			
			        	            double[] data = dataArr.get(component);
				
			        	            double x = Lp.getX();
			        	            double y = Lp.getY();
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
			        	            minData[component] =maxData[component] ;
			        	            
			        	            
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
			        	                
			        	                
			        	            }
			        	            Polygon polygon = new Polygon();
			        	            polygon.addPoint(0, 0);
			        	            for (int i = 0; i < dataCount; i++)
			        	            {
			        	            	polygon.addPoint(i, (int) (data[i]-minData[component]));
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
	            		
	                	if(paintMode == PaintMode.point)
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
    		                polygon.addPoint(i, (int) (data[i]-minData[component]));
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
			if(event.getType()== ROIEvent.ROIEventType.ROI_CHANGED)
				computeData();
		}
		@Override
		public void sequenceChanged(SequenceEvent sequenceEvent) {
			if(sequenceEvent.getSource() == SequenceEvent.SequenceEventSourceType.SEQUENCE_DATA )
				computeData();
		}
		@Override
		public void sequenceClosed(Sequence sequence) {
			sequence.removeROI(displayRectangle);
		}

		@Override
		public void viewerChanged(ViewerEvent event) {
			if(event.getType()== ViewerEvent.ViewerEventType.POSITION_CHANGED && paintMode == PaintMode.line)
			{
				computeData();
			}
		}
		@Override
		public void viewerClosed(Viewer viewer) {
			
		}
    	
    	
    }	
    @Override
    public void mouseMove(MouseEvent e, Point5D.Double imagePoint, IcyCanvas canvas)
    {
    	cursorPos = imagePoint;
    	painterChanged();
    	enableAddRoi = true;
    }
    @Override
    public void mouseDrag(MouseEvent e, Point5D.Double imagePoint, IcyCanvas canvas)
    {
    	cursorPos = imagePoint;
    	painterChanged();
    	enableAddRoi = false;
    } 
    
    @Override
    public void mousePressed(MouseEvent e, Point5D.Double imagePoint, IcyCanvas canvas)
    {
    	enableAddRoi = false;
    } 
    @Override
    public void mouseReleased(MouseEvent e, Point5D.Double imagePoint, IcyCanvas canvas)
    {
    	enableAddRoi = true;
    }     
    
	@Override
    public void paint(Graphics2D g, Sequence sequence, IcyCanvas canv)
    {
		if(canv != canvas)
		{
			canvas = canv;
			canvas.getViewer().addListener(this);
			canvas.getViewer().addKeyListener(this);
		}
		if(lastPoint == null)
			lastPoint = new Point(0,0);
        // create a graphics object so that we can then dispose it at the end of the paint to clean
        // all change performed in the paint,
        // like transform, color change, stroke (...).
		HashMap<ROI2D,IntensityPaint> roiPairTemp = new HashMap<ROI2D,IntensityPaint>();
        Graphics2D g2 = (Graphics2D) g.create();
        for (ROI2D roi : sequence.getROI2Ds())
        {
        	if(roi.getName().contains("[") ||roi.getName().contains("(") )
        		continue;
            {
            	IntensityPaint ip;
            	
            	
        		if(roiPairDict.containsKey(roi))
        		{
        			ip = roiPairDict.get(roi);
//        			rect.displayRectangle.setName("["+roi.getName()+"]");
//        			Rectangle2D box2 = rect.displayRectangle.getBounds2D();
//       //        			if(!sequence.getROI2Ds().contains(ip.displayRectangle)){//        				sequence.removeROI(rect.displayRectangle);//        				rect.displayRectangle.remove();//            			rect.displayRectangle new ROI2DRectangle(lastPoint.getX(),lastPoint.getY(),Math.min(800,sequence.getWidth()),0);;;//            			rect.displayRectangle.setColor(roi.getColor());//            			sequence.addROI(rect.displayRectangle);//        			}
        			if(ip != null)
            		{
	        			if(!sequence.getROI2Ds().contains(ip.displayRectangle))
	        			{
	        				ip.dispose();
	        			}
	        			else
	        				roiPairTemp.put(roi, ip);
            		}
        		}
        		else
        		{
        			ip = null;
        			if(enableAddRoi)
        			{
        				roi.setName(""+Integer.toString(nameIndex)+"#");
            			nameIndex +=1;
            			roi.setColor(getRandomColor());
            			//roi.setSelectedColor(getRandomColor());
        				ip = new IntensityPaint(roi,sequence,canvas);
        				lastPoint.setLocation(lastPoint.getX()+10,lastPoint.getY()+10);
        				if(lastPoint.getX()>sequence.getHeight() || lastPoint.getY()>sequence.getWidth())
        					lastPoint.setLocation(0,0);
        				sequence.addROI(ip.displayRectangle);
        			}
        			roiPairTemp.put(roi, ip);
        		}
        		if(ip != null && !skipSelect &&  roi instanceof ROI2DShape)
        		{
	        		if(roi.isSelected())
	        			ip.displayRectangle.setSelected(roi.isSelected());
	        		else if(ip.displayRectangle.isSelected())
	        			roi.setSelected(ip.displayRectangle.isSelected());
        		}
        		
        		try
        		{
        			if(ip != null)
        				drawHisto(roi, g2, sequence, canvas);
        		}
        		catch(Exception e2)
        		{
        			
        		}
        		
        	}
        }
        for (ROI2D roi : roiPairDict.keySet())
        {
        	if(!roiPairTemp.containsKey(roi))
        	{
        		if(roiPairDict.get(roi) != null)
        		{
        			roiPairDict.get(roi).dispose();
        			sequence.removeROI(roiPairDict.get(roi).displayRectangle); 
        		}
        	}
        }
        roiPairDict.clear();
        roiPairDict = roiPairTemp;
        g2.dispose();
    }

    void drawHisto(ROI2D roi, Graphics2D g, Sequence sequence, final IcyCanvas canvas)
    {

    	if(!roiPairDict.containsKey(roi))
    		return;
    	IntensityPaint ip = roiPairDict.get(roi);
    	
    	String currentValueX = "";
    	String currentValueV = "";
    	String maxValue = "";
    	String minValue = "";
    	
        for (int component = 0; component < sequence.getSizeC(); component++)
        {

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

	            if(ip.paintMode == PaintMode.line)
	        	{
	            	Line2D line = ((ROI2DLine) roi).getLine();
	            	int pos;
	            	if(Math.min(line.getX1(),line.getX2()) >= cursorPos.x)
	            		pos = 0;
	            	else if(Math.max(line.getX1(),line.getX2()) <= cursorPos.x)
	            		pos = ip.dataCount;
	            	else
	            	{
	            		pos = (int)( (cursorPos.x-Math.min(line.getX1(),line.getX2()))/ line.getP1().distance(line.getP2())*ip.dataCount);
	            		try
	            		{
	            			currentValueX = String .format("%.1f",cursorPos.x);
	            			currentValueV += String .format("%.1f ",ip.dataArr.get(component)[pos]);
	            		}
	            		catch(Exception e2)
	            		{
	            			
	            		}
	            	}
	            	
	            	ip.cursor1.setLine(pos, 0, pos, polyBox.getHeight());
	        	}
	            else
	            {
	            	int pos = (int) cursorPos.z;
	            	ip.cursor1.setLine(pos, 0, pos, polyBox.getHeight());
	            	try
            		{
	            		currentValueX = String .format("%.1f",cursorPos.z);
	            		currentValueV += String .format("%.3f :",ip.dataArr.get(component)[pos]);
            		}
            		catch(Exception e2)
            		{
            			
            		}	
	            }
	            
	            double sx = rectBox.getWidth()/polyBox.getWidth();
	            double sy = rectBox.getHeight()/polyBox.getHeight();

	            if(sx<100 && sy<100)
	            {
	            	g.translate(rectBox.getMinX(), rectBox.getMaxY());
	            	g.scale(sx, -sy);
	            	g.draw(ip.drawPolygon[component]);
		            g.setColor(new Color(100,100,170));
		            g.draw(ip.cursor1);
		            g.setColor(new Color(236,10,170));
	            }
	            else
	           {
	        	   char[] c = "Exceeding display limit!".toCharArray();
   	            	g.drawChars(c, 0, c.length ,(int)rectBox.getCenterX()-10,(int)rectBox.getCenterY());
	           }

	           
	            
            }
            finally
            {
            	g.setTransform(originalTransform);

	            //min,max
            	double xStart,xEnd;
        	    
	            if(ip.paintMode == PaintMode.line)
	        	{
	            	Line2D line = ((ROI2DLine) roi).getLine();
	            	Point2D Lp;
	            	Point2D Rp;
	            	if(line.getX2()>line.getX1())
	            	{
	            		Lp = line.getP1();
	            		Rp = line.getP2();
	            	}
	            	else
	            	{
	            		Lp = line.getP2();
	            		Rp = line.getP1();
	            	}
	            	xStart = Lp.getX();
	            	xEnd = Rp.getX();
	            	
	            	int pos;
	            	double yp;
	            	
	            	if(Math.min(line.getX1(),line.getX2()) >= cursorPos.x)
	            	{
	            		pos = (int) Lp.getX();
	            		yp = Lp.getY();
	            	}
	            	else if(Math.max(line.getX1(),line.getX2()) <= cursorPos.x)
	            	{
	            		pos = (int) Rp.getX();
	            		yp = Rp.getY();
	            	}
	            	else
	            	{
	            		pos = (int)cursorPos.x;
	            		yp = (cursorPos.x-Lp.getX())/(line.getX2()-line.getX1()) * (line.getY2()-line.getY1()) + Lp.getY();
	            	}
	            	
	            	ip.cursor2.setLine(pos, yp+10 , pos, yp-10);
	            	g.draw(ip.cursor2);
	            	
	            	
	        	}
	            else
	            {
	            	xStart = 0;
	            	xEnd = ip.dataCount;
	            }

	            maxValue +=String .format("%.1f ",ip.maxData[component]);
	            minValue += String .format("%.1f ",ip.minData[component]);
        	    if(component == sequence.getSizeC()-1)
        	    {
        	    
    	            char[] c = String .format("%.1f",xStart).toCharArray();
    	            //x1
    	            g.drawChars(c, 0, c.length ,(int)rectBox.getMinX(),(int)rectBox.getMaxY()+30);
           	    	
    	            c = String .format("%.1f",xEnd).toCharArray();
    	            //x2
    	            g.drawChars(c, 0, c.length ,(int)rectBox.getMaxX(),(int)rectBox.getMaxY()+30);
           	    
                    
    	            
            	    //c = (ip.displayRectangle.getName()).toCharArray();
    	            //ROI Name of line ROI
            	    //g.drawChars(c, 0, c.length ,(int) Rp.getX()+10,(int)Rp.getY());

            	    
            	    //c = (ip.displayRectangle.getName()).toCharArray();
    	            //ROI Name of line ROI
            	    //g.drawChars(c, 0, c.length, (int)rectBox.getCenterX(),(int)rectBox.getMaxY()-7 );
        	    	
		            //c = ("max:"+maxValue+" min:"+minValue).toCharArray();
    	            c = maxValue.toCharArray();
    	            g.drawChars(c, 0, c.length ,(int)rectBox.getMaxX()+10,(int)rectBox.getMinY()+10);
    	            
    	            c = minValue.toCharArray();
    	            g.drawChars(c, 0, c.length ,(int)rectBox.getMaxX()+10,(int)rectBox.getMaxY()-5);
    	            
	                c = currentValueX.toCharArray();
	                g.drawChars(c, 0, c.length,(int) (rectBox.getMinX()+(ip.cursor1.x1/ip.dataCount)*rectBox.getWidth()) -20 ,(int)rectBox.getMaxY()+ 15 );
	                
	                c = currentValueV.toCharArray();
	                g.drawChars(c, 0, c.length,(int) (rectBox.getMinX()+(ip.cursor1.x1/ip.dataCount)*rectBox.getWidth()) -20 ,(int)rectBox.getMinY()-5 );
	                
        	    }
        	    
            }
            

        }

    }
	@Override
	public void viewerChanged(ViewerEvent event) {
		if(event.getType()== ViewerEvent.ViewerEventType.POSITION_CHANGED )
		{
			cursorPos.z= canvas.getPositionZ();
			painterChanged();
		}
				
	}
	@Override
	public void viewerClosed(Viewer viewer) {
		try
		{
			canvas.getViewer().removeListener(this);
			canvas.getViewer().removeKeyListener(this);
		}
		catch(Exception e)
		{
			System.out.print(e.toString());
		}
		
	}
	@Override
	public void keyPressed(KeyEvent arg0) {
		if(arg0.getKeyChar()=='s')
		{
			skipSelect = !skipSelect;
		}
		
	}
	@Override
	public void keyReleased(KeyEvent arg0) {

		
	}
	@Override
	public void keyTyped(KeyEvent arg0) {
		// TODO Auto-generated method stub
		
	}

}