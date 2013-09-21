package plugins.oeway.viewers;
/**
 *  this class is modified from the plugin Intensity Profile by Fab.
 * 
 * 
 * 
 * @author Wei Ouyang
 * 
 */

import icy.canvas.IcyCanvas;
import icy.gui.component.IcySlider;
import icy.gui.util.ComponentUtil;
import icy.gui.util.GuiUtil;
import icy.image.IcyBufferedImage;
import icy.roi.BooleanMask2D;
import icy.roi.ROI;
import icy.roi.ROI2DEllipse;
import icy.roi.ROI2DLine;
import icy.roi.ROI2DPolyLine;
import icy.roi.ROI2DPolygon;
import icy.roi.ROI2DRectangle;
import icy.roi.ROI2DShape;
import icy.sequence.DimensionId;
import icy.sequence.Sequence;
import icy.system.thread.ThreadUtil;
import icy.type.collection.array.Array1DUtil;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.SwingConstants;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartMouseEvent;
import org.jfree.chart.ChartMouseListener;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.plot.Marker;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.plot.ValueMarker;
import org.jfree.chart.plot.XYPlot;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;
import org.jfree.ui.RectangleAnchor;
import org.jfree.ui.TextAnchor;

import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
public class IntensityProfile  {

    final IcySlider slider;
	JButton exportToExcelButton = new JButton("Export to excel");
	JCheckBox graphOverZ = new JCheckBox("Graph Z");
	JButton rowOColBtn = new JButton("row");
	JLabel indexLbl = new JLabel("0");
	JLabel maxIndexLbl = new JLabel("0");
	public PanningChartPanel chartPanel ;
	public JFreeChart chart;
	XYSeriesCollection xyDataset = new XYSeriesCollection();
	
	String OPTION_meanAlongZ = "Mean along Z";
	String OPTION_meanAlongT = "Mean along T";	
	
	ArrayList<Marker> markerDomainList = new ArrayList<Marker>();
	ArrayList<Marker> markerRangeList = new ArrayList<Marker>();
	
	CheckComboBox optionComboBox;
	
	ROI associatedROI = null;
	private Sequence sequence;
	
	boolean rowMode = true;
	int lineIndex = 0 ;
	
	IcyCanvas mainCanvas;
	public int posX = 0;
	public int posY = 0;
	
	public boolean timerTaskSet = false;
    Timer timer;
    public void PreventUpdateTimer(int seconds) {
        timer = new Timer();
        timerTaskSet = true;
        timer.schedule(new TimerTestTask(), seconds*1000);
    }
    class TimerTestTask extends TimerTask {
        public void run() {
            System.out.println("In TimerTestTask, execute run method.");
            timer.cancel(); 
            timerTaskSet = false;
        }
    }
	
