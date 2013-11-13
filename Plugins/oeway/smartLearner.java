package plugins.oeway;

import icy.sequence.Sequence;

import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.IOException;

import com.rapidminer.RapidMiner;
import com.rapidminer.Process;
import com.rapidminer.operator.IOContainer;
import com.rapidminer.operator.MissingIOObjectException;
import com.rapidminer.operator.Model;
import com.rapidminer.operator.Operator;
import com.rapidminer.operator.OperatorCreationException;
import com.rapidminer.operator.OperatorException;
import com.rapidminer.operator.io.CSVExampleSetWriter;
import com.rapidminer.operator.performance.PerformanceVector;
import com.rapidminer.tools.OperatorService;
import com.rapidminer.tools.XMLException;

import plugins.adufour.ezplug.EzButton;
import plugins.adufour.ezplug.EzGroup;
import plugins.adufour.ezplug.EzPlug;
import plugins.adufour.ezplug.EzStoppable;
import plugins.adufour.ezplug.EzVarFile;
import plugins.adufour.ezplug.EzVarSequence;
import plugins.nherve.maskeditor.MaskEditor;
import plugins.nherve.toolbox.image.BinaryIcyBufferedImage;
import plugins.nherve.toolbox.image.mask.Mask;
import plugins.nherve.toolbox.image.mask.MaskException;
import plugins.nherve.toolbox.image.mask.MaskStack;

import com.rapidminer.example.*;
import com.rapidminer.example.table.*;
import com.rapidminer.example.set.*;
import com.rapidminer.tools.Ontology;
import com.rapidminer.tools.math.Averagable;

import java.util.*;

import javax.swing.JButton;
import javax.swing.JFileChooser;
public class smartLearner extends EzPlug implements EzStoppable, ActionListener
{

	EzVarFile					varTrainXmlFile;
	EzVarFile					varPredictXmlFile;
	
	
	EzVarSequence				varPredictSequence;
	EzVarSequence				varTrainSequence;
	EzButton 					varTrainBtn;
	EzButton 					varPredictBtn;
	
