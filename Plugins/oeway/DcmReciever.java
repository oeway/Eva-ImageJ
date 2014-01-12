package plugins.oeway;
import java.io.File;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.prefs.Preferences;

import plugins.adufour.ezplug.EzPlug;
import plugins.adufour.ezplug.EzStoppable;
import plugins.adufour.ezplug.EzVar;
import plugins.adufour.ezplug.EzVarBoolean;
import plugins.adufour.ezplug.EzVarFolder;
import plugins.adufour.ezplug.EzVarText;
import plugins.adufour.vars.lang.Var;
import icy.gui.dialog.MessageDialog;

/**
 * DICOM/DICONDE file reciever, originally work with GE Rhythm RT scanner
 * .DCM image can be sent into this reciever, file can be stored in a folder and open in Icy.
 * 
 * @author Will Ouyang
 */
public class DcmReciever extends EzPlug implements EzStoppable{
	EzVarFolder					varRecieveFolder;
	EzVarText					varTextAEtitle;
	EzVarText					varTextIP;
	EzVarText					varTextPort;
	EzVarBoolean				varBooleanOpen;
	DcmRcv dcmRcv;
	boolean stopFlag;
	Preferences _prefs;
	@Override
	protected void initialize() {
		varRecieveFolder = new EzVarFolder("Recieve Folder", null);
		varTextAEtitle = new EzVarText("AE Title", new String[] { "EVASCP"}, 0, true);
		varTextIP = new EzVarText("Host IP", new String[] { "127.0.0.1"}, 0, true);
		varTextPort = new EzVarText("Host Port", new String[] { "104"}, 0, true);
		varBooleanOpen = new EzVarBoolean("Open Immediately",true);
		super.addEzComponent(varRecieveFolder);
		super.addEzComponent(varTextAEtitle);
		super.addEzComponent(varTextIP);
		super.addEzComponent(varTextPort);
		super.addEzComponent(varBooleanOpen);
		Preferences root = Preferences.userNodeForPackage(getClass());
	    _prefs = root.node(root.absolutePath() + "/prefs_dcm_reciever");
	    File folder = new File(_prefs.get("recieveFolder", ""));
	    varRecieveFolder.setValue(folder);
	    varRecieveFolder.valueChanged((Var<File>)null, folder, folder);
	    varTextAEtitle.setValue(_prefs.get("AEtitle", "EVASCP"));
	    varTextPort.setValue(_prefs.get("HostPort", "104"));
	    
	    //get host ip list and fill it to the text box
	    String storedIp = _prefs.get("HostIP", "127.0.0.1");
	    String ipaddr ="127.0.0.1";
	    ArrayList<String> ipList = new ArrayList<String>();
	    try {
        	Enumeration<NetworkInterface> e=NetworkInterface.getNetworkInterfaces();
            while(e.hasMoreElements())
            {
                NetworkInterface n=(NetworkInterface) e.nextElement();
                Enumeration<InetAddress> ee = n.getInetAddresses();
                while(ee.hasMoreElements())
                {
                    InetAddress i= (InetAddress) ee.nextElement();
                    if(storedIp.equals(i.getHostAddress()))
                    {
                    	ipaddr = storedIp;
                    }
                    ipList.add(i.getHostAddress());
                }
            }
        } catch (Exception e) {
            System.out.println("Error getting User IP Address - " + e.toString());
        }
	    String ipaddrArr[];
	    ipaddrArr = new String[ipList.size()];
	    for(int i=0;i<ipList.size();i++)
	    	ipaddrArr[i] = ipList.get(i);
	    
	    varTextIP.setValue(ipaddr);
	    varTextIP.setDefaultValues(ipaddrArr, 0, false);    
	}
	
	@Override
	public void clean() {

	}

	@Override
	protected void execute() {
		varBooleanOpen.setEnabled(false);
		varTextPort.setEnabled(false);
		varTextIP.setEnabled(false);
		varTextAEtitle.setEnabled(false);
		varRecieveFolder.setEnabled(false);
          try {
        	  if(varRecieveFolder.getValue()== null)
        	  {
        		  MessageDialog.showDialog("Please select RecieveFolder.", MessageDialog.ERROR_MESSAGE);
        		  return;
        	  }
        	  dcmRcv = DcmRcv.setSimpleDcmRcv(varTextIP.getValue(),varTextPort.getValue(),varTextAEtitle.getValue(),varRecieveFolder.getValue().getAbsolutePath());
        	  DcmRcv.openOnRecieve = varBooleanOpen.getValue();
        	  if(dcmRcv != null)
        	  {
        		  dcmRcv.start();
        		  try
        		  {
        			  _prefs.put("recieveFolder", varRecieveFolder.getValue().getAbsolutePath());
        			  _prefs.put("AEtitle", varTextAEtitle.getValue());
        			  _prefs.put("HostPort", varTextPort.getValue());
        			  _prefs.put("HostIP", varTextIP.getValue());
        		  }
        		  catch(Exception e)
        		  {
        			  System.out.println(e.toString());
        		  }
	        	  stopFlag = false;
	        	  while(!stopFlag)
	        	  {
	        		  Thread.sleep(100);
	        	  }
	        	  dcmRcv.stop();
        	  }
          }
          catch(Exception e)
          {
        	  System.out.println(e.toString());
        	  MessageDialog.showDialog("Error occured.", MessageDialog.ERROR_MESSAGE);
          }
      	varBooleanOpen.setEnabled(true);
		varTextPort.setEnabled(true);
		varTextIP.setEnabled(true);
		varTextAEtitle.setEnabled(true);
		varRecieveFolder.setEnabled(true);
	}

	@Override
	public void stopExecution() {
		stopFlag = true;
	}



}