	public IntensityProfile(IcyCanvas mainCav,Sequence seq){
		
		sequence = seq;

		// option
		Set<String> mapValue = new HashSet<String>(); 
		mapValue.add( OPTION_meanAlongZ );
		mapValue.add( OPTION_meanAlongT );		
		
		optionComboBox = new CheckComboBox( mapValue );		
		ComponentUtil.setFixedHeight( optionComboBox , 22 );		
		
		optionComboBox.addSelectionChangedListener( new CheckComboBoxSelectionChangedListener() {
			
			@Override
			public void selectionChanged(int idx) {
				updateChart();
			}
		});
		//row or column button

        rowOColBtn.setFocusPainted(false);
        rowOColBtn.setPreferredSize(new Dimension(65, 22));
        rowOColBtn.setMaximumSize(new Dimension(80, 22));
        rowOColBtn.setMinimumSize(new Dimension(65, 22));
        rowOColBtn.setMargin(new Insets(2, 8, 2, 8));
        rowOColBtn.addActionListener(new ActionListener()
        {
            @Override
            public void actionPerformed(ActionEvent e)
            {
            	if(rowOColBtn.getText()=="row")
            	{
            		rowOColBtn.setText("column");
            		rowMode = false;	
            	}
            	else
            	{
            		rowOColBtn.setText("row");
            		rowMode = true;
            	}
            	updateChart();
            }
        });
        rowOColBtn.setToolTipText("slide in row or column");
        
        indexLbl.setHorizontalAlignment(JLabel.RIGHT );
        maxIndexLbl.setHorizontalAlignment(JLabel.RIGHT );
        
		//slide
		slider = new IcySlider(SwingConstants.HORIZONTAL);
        slider.setFocusable(false);
        slider.setMaximum(0);
        slider.setMinimum(0);
        slider.setToolTipText("Move cursor to navigate in T dimension");
        slider.addChangeListener(new ChangeListener()
        {
            @Override
            public void stateChanged(ChangeEvent e)
            {
            	try
            	{
	            	indexLbl.setText(String.valueOf(slider.getValue()));
	            	lineIndex = slider.getValue();
	            	if(rowMode)
	            	{
	            		posY = lineIndex;
	            		mainCanvas.mouseImagePositionChanged(DimensionId.Y);
	            	}
	            	else
	            	{
	            		posX = lineIndex;
	            		mainCanvas.mouseImagePositionChanged(DimensionId.X);
	            	}
            	}
            	finally
            	{
            		updateChart();            		
            	}

            
            }
        });
        ComponentUtil.setFixedHeight(slider, 22);
		 
        
		// Chart
		chart = ChartFactory.createXYLineChart(
				"", "", "intensity", xyDataset,
				PlotOrientation.VERTICAL, false, true, true);
		chartPanel = new PanningChartPanel(chart, 500, 200, 500, 200, 500, 500, false, false, true, false, true, true);		
		
		
		chartPanel.addChartMouseListener(new ChartMouseListener() {

            @Override
            public void chartMouseClicked(final ChartMouseEvent event){

            }

            @Override
            public void chartMouseMoved(final ChartMouseEvent event){
                if(!timerTaskSet){
                	PreventUpdateTimer(2);
                }
            	try
            	{
	                Point2D p = event.getTrigger().getPoint();
	                Rectangle2D plotArea = chartPanel.getScreenDataArea();
	                XYPlot plot = (XYPlot) chart.getPlot(); // your plot
	                //get the actual cordinate
	                double chartX = plot.getDomainAxis().java2DToValue(p.getX(), plotArea, plot.getDomainAxisEdge());
	            	if(rowMode)
	            	{
	            		posX = (int) chartX;
	            		mainCanvas.mouseImagePositionChanged(DimensionId.X);
	            	}
	            	else
	            	{
	            		posY = (int) chartX;
	            		mainCanvas.mouseImagePositionChanged(DimensionId.Y);
	            	}
            	}
    	        finally
    	        {
    	        	
    	        }

            }

			
        });
		
		
		mainCanvas = mainCav;
		//add to canvas
		mainCanvas.add( GuiUtil.createPageBoxPanel(optionComboBox,chartPanel,GuiUtil.createLineBoxPanel(rowOColBtn,slider,maxIndexLbl) )) ;

		// NOW DO SOME OPTIONAL CUSTOMISATION OF THE CHART...
        chart.setBackgroundPaint(Color.white);

//        final StandardLegend legend = (StandardLegend) chart.getLegend();
  //      legend.setDisplaySeriesShapes(true);
        
        // get a reference to the plot for further customisation...
        final XYPlot plot = chart.getXYPlot();
        
        plot.setBackgroundPaint(Color.lightGray);
    //    plot.setAxisOffset(new Spacer(Spacer.ABSOLUTE, 5.0, 5.0, 5.0, 5.0));

        
        // set the stroke for each series...
        plot.getRenderer().setSeriesStroke(
            0, 
            new BasicStroke(
                1.0f, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND, 
                1.0f, new float[] {1.0f, 1.0f}, 0.0f
            )
        );

        updateXYNav();
    	updateChart();
	}
	

	Runnable updateRunnable;
	
	public BufferedImage getImage()
	{

			BufferedImage objBufferedImage=chart.createBufferedImage(600,800);
			return objBufferedImage;

	}
	public void updateChart()
	{
		if(timerTaskSet)
			return;
		chart.setAntiAlias( true );
		chart.setTextAntiAlias( true );
		
	//	updateChartThreaded();
		if ( updateRunnable == null )
		{
			updateRunnable = new Runnable() {

				@Override
				public void run() {
					try
					{
						updateChartThreaded();
					}
					catch(Exception e)
					{
						
					}
				}
			};
		}
		ThreadUtil.bgRunSingle( updateRunnable );
	}
	