	EzButton 					varExportBtn;
	
	
	IOContainer modelContainer;
	 public void export(String filePath) throws OperatorCreationException
	 {
			Sequence sequence = varTrainSequence.getValue();
			MaskEditor me = MaskEditor.getRunningInstance(true);
			MaskStack allStack = me.getBackupObject();
			MaskStack classStack = new MaskStack();
			classStack.beginUpdate();
			for(Mask m : allStack)
			{
				if(!m.getLabel().contains("predict"))
					classStack.add(m);
			}
			classStack.endUpdate();
			
			int numOfAttr =sequence.getSizeC()*sequence.getSizeZ()*sequence.getSizeT();
			// create attribute list
		    List<Attribute> attributes = new LinkedList<Attribute>();
		    for (int a = 0; a < numOfAttr; a++) {
		      attributes.add(AttributeFactory.createAttribute("att" + a, 
		                                                      Ontology.REAL));
		    }
		    Attribute label = AttributeFactory.createAttribute("label", 
		                                                       Ontology.NOMINAL);
		    attributes.add(label);
				
		    // create table
		    MemoryExampleTable table = new MemoryExampleTable(attributes);
		    
		    // fill table (here: only real values)
		    for(int x=0;x<sequence.getWidth();x++)
		    	for(int y=0;y<sequence.getHeight();y++)
		    	{
				    for(Mask m: classStack)
				    {
					    if(m.contains(x,y))
					    {
					    	
						      double[] data = new double[attributes.size()];
						      int i =0;
						      for(int t=0;t<sequence.getSizeT();t++)
						    	{
						    	  for(int z=0;z<sequence.getSizeZ();z++)
						    		  for(int c=0;c<sequence.getSizeC();c++)
						    		  {
						    			  // fill with proper data here
						    			  data[i] = sequence.getData(t, z, c, y, x);
						    			  i++;
						    		  }
						    	}	
						      // maps the nominal classification to a double value
						      data[data.length - 1] = 
						          label.getMapping().mapString(m.getLabel());
						          
						      // add data row
						      table.addDataRow(new DoubleArrayDataRow(data));
						      break;
					    	
					    }
				    }
		    	}
				
		    // create example set
		    ExampleSet exampleSet = table.createExampleSet(label);
		    
			Process process;
			try {
				process = new Process();
			    // create a process
				Operator dataCSVWriter = OperatorService.createOperator(CSVExampleSetWriter.class);
				dataCSVWriter.setParameter(CSVExampleSetWriter.PARAMETER_CSV_FILE,filePath);
			    process.getRootOperator().getSubprocess(0).addOperator(dataCSVWriter);
			    process.getRootOperator().getSubprocess(0).getInnerSources().getPortByIndex(0).connectTo( dataCSVWriter.getInputPorts().getPortByName("input"));	
			    // run the process with new IOContainer using the created exampleSet	   
			    
				modelContainer = process.run(new IOContainer(exampleSet)); 
			    
			} catch (OperatorException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} 
		 
	 }
	public void train()
	{
		Sequence sequence = varTrainSequence.getValue();
		MaskEditor me = MaskEditor.getRunningInstance(true);
		MaskStack allStack = me.getBackupObject();
		MaskStack classStack = new MaskStack();
		classStack.beginUpdate();
		for(Mask m : allStack)
		{
			if(!m.getLabel().contains("predict"))
				classStack.add(m);
		}
		classStack.endUpdate();
		
		int numOfAttr =sequence.getSizeC()*sequence.getSizeZ()*sequence.getSizeT();
		// create attribute list
	    List<Attribute> attributes = new LinkedList<Attribute>();
	    for (int a = 0; a < numOfAttr; a++) {
	      attributes.add(AttributeFactory.createAttribute("att" + a, 
	                                                      Ontology.REAL));
	    }
	    Attribute label = AttributeFactory.createAttribute("label", 
	                                                       Ontology.NOMINAL);
	    attributes.add(label);
			
	    // create table
	    MemoryExampleTable table = new MemoryExampleTable(attributes);
	    
	    // fill table (here: only real values)
	    for(int x=0;x<sequence.getWidth();x++)
	    	for(int y=0;y<sequence.getHeight();y++)
	    	{
			    for(Mask m: classStack)
			    {
				    if(m.contains(x,y))
				    {
				    	
					      double[] data = new double[attributes.size()];
					      int i =0;
					      for(int t=0;t<sequence.getSizeT();t++)
					    	{
					    	  for(int z=0;z<sequence.getSizeZ();z++)
					    		  for(int c=0;c<sequence.getSizeC();c++)
					    		  {
					    			  // fill with proper data here
					    			  data[i] = sequence.getData(t, z, c, y, x);
					    			  i++;
					    		  }
					    	}	
					      // maps the nominal classification to a double value
					      data[data.length - 1] = 
					          label.getMapping().mapString(m.getLabel());
					          
					      // add data row
					      table.addDataRow(new DoubleArrayDataRow(data));
					      break;
				    	
				    }
			    }
	    	}
			
	    // create example set
	    ExampleSet exampleSet = table.createExampleSet(label);
	    
		Process process;
		try {
			process = new Process(varTrainXmlFile.getValue());
		    // create a process
		    //process.getRootOperator().getSubprocess(0).addOperator(someOperator);
		    //process.getRootOperator().getSubprocess(0).getInnerSources().getPortByIndex(0).connectTo( someOperator.getInputPorts().getPortByName("input"));	
		    // run the process with new IOContainer using the created exampleSet	   
		    
			modelContainer = process.run(new IOContainer(exampleSet)); 
		    
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (XMLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (OperatorException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	public void predict() throws MaskException, MissingIOObjectException
	{
		Sequence sequence = varPredictSequence.getValue();
		MaskEditor me = MaskEditor.getRunningInstance(true);
		MaskStack stack = me.getBackupObject();
		
		int numOfAttr =sequence.getSizeC()*sequence.getSizeZ()*sequence.getSizeT();
		// create attribute list
	    List<Attribute> attributes = new LinkedList<Attribute>();
	    for (int a = 0; a < numOfAttr; a++) {
	      attributes.add(AttributeFactory.createAttribute("att" + a, 
	                                                      Ontology.REAL));
	    }
	    Attribute label = AttributeFactory.createAttribute("label", 
	                                                       Ontology.NOMINAL);
	    attributes.add(label);
			
	    // create table
	    MemoryExampleTable table = new MemoryExampleTable(attributes);
	    
	    // fill table (here: only real values)
	    for(int x=0;x<sequence.getWidth();x++)
	    	for(int y=0;y<sequence.getHeight();y++)
	    	{

			      double[] data = new double[attributes.size()];
			      int i =0;
			      for(int t=0;t<sequence.getSizeT();t++)
			    	{
			    	  for(int z=0;z<sequence.getSizeZ();z++)
			    		  for(int c=0;c<sequence.getSizeC();c++)
			    		  {
			    			  // fill with proper data here
			    			  data[i] = sequence.getData(t, z, c, y, x);
			    			  i++;
			    		  }
			    	}	
			      // maps the nominal classification to a double value
			      //data[data.length - 1] = 
			      //    label.getMapping().mapString(m.getLabel());
			          
			      // add data row
			      table.addDataRow(new DoubleArrayDataRow(data));
	    	}
			
	    // create example set
	    ExampleSet exampleSet = table.createExampleSet(label);
	    
	
	    Model model = modelContainer.get(Model.class);
	    
		Process process;
		try {
			process = new Process(varPredictXmlFile.getValue());
		    // create a process
		    //process.getRootOperator().getSubprocess(0).addOperator(someOperator);
		    //process.getRootOperator().getSubprocess(0).getInnerSources().getPortByIndex(0).connectTo( someOperator.getInputPorts().getPortByName("input"));	
		    // run the process with new IOContainer using the created exampleSet	   
		    
			IOContainer	predictDataContainer = process.run(new IOContainer(model,exampleSet)); 
		 	ExampleSet predictDataSet = predictDataContainer.get(ExampleSet.class);
		 
		    stack.beginUpdate();
		 	for(int i=0;i<stack.size();i++)
		 	{
		 		Mask m = stack.getByIndex(i);
		 		if(m.getLabel().contains("predict"))
		 			stack.remove(m);
		 		
		 	}
			 for(int i=0;i<predictDataSet.size();i++)
			 {
				 Example e =predictDataSet.getExample(i);
				 String predictlabel = predictDataSet.getAttributes().getPredictedLabel().getMapping().mapIndex((int) e.getPredictedLabel());
				 
				Mask m = stack.getByLabel("predict_"+predictlabel);
				if(m ==null)
				{
					m = stack.createNewMask("predict_"+predictlabel, false, Color.BLUE, MaskEditor.getRunningInstance(false).getGlobalOpacity());
				
				}
				byte[] bb = m.getBinaryData().getRawData();
					bb[i] =  BinaryIcyBufferedImage.TRUE ;
			 }

			 for(Mask m: stack)
				 m.getBinaryData().dataChanged();
				
				stack.endUpdate();
		    
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (XMLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (OperatorException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	@Override
	public void stopExecution() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void clean() {
		// TODO Auto-generated method stub
		
	}

	
	@Override
	protected void execute() {

		
	}

	@Override
	protected void initialize() {
		varTrainXmlFile = new EzVarFile("training xml file", null);
		varPredictXmlFile = new EzVarFile("predicting xml file", null);
		varTrainSequence = new EzVarSequence("Training sequence");
		varPredictSequence = new EzVarSequence("Predicting sequence");
		varPredictBtn = new EzButton("predict",this);
		varTrainBtn = new EzButton("Train",this);
		varExportBtn = new EzButton("Export",this);
		EzGroup trainGroup = new EzGroup("Train",varExportBtn,varTrainXmlFile,varTrainSequence,varTrainBtn);
		EzGroup predictGroup = new EzGroup("Predict",varPredictXmlFile,varPredictSequence,varPredictBtn);
		
		super.addEzComponent(trainGroup);
		super.addEzComponent(predictGroup);
		
		
		
		/*
		 * 
		 * You can also use the simple method RapidMiner.init() and configure the settings via this list of environment variables:
			rapidminer.init.operators (file name)
			rapidminer.init.plugins.location (directory name)
			rapidminer.init.weka (boolean)
			rapidminer.init.jdbc.lib (boolean)
			rapidminer.init.jdbc.classpath (boolean)
			rapidminer.init.plugins (boolean)
		 */
		RapidMiner.init();

	}

	@Override
	public void actionPerformed(ActionEvent e) {
			if (((JButton)e.getSource()).getText().equals(varTrainBtn.name))
			{
				train();
			}
			else if (((JButton)e.getSource()).getText().equals(varPredictBtn.name))
			{
				try {
					predict();
				} catch (MissingIOObjectException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				} catch (MaskException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
			}
			else if (((JButton)e.getSource()).getText().equals(varExportBtn.name))
			{
				try {
					final JFileChooser fc = new JFileChooser();
					int returnVal = fc.showOpenDialog(null);

			        if (returnVal == JFileChooser.APPROVE_OPTION) {
			            File file = fc.getSelectedFile();
			            export(file.getAbsolutePath());
			        }
				} catch (OperatorCreationException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
			}
			
		}
		
}
