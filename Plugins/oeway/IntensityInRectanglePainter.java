package plugins.oeway;
import icy.canvas.IcyCanvas;
import icy.image.IcyBufferedImage;
import icy.painter.Overlay;
import icy.roi.ROI2D;
import plugins.kernel.roi.roi2d.ROI2DRectangle;
import plugins.kernel.roi.roi2d.ROI2DShape;
import icy.sequence.Sequence;
import icy.type.collection.array.Array1DUtil;
import icy.util.ShapeUtil;
import icy.util.ShapeUtil.ShapeConsumer;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Polygon;
import java.awt.Shape;
import java.awt.geom.AffineTransform;
import java.awt.geom.Line2D;
import java.awt.geom.Rectangle2D;
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
	public HashMap<ROI2D,ROI2D> roiPairDict = new HashMap<ROI2D,ROI2D>();
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
	@Override
    public void paint(Graphics2D g, Sequence sequence, IcyCanvas canvas)
    {
		if(lastPoint == null)
			lastPoint = new Point(0,-Math.max(100,sequence.getHeight()));
        // create a graphics object so that we can then dispose it at the end of the paint to clean
        // all change performed in the paint,
        // like transform, color change, stroke (...).
		HashMap<ROI2D,ROI2D> roiPairTemp = new HashMap<ROI2D,ROI2D>();
        Graphics2D g2 = (Graphics2D) g.create();
        for (ROI2D roi : sequence.getROI2Ds())
        {
        	
            if (roi instanceof ROI2DShape){
            	if("ROI2DLine".equals(roi.getSimpleClassName()))
            	{
            		ROI2D rect;
            		if(roiPairDict.containsKey(roi))
            		{
            			rect = roiPairDict.get(roi);
            			rect.setName("["+roi.getName()+"]");
            			Rectangle2D box2 = rect.getBounds2D();
           
            			if(!sequence.getROI2Ds().contains(rect)){
            				sequence.removeROI(rect);
            				rect.remove();
            				Rectangle2D box = roi.getBounds2D();
                			rect = new ROI2DRectangle(box.getMinX(),box2.getMinY(),Math.max(5,box.getMaxX()),Math.max(5,box2.getMaxY()));
                			rect.setColor(roi.getColor());
                			sequence.addROI(rect);
            			}
            			
            		}
            		else
            		{
            			roi.setName(""+Integer.toString(nameIndex)+"#");
            			nameIndex +=1;
            			roi.setColor(getRandomColor());
            			//roi.setSelectedColor(getRandomColor());
            			
            			rect = new ROI2DRectangle(lastPoint.getX(),lastPoint.getY(),Math.min(800,sequence.getWidth()),0);
            			lastPoint.setLocation(lastPoint.getX()+20,lastPoint.getY()-20);
            			rect.setName("("+roi.getName()+")");
            			rect.setColor(roi.getColor());
            			sequence.addROI(rect);

            		}
            		
            		
            		if(roi.isSelected())
            			rect.setSelected(roi.isSelected());
            		else if(rect.isSelected())
            			roi.setSelected(rect.isSelected());
            		roiPairTemp.put(roi, rect);
            		
            		computeIntensityROI((ROI2DShape) roi,(ROI2DShape) rect, g2, sequence, canvas);
            	}
            }
        }
        for (ROI2D roi : roiPairDict.keySet())
        {
        	if(!roiPairTemp.containsKey(roi))
        		sequence.removeROI(roiPairDict.get(roi));          		
        }
        roiPairDict.clear();
        roiPairDict = roiPairTemp;
        g2.dispose();
    }

    private void computeIntensityROI(final ROI2DShape roi,final ROI2DShape rect, final Graphics2D g, final Sequence sequence, final IcyCanvas canvas)
    {

        ShapeUtil.consumeShapeFromPath(roi.getPathIterator(null), new ShapeConsumer()
        {
            @Override
            public boolean consume(Shape shape)
            {
                if (shape instanceof Line2D)
                {
                    drawHisto((Line2D) shape,roi.getName(), rect, g, sequence, canvas);
                }
                return true; // continue
            }
        });
    }

    void drawHisto(Line2D line,String roiName, ROI2DShape rect, Graphics2D g, Sequence sequence, final IcyCanvas canvas)
    {
        for (int component = 0; component < sequence.getSizeC(); component++)
        {
            // create histo data
            int distance = (int) line.getP1().distance(line.getP2());

            double vx = (line.getP2().getX() - line.getP1().getX()) / distance;
            double vy = (line.getP2().getY() - line.getP1().getY()) / distance;

            double[] data = new double[distance];

            double x = line.getP1().getX();
            double y = line.getP1().getY();
            IcyBufferedImage image = canvas.getCurrentImage();
            double maxData ;
            if (image.isInside((int) x, (int) y))
            {
            	maxData = Array1DUtil.getValue(image.getDataXY(component), image.getOffset((int) x, (int) y),
                        image.isSignedDataType());
            }
            else
            {
            	maxData = 0;
            }
            double minData = maxData;
            for (int i = 0; i < distance; i++)
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
                if(data[i]>maxData)
                	maxData = data[i];
                if(data[i]<minData)
                	minData = data[i];
                x += vx;
                y += vy;
            }

            AffineTransform originalTransform = g.getTransform();

            Polygon polygon = new Polygon();
            
            polygon.addPoint(0, 0);
            for (int i = 0; i < distance; i++)
                // pity polygon does not support this with double...
                polygon.addPoint(i, (int) data[i]);
            polygon.addPoint(distance, 0);

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
            Rectangle2D rectBox = ((ROI2DRectangle) rect).getRectangle();
            Rectangle2D polyBox = polygon.getBounds2D();
            try
            {
	           
	            g.translate(rectBox.getMinX(), rectBox.getMaxY());
	            double sx = rectBox.getWidth()/polyBox.getWidth();
	            double sy = rectBox.getHeight()/polyBox.getHeight();
	            g.scale(sx, -sy);
	            g.draw(polygon);


            }
            finally
            {
            	g.setTransform(originalTransform);
            	 // transform to put the painter at the right place
	            char[] c = Integer.toString((int)line.getP1().getX()).toCharArray();
	            //x1
        	    g.drawChars(c, 0, c.length ,(int)rectBox.getMinX(),(int)rectBox.getMaxY()+15);
        	    
	            c = Integer.toString((int)line.getP2().getX()).toCharArray();
	            //x2
        	    g.drawChars(c, 0, c.length ,(int)rectBox.getMaxX(),(int)rectBox.getMaxY()+15);
        	    
        	    c = ("max:"+Integer.toString((int)maxData)+" min:"+Integer.toString((int)minData)).toCharArray();
	            //min,max
        	    
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
        	    c = (roiName).toCharArray();
	            //ROI Name of line ROI
        	    if(line.getP1().getX()> line.getP2().getX())
        	    	g.drawChars(c, 0, c.length ,(int) line.getP1().getX()+10,(int)line.getP1().getY());
        	    else
        	    	g.drawChars(c, 0, c.length ,(int) line.getP2().getX()+10,(int)line.getP2().getY());
        	    
        	    c = (rect.getName()).toCharArray();
	            //ROI Name of line ROI
        	    g.drawChars(c, 0, c.length, (int)rectBox.getCenterX(),(int)rectBox.getMaxY()-7 );
        	    
            }
            

        }

    }

}