	private void updateChartThreaded() { 	
		
		// check if ROI still exist in a sequence
		
		removeAllHorizontalRangeMarker();
		
		if ( associatedROI != null )
		{
			if ( associatedROI.getSequences().size() == 0 )
			{
				associatedROI = null;
			}
		}
		
		// create dataSet		

		xyDataset.removeAllSeries();		

		// check z to display
		
		int currentZ = 0;
		int currentT = 0;
		
		currentT = mainCanvas.getPositionT();
		currentZ = mainCanvas.getPositionZ();
		
		if ( currentZ < 0 ) currentZ = 0; // 3D return -1.
		if ( currentT < 0 ) currentT = 0;
		
		
		if(rowMode)
		{
			if (lineIndex >sequence.getHeight()-1)
			{
				lineIndex = sequence.getHeight()-1;
				slider.setValue( lineIndex);
				return;
			}
			Point2D p1 = new Point2D.Double(0,lineIndex );
			Point2D p2 = new Point2D.Double(sequence.getWidth(),lineIndex);
			associatedROI = new ROI2DLine(p1,p2);
		}else
		{
			if (lineIndex >sequence.getWidth()-1)
			{
				lineIndex = sequence.getWidth()-1;
				slider.setValue( lineIndex);
				return;
			}
			Point2D p1 = new Point2D.Double(lineIndex, 0);
			Point2D p2 = new Point2D.Double(lineIndex, sequence.getHeight());
			associatedROI = new ROI2DLine(p1,p2);
		}
		
		sequence.addROI(associatedROI);
		
		if ( associatedROI != null )
		{

			// compute chart
	        try
	        {
				Sequence sequence = associatedROI.getSequences().get( 0 );
				if ( associatedROI instanceof ROI2DLine || associatedROI instanceof ROI2DPolyLine )
				{
					
					ROI2DShape roiShape = (ROI2DShape) associatedROI;
					ArrayList<Point2D> pointList = roiShape.getPoints();
					
					computeLineProfile( pointList, currentT, currentZ , sequence );
									
					if ( optionComboBox.isItemSelected( OPTION_meanAlongZ ) )
					{
						computeZMeanLineProfile( pointList, currentT, sequence );
					}
					if ( optionComboBox.isItemSelected( OPTION_meanAlongT ) )
					{
						computeTMeanLineProfile( pointList, currentZ , sequence );
					}
									
				}
				
				if ( associatedROI instanceof ROI2DRectangle 
						||
						associatedROI instanceof ROI2DEllipse
						||
						associatedROI instanceof ROI2DPolygon
	//					||
	//					associatedROI instanceof ROI2DArea
						)
				{
					ROI2DShape roiShape = (ROI2DShape) associatedROI;
					
					BooleanMask2D boolMask = roiShape.getAsBooleanMask();
					boolMask.intersect( new ROI2DRectangle( sequence.getBounds() ).getAsBooleanMask() );
					
					//computeSurfaceProfileAlongZ(boolMask, sequence, currentT)
					
					getValueForSurfaceAllComponent( boolMask , sequence.getImage( currentT, currentZ ) );
					//computeSurfaceProfileAlongZ( boolMask , sequence , currentT );
					if ( optionComboBox.isItemSelected( OPTION_meanAlongZ ) )
					{
						computeSurfaceProfileAlongZ( boolMask, sequence, currentT );
					}
					if ( optionComboBox.isItemSelected( OPTION_meanAlongT ) )
					{
						computeSurfaceProfileAlongT( boolMask, sequence, currentZ );
					}
				}
				
	        }
	        finally
	        {
			sequence.removeROI(associatedROI);
			associatedROI= null;
	        }
			
		}

		chart.fireChartChanged();
		//disable autorange
        final XYPlot plot = chart.getXYPlot();
        ValueAxis domainAxis = plot.getRangeAxis(0);
        domainAxis.setAutoRange(false);	
        

	}
	public void updateXYNav()
	{
		if(rowMode)
		{
			rowOColBtn.setText("row");
			slider.setMaximum(sequence.getHeight()-1);
        	slider.setMinimum(0);
        	if(slider.getValue()>sequence.getHeight()-1)
        		slider.setValue(0);
        	indexLbl.setText(String.valueOf(slider.getValue()));
        	maxIndexLbl.setText(String.valueOf(sequence.getHeight()-1));
        	
		}
		else
		{
			rowOColBtn.setText("column");
			slider.setMaximum(sequence.getWidth()-1);
        	slider.setMinimum(0);
        	if(slider.getValue()>sequence.getWidth()-1)
        		slider.setValue(0);
        	indexLbl.setText(String.valueOf(slider.getValue()));
        	maxIndexLbl.setText(String.valueOf(sequence.getWidth()-1));
		}
	}

