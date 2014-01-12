package plugins.oeway;

import java.io.File;
import javax.swing.JFileChooser;

import org.dcm4che2.data.DicomObject;
import icy.plugin.abstract_.PluginActionable;
import icy.sequence.Sequence;
/**
 * 
 * DICOM/DICONDE file format import plugin, 8bit/16bit DCM image can be open with this class,
 * Only 16bit image tested.
 * 
 * @author Will Ouyang
 */
public class DcmImporter extends PluginActionable {
	final JFileChooser fc = new JFileChooser();

	@Override
	public void run() {

		int returnVal = fc.showOpenDialog(null);
        if (returnVal == JFileChooser.APPROVE_OPTION) 
        {
            File file = fc.getSelectedFile();
            try
            {
            	//read image
            	Sequence seq = DcmImg.readImage(file);
				if(seq!=null)
				{
					addSequence(seq);
					//read head
					DicomObject dcmObj= DcmImg.readHead(file);
				    String name = DcmImg.getHeaderInfo(dcmObj);
				    seq.setName(name);
				    DcmImg.printHeaders(dcmObj);
				}
            }
            catch(Exception e)
            {
            	System.out.print(e.toString());
            	
            }
        } 
	        

	}	


}
