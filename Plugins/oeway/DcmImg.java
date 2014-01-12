package plugins.oeway;

import icy.sequence.Sequence;

import java.awt.image.BufferedImage;
import java.awt.image.DataBuffer;
import java.awt.image.Raster;
import java.io.File;
import java.io.IOException;
import java.util.Date;
import java.util.Iterator;

import javax.imageio.ImageReader;
import javax.imageio.stream.FileImageInputStream;

import org.dcm4che2.data.DicomElement;
import org.dcm4che2.data.DicomObject;
import org.dcm4che2.imageio.plugins.dcm.DicomImageReadParam;
import org.dcm4che2.imageioimpl.plugins.dcm.DicomImageReader;
import org.dcm4che2.imageioimpl.plugins.dcm.DicomImageReaderSpi;
import org.dcm4che2.io.DicomInputStream;
import org.dcm4che2.util.TagUtils;
/**
 * 
 * DICOM/DICONDE file format import class, 8bit/16bit DCM image can be open with this class,
 * Only 16bit image tested.
 * 
 * @author Will Ouyang
 */
public class DcmImg {
	public static DicomObject readHead(File file)
	{
		DicomObject dcmObj;
		DicomInputStream din = null;
		try {
		    din = new DicomInputStream(file);
		    dcmObj = din.readDicomObject();

		}
		catch (IOException e) {
		    e.printStackTrace();
		    dcmObj = null;
		}
		finally {
		    try {
		        din.close();
		    }
		    catch (IOException ignore) {
		    }
		}
		return dcmObj;
	}
	public static String getHeaderInfo(DicomObject dcmObj) {
		Date d = new Date();
		String patientName = "-";
		String creationDate = d.toString() ;
		String creationTime = "-";
		Iterator<DicomElement> iter = dcmObj.datasetIterator();
		while (iter.hasNext()) {
		    DicomElement element = iter.next();
		    int tag = element.tag();
		    try 
		    {
		        String tagName = dcmObj.nameOf(tag);
		        String tagAddr = TagUtils.toString(tag);
		        String tagVR = dcmObj.vrOf(tag).toString();
		        String tagValue = dcmObj.getString(tag);
		        if(tagAddr.equals("(0010,0010)")) {
		        	patientName = tagValue;
				}
				else if(tagAddr.equals("(0008,0012)"))
				{
					creationDate = tagValue;
				}
				else if(tagAddr.equals("(0008,0013)"))
				{
					creationTime = tagValue;
				}
		        
			} catch (Exception e) {
			    e.printStackTrace();
			}
		    
	    }
		return patientName+"-"+creationDate+"-"+creationTime;
	  }
	 public static void printHeaders(DicomObject dcmObj) {
	        Iterator<DicomElement> iter = dcmObj.datasetIterator();
	        while (iter.hasNext()) {
	            DicomElement element = iter.next();
	            int tag = element.tag();
	            try {
	                String tagName = dcmObj.nameOf(tag);
	                String tagAddr = TagUtils.toString(tag);
	                String tagVR = dcmObj.vrOf(tag).toString();
	                if (tagVR.equals("SQ")) {
	                    if (element.hasItems()) {
	                        System.out.println(tagAddr + " [" + tagVR + "] " + tagName);
	                        printHeaders(element.getDicomObject());
	                        continue;
	                    }
	                }
	                String tagValue = dcmObj.getString(tag);
	                System.out.println(tagAddr + " [" + tagVR + "] " + tagName + " [" + tagValue + "]");
	            } catch (Exception e) {
	                e.printStackTrace();
	            }
	        }
	    }
	 public static Sequence readImage(File file)
	 {
		Sequence seq= new Sequence();
		try {
			System.out.println("Reading DICOM image...");	
			Raster raster = null;
			 try {
				 
                 FileImageInputStream fis = null;
                 int frameCount = -1;
                 System.out.println("processing of file");
                 fis = new FileImageInputStream(file);
                 DicomImageReader codec = (DicomImageReader) new DicomImageReaderSpi().createReaderInstance();
                if (codec == null)
                 {
                	fis.close();
                    throw new IOException("Unable to create codec");
                 }
                 codec.setInput(fis);
                 frameCount = codec.getNumImages(true);
                 System.out.println("Number of frames is " +
                                 Integer.toString(frameCount));
     			try
     			{
     	           DicomImageReadParam param = (DicomImageReadParam) codec.getDefaultReadParam();
                   raster = codec.readRaster(0, param);
                   int dataType = raster.getDataBuffer().getDataType();
                   System.out.println("Data type is " + Integer.toString(dataType));
                   System.out.println("Data type size is " + Integer.toString(DataBuffer.getDataTypeSize(dataType)));		
	       			System.out.println("Extracting frames...");
	     			int imgDataType = BufferedImage.TYPE_BYTE_GRAY;
	     			switch (dataType)
	     			{
	     				case DataBuffer.TYPE_BYTE: 
	     					imgDataType = BufferedImage.TYPE_BYTE_GRAY;
	     					break; 
	     				case DataBuffer.TYPE_USHORT:
	     					imgDataType = BufferedImage.TYPE_USHORT_GRAY;
	     					break;
	     				default: throw new IllegalArgumentException("Unknown data buffer type: "+ dataType); 
	     			}
	     				
	     			for (int i=0; i < frameCount; i++) {
	     				raster = codec.readRaster(i, param);
	     				BufferedImage bufferedImage = new BufferedImage(raster.getWidth(), raster.getHeight(),imgDataType);
	     		        bufferedImage.setData(raster);	
	     				//BufferedImage img = reader.read(i);
	     				seq.addImage(bufferedImage);
	     				System.out.println(" > Frame "+ (i+1));
	     			}	
     			}
     			catch(Exception e2)
     			{
     				ImageReader reader = new DicomImageReaderSpi().createReaderInstance();
     				FileImageInputStream input = new FileImageInputStream(file);
     				reader.setInput(input);			
     				int numFrames = reader.getNumImages(true);
     				System.out.println("DICOM image has "+ numFrames +" frames...");			
     				System.out.println("Extracting frames...");
     				for (int i=0; i < numFrames; i++) {
     					BufferedImage img = reader.read(i);
     					seq.addImage(img);
     					System.out.println(" > Frame "+ (i+1));
     				}	
     			}
			 } catch (Exception e) {
                 System.out.println("Error reading raster image");
                 System.out.println(e.getClass().getName() + " " + e.getMessage());
         }

			System.out.println("Finished.");
			System.gc();
			return seq;
		} catch(Exception e) {
				e.printStackTrace();
				return null;
		}
	 }
}