	private void getValueForSurfaceAllComponent(BooleanMask2D boolMask, IcyBufferedImage image) {
		
		for( int c= 0 ; c < image.getSizeC() ; c++ )
		{
			XYSeries seriesXY = new XYSeries("Mean of surface c"+c );
			double value = getValueForSurface( boolMask, image , c );
			drawHorizontalSurfaceValue( value , c );

			seriesXY.add( 0, value );
			//seriesXY.add( 100, value );
			xyDataset.addSeries(seriesXY);
		}			
		
		
	}

	private void computeSurfaceProfileAlongZ(BooleanMask2D boolMask, Sequence sequence , int currentT ) {

		for( int c= 0 ; c < sequence.getSizeC() ; c++ )
		{
			XYSeries seriesXY = new XYSeries("Mean along Z c" +c );
			for ( int z = 0 ; z< sequence.getSizeZ() ; z++ )
			{
				double value = getValueForSurface( boolMask, sequence.getImage( currentT , z ) , c );				
				seriesXY.add( z , value );
			}
			xyDataset.addSeries(seriesXY);
		}			
		
	}

	private void computeSurfaceProfileAlongT(BooleanMask2D boolMask, Sequence sequence , int currentZ ) {

		for( int c= 0 ; c < sequence.getSizeC() ; c++ )
		{
			XYSeries seriesXY = new XYSeries("Mean along T c" +c + " z"+currentZ );
			for ( int t = 0 ; t< sequence.getSizeT() ; t++ )
			{
				double value = getValueForSurface( boolMask, sequence.getImage( t , currentZ ) , c );				
				seriesXY.add( t , value );
			}
			xyDataset.addSeries(seriesXY);
		}			
		
	}

	/**
	 * Mean value of the surface
	 */
	private double getValueForSurface(BooleanMask2D boolMask, IcyBufferedImage image , int component )
	{
		double result=0;
		
		double[] imageData = Array1DUtil.arrayToDoubleArray( image.getDataXY( component ) , image.isSignedDataType() );

		int minX = boolMask.bounds.x;
		int minY = boolMask.bounds.y;
		int maxX = boolMask.bounds.width + minX ;
		int maxY = boolMask.bounds.height + minY ;
		
		int imageWidth = image.getWidth();
		
		int offset = 0;
		for ( int y = minY ; y< maxY ; y++ )
		{
			for ( int x = minX ; x< maxX ; x++ )
			{
				//boolMask.contains(x, y)
				//if ( boolMask.mask[(y-minY)*width+(x-minX)] == true )
				if ( boolMask.mask[offset++] == true )
				{
					result+= imageData[y*imageWidth+x];
				}

			}
		}
		if ( offset != 0 )
		{
			result /= (double)offset;
		}
		
		
//		Array1DUtil.getValue( image.getDataXY( component ) , component, image.isSignedDataType() );
		
		
		
//		 data[component][i] = Array1DUtil.getValue( 
//       		  image.getDataXY( component ) , 
//       		  image.getOffset( (int)x, (int)y ) , 
//       		  image.isSignedDataType() ) ;
		
		return result;
	}

	private void computeZMeanLineProfile(ArrayList<Point2D> pointList, int currentT , Sequence sequence) {
		
		if ( sequence.getSizeZ() > 1 )
		{
			double [][] result = null;
			for ( int z = 0 ; z < sequence.getSizeZ() ; z++ )
			{
				Profile profile = getValueForPointList( pointList , 
						sequence.getImage( currentT , z ) );

				if ( result == null )
				{
					result = new double[profile.values.length][profile.values[0].length];
				}

				for( int c= 0 ; c < sequence.getSizeC() ; c++ )
				{
					for( int i = 0 ; i < profile.values[c].length ; i++ )
					{
						result[c][i]+= profile.values[c][i];
					}
				}					
			}
			for( int c= 0 ; c < sequence.getSizeC() ; c++ )
			{
				for( int i = 0 ; i < result[c].length ; i++ )
				{
					result[c][i] /= sequence.getSizeZ();
				}
			}

			for( int c= 0 ; c < sequence.getSizeC() ; c++ )
			{
				XYSeries seriesXY = new XYSeries("Mean along Z c" +c );		
				for( int i = 0 ; i < result[c].length ; i++ )
				{							
					seriesXY.add(i, result[c][i]);
				}						
				xyDataset.addSeries(seriesXY);
			}
		}
		
	}


	private void computeTMeanLineProfile(ArrayList<Point2D> pointList, int currentZ, Sequence sequence) {
		
		if ( sequence.getSizeT() > 1 )
		{
			double [][] result = null;
			for ( int t = 0 ; t < sequence.getSizeT() ; t++ )
			{
				Profile profile = getValueForPointList( pointList , 
						sequence.getImage( t , currentZ ) );

				if ( result == null )
				{
					result = new double[profile.values.length][profile.values[0].length];
				}

				for( int c= 0 ; c < sequence.getSizeC() ; c++ )
				{
					for( int i = 0 ; i < profile.values[c].length ; i++ )
					{
						result[c][i]+= profile.values[c][i];
					}
				}					
			}
			for( int c= 0 ; c < sequence.getSizeC() ; c++ )
			{
				for( int i = 0 ; i < result[c].length ; i++ )
				{
					result[c][i] /= sequence.getSizeT();
				}
			}

			for( int c= 0 ; c < sequence.getSizeC() ; c++ )
			{
				XYSeries seriesXY = new XYSeries("Mean along T c" +c );		
				for( int i = 0 ; i < result[c].length ; i++ )
				{							
					seriesXY.add(i, result[c][i]);
				}						
				xyDataset.addSeries(seriesXY);
			}
		}
		
	}
	
	private void computeLineProfile(ArrayList<Point2D> pointList, int currentT, int currentZ, Sequence sequence) {
				
		Profile profile = getValueForPointList( pointList , sequence.getImage( currentT , currentZ ) );

		drawVerticalROIBreakBar( profile );

		for( int c= 0 ; c < sequence.getSizeC() ; c++ )
		{
			XYSeries seriesXY = new XYSeries("Intensity c" +c + " t"+currentT + " z" +currentZ );		
			for( int i = 0 ; i < profile.values[c].length ; i++ )
			{							
				//System.out.println();
				seriesXY.add(i, profile.values[c][i]);
			}
			xyDataset.addSeries(seriesXY);
		}


		
	}

	private void drawVerticalROIBreakBar(Profile profile) {

		final XYPlot plot = chart.getXYPlot();

		for ( Marker marker : markerDomainList )
		{
			plot.removeDomainMarker(marker);
		}
		
		if ( profile.roiLineBreaks.size() <=1 ) return;
		
		int nb = 1;
		for ( Integer i : profile.roiLineBreaks )
		{
			final Marker start = new ValueMarker( i );
			markerDomainList.add( start );
			start.setPaint(Color.black);
			start.setLabel( "" + nb );
			start.setLabelAnchor(RectangleAnchor.TOP_RIGHT);
			start.setLabelTextAnchor(TextAnchor.TOP_LEFT);
			plot.addDomainMarker(start);
			nb++;
		}
		
	}

	void removeAllHorizontalRangeMarker()
	{
		ThreadUtil.invokeNow( new Runnable() {
			
			@Override
			public void run() {

				final XYPlot plot = chart.getXYPlot();
				for ( Marker marker : markerRangeList )
				{
					plot.removeRangeMarker(marker);
				}		
				
			}
		} );
	}
	private void drawHorizontalSurfaceValue( final double value , final int c ) {

		ThreadUtil.invokeNow( new Runnable() {
			
			@Override
			public void run() {

				final XYPlot plot = chart.getXYPlot();
				
				final Marker start = new ValueMarker( value );
				markerRangeList.add( start );
				start.setLabel("channel "+c + " mean value: " +value );
				
				start.setPaint(Color.black);
				switch ( c ) {
				case 0:
					start.setPaint(Color.red);
					start.setLabelAnchor(RectangleAnchor.BOTTOM_LEFT);
					start.setLabelTextAnchor(TextAnchor.TOP_LEFT);
					break;
				case 1:
					start.setPaint(Color.green.darker() );	
					start.setLabelAnchor(RectangleAnchor.BOTTOM_LEFT);
					start.setLabelTextAnchor(TextAnchor.TOP_LEFT);
					break;
				case 2:
					start.setPaint(Color.blue);	
					start.setLabelAnchor(RectangleAnchor.BOTTOM_LEFT);
					start.setLabelTextAnchor(TextAnchor.TOP_LEFT);
					break;
				}
				start.setLabelPaint( start.getPaint() );
				plot.addRangeMarker(start);
			}
		});
		
		
	}
	
	class Profile
	{
		double[][] values;
		ArrayList<Integer> roiLineBreaks = new ArrayList<Integer>();	
	}
	
	private Profile getValueForPointList( ArrayList<Point2D> pointList , IcyBufferedImage image ) {
				
		ArrayList<double[][]> dataList = new ArrayList<double[][]>();
		ArrayList<Integer> roiLineBreaks = new ArrayList<Integer>();
		
		int indexSize = 0;
		
		for ( int i = 0 ; i < pointList.size() -1 ; i++ )
		{
			double dataTmp[][] = getValueFor1DSegment ( pointList.get( i ), pointList.get( i +1 ) , image ) ;
			indexSize+= dataTmp[0].length;
			dataList.add( dataTmp );
			roiLineBreaks.add( indexSize );
		}
		
		double data[][] = new double[image.getSizeC()][indexSize];
		
		int index = 0;
		for ( double[][] dataToAdd : dataList )
		{
			for ( int c = 0 ; c < image.getSizeC() ; c++ )
			{
				for ( int i = 0 ; i< dataToAdd[0].length ; i++ )
				{
					data[c][index+i] = dataToAdd[c][i];
				}
			}
			index+=dataToAdd[0].length;
		}
		
		Profile profile = new Profile();
		profile.values = data;
		profile.roiLineBreaks = roiLineBreaks;
		return profile;

	}
	
	private double[][] getValueFor1DSegment( Point2D p1, Point2D p2 , IcyBufferedImage image ) {
		
        int distance = (int) p1.distance( p2 );

        double vx = ( p2.getX() - p1.getX() ) / (double)distance;
        double vy = ( p2.getY() - p1.getY() ) / (double)distance;

        int nbComponent= image.getSizeC();
        double[][] data = new double[nbComponent][distance];

        double x = p1.getX();
        double y = p1.getY();

        for ( int i = 0 ; i< distance ; i++ )
        {
                   //IcyBufferedImage image = canvas.getCurrentImage();
                   if ( image.isInside( (int)x, (int)y ) )
                   {
                	   	for ( int component = 0 ; component < nbComponent ; component ++ )
                	   	{
                              data[component][i] = Array1DUtil.getValue( 
                            		  image.getDataXY( component ) , 
                            		  image.getOffset( (int)x, (int)y ) , 
                            		  image.isSignedDataType() ) ;
                	   	}     
                              
                   }else
                   {
                	   for ( int component = 0 ; component < nbComponent ; component ++ )
                	   {
                		   data[component][i] = 0 ;
                	   }
                   }

                   x+=vx;
                   y+=vy;
        }
        
        return data;

	}



//	@Override
//	public void actionPerformed(ActionEvent e) {
//		
//		if ( e.getSource() == exportToExcelButton )
//		{
//			System.out.println("Export to excel.");
//		}
//		
//		if ( e.getSource() == associateROIButton )
//		{
//			// remove previous listener.
//					
//			updateChart();
//		}
//
//	}
//
//	@Override
//	public void roiChanged(ROIEvent event) {		
//		
//		updateChart();
//	
//	}




	
	
	
	
	

}
