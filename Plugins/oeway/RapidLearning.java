package plugins.oeway;

import icy.gui.frame.progress.AnnounceFrame;
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
import com.rapidminer.tools.OperatorService;
import com.rapidminer.tools.XMLException;

import plugins.adufour.ezplug.EzButton;
import plugins.adufour.ezplug.EzGroup;
import plugins.adufour.ezplug.EzPlug;
import plugins.adufour.ezplug.EzStoppable;
import plugins.adufour.ezplug.EzVarEnum;
import plugins.adufour.ezplug.EzVarFile;
import plugins.adufour.ezplug.EzVarSequence;
import plugins.nherve.maskeditor.MaskEditor;
import plugins.nherve.toolbox.image.mask.Mask;
import plugins.nherve.toolbox.image.mask.MaskException;
import plugins.nherve.toolbox.image.mask.MaskStack;

import com.rapidminer.example.*;
import com.rapidminer.example.table.*;
import com.rapidminer.tools.Ontology;

import java.util.*;

import javax.swing.JButton;
import javax.swing.JFileChooser;

/**
 * RapidLearning uses RapidMiner as a machine learning engine to learn model from image data
 * This plugin can do supervised learning and unsupervised learning with image data, Z,T,C dimension will be used as features,
 * use MaskEditor to define labels, more than 2 labels can be defined. 
 * 
 * 
 * In order to get this plugin work, you should install RapidMiner first. Put all library of RapidMiner to the Plugins directory will work.
 * 
 * @author Will Ouyang 
 * 
 */
public class RapidLearning extends EzPlug implements EzStoppable, ActionListener
{

	private enum StageEnumeration
	{
		Training, Predicting
	}
	private enum LearningTypeEnumeration
	{
		Supervised, Unsupervised
	}
	EzVarFile					varTrainXmlFile;
	EzVarFile					varPredictXmlFile;
	
	
	EzVarSequence				varPredictSequence;
	EzVarSequence				varTrainSequence;
	
	EzButton 					varExportBtn;
	EzVarEnum<LearningTypeEnumeration>	varLearningType;
	EzVarEnum<StageEnumeration>	varStage;
	
	boolean stopFlag;
	IOContainer trainingOutputContainer;
	
	
    public Color getRandomColor()
    {
    	Random random = new Random();
    	final float hue = random.nextFloat();
    	final float saturation = (random.nextInt(1000) + 4000) / 5000f;
    	final float luminance = 0.9f;
    	final Color clr = Color.getHSBColor(hue, saturation, luminance);
    	return clr;
    }
    public ExampleSet generateExampleSet(Sequence sequence,boolean labelled, MaskStack allStack)
    {
		MaskStack maskStack =null;
		if(allStack!=null)
		{
			maskStack = new MaskStack();
			maskStack.beginUpdate();
			for(Mask m : allStack)
			{
				if(!m.getLabel().contains("predict") )
					maskStack.add(m);
			}
			maskStack.endUpdate();
		}
		// create attribute list
	    List<Attribute> attributes = new LinkedList<Attribute>();
	    for(int t=0;t<sequence.getSizeT();t++)
    	{
    	  for(int z=0;z<sequence.getSizeZ();z++)
    		  for(int c=0;c<sequence.getSizeC();c++)
    		  {
    			  attributes.add(AttributeFactory.createAttribute("t"+t +"z"+z+"c"+c , 
	                                                      Ontology.REAL));
    		  }
	    }
	    Attribute xAttr = AttributeFactory.createAttribute("x", Ontology.REAL);
	    Attribute yAttr = AttributeFactory.createAttribute("y", Ontology.REAL);
	    Attribute label = AttributeFactory.createAttribute("label",Ontology.NOMINAL);
	    
	    int xIndex = attributes.size();
	    attributes.add(xAttr);
	    int yIndex = attributes.size();
	    attributes.add(yAttr);
	    int labelIndex = attributes.size();
	    if(labelled)
	    	attributes.add(label);
	    
			
	    // create table
	    MemoryExampleTable table = new MemoryExampleTable(attributes);
	    
	    
	    // fill table (here: only real values)
	    if(maskStack != null)
	    {
	    	for(int y=0;y<sequence.getHeight();y++)
	    	for(int x=0;x<sequence.getWidth();x++)
	    	{
			    for(Mask m: maskStack)
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
					      
					      data[xIndex] = x;
					      data[yIndex] = y;
					      if(labelled)
					      {
					    	  // maps the nominal classification to a double value
					    	  data[labelIndex] = 
					          label.getMapping().mapString(m.getLabel());
					      }
					      // add data row
					      table.addDataRow(new DoubleArrayDataRow(data));
					      
				    	
				    }
			    }
	    	}
	    }
	    else  // no mask available
	    {
		    for(int y=0;y<sequence.getHeight();y++)
		    	for(int x=0;x<sequence.getWidth();x++)
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
				      data[xIndex] = x;
				      data[yIndex] = y;
				      // add data row
				      table.addDataRow(new DoubleArrayDataRow(data));
		    	}
	    }
	    // create example set
	    ExampleSet exampleSet;
	    if(labelled)
	    {
	    	exampleSet= table.createExampleSet(label);
	    }
	    else
	    {
	    	exampleSet= table.createExampleSet();
	    } 	
	    exampleSet.getAttributes().setSpecialAttribute(yAttr,"y");
	    exampleSet.getAttributes().setSpecialAttribute(xAttr,"x");
	    return exampleSet;
    }
	 public boolean export(ExampleSet exampleSet, String filePath, boolean isSupervised) throws OperatorCreationException
	 {
			Process process;
			try {
				process = new Process();
			    // create a process
				Operator dataCSVWriter = OperatorService.createOperator(CSVExampleSetWriter.class);
				dataCSVWriter.setParameter(CSVExampleSetWriter.PARAMETER_CSV_FILE,filePath);
			    process.getRootOperator().getSubprocess(0).addOperator(dataCSVWriter);
			    process.getRootOperator().getSubprocess(0).getInnerSources().getPortByIndex(0).connectTo( dataCSVWriter.getInputPorts().getPortByName("input"));	
			    // run the process with new IOContainer using the created exampleSet	   
			    
			    trainingOutputContainer = process.run(new IOContainer(exampleSet)); 
				new AnnounceFrame("Data exported!",10);
			    
			} catch (OperatorException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				return false;
			} 
			catch(Exception e2)
			{
				e2.printStackTrace();
				return false;
			}
			return true;
		 
	 }
	public boolean train(Process process, ExampleSet exampleSet , MaskStack allStack, boolean isSupervised)
	{

	    if(stopFlag)
	    	return false;
		try {
		    // create a process
		    //process.getRootOperator().getSubprocess(0).addOperator(someOperator);
		    //process.getRootOperator().getSubprocess(0).getInnerSources().getPortByIndex(0).connectTo( someOperator.getInputPorts().getPortByName("input"));	
		    // run the process with new IOContainer using the created exampleSet	   
		    
			trainingOutputContainer = process.run(new IOContainer(exampleSet)); 
			//new AnnounceFrame("Training done!",10);
		    
		} catch (OperatorException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}
		catch(Exception e2)
		{
			e2.printStackTrace();
			return false;
		}
		if(!isSupervised)
		{
			try {
				ExampleSet exampleSetOutput = trainingOutputContainer.get(ExampleSet.class);
				Attribute clusterAttr = exampleSetOutput.getAttributes().getCluster();
			    Attribute xAttr = exampleSetOutput.getAttributes().getSpecial("x");
			    Attribute yAttr = exampleSetOutput.getAttributes().getSpecial("y");
				if(clusterAttr!= null && xAttr !=null && yAttr !=null)
				{
					NominalMapping clusterMapping =clusterAttr.getMapping();
					allStack.beginUpdate();
				 	for(int i=0;i<allStack.size();i++)
				 	{
				 		Mask m = allStack.getByIndex(i);
				 		if(m.getLabel().contains("predict"))
				 			allStack.remove(m);
				 		
				 	}
				 	for(String name : clusterMapping.getValues())
				 	{
				 		if(allStack.getByLabel("predict_"+name) !=null)
				 			allStack.remove(allStack.getByLabel("predict_"+name));
				 		allStack.createNewMask("predict_"+name, false, getRandomColor(), MaskEditor.getRunningInstance(false).getGlobalOpacity());
				 	}
					 for(int i=0;i<exampleSetOutput.size();i++)
					 {
						Example e =exampleSetOutput.getExample(i);
						String predictlabel = clusterMapping.mapIndex((int) e.getValue(clusterAttr));
						Mask m = allStack.getByLabel("predict_"+predictlabel);
						m.getBinaryData().set((int)e.getValue(xAttr), (int)e.getValue(yAttr),true );
					 }

					 for(Mask m: allStack)
						 m.getBinaryData().dataChanged();
						
					 allStack.endUpdate();

				}
				else
					return false;
			} catch (MissingIOObjectException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				return false;
			} catch (MaskException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
				return false;
			}
			catch(Exception e2)
			{
				e2.printStackTrace();
				return false;
			}
			
		}
		return true;
	}

	public boolean predict(Process process,ExampleSet exampleSet , MaskStack stack, boolean isSupervised) throws MaskException, MissingIOObjectException
	{

		try {
			
		    // create a process
		    //process.getRootOperator().getSubprocess(0).addOperator(someOperator);
		    //process.getRootOperator().getSubprocess(0).getInnerSources().getPortByIndex(0).connectTo( someOperator.getInputPorts().getPortByName("input"));	
		    // run the process with new IOContainer using the created exampleSet	   
			IOContainer	predictDataContainer;
			if(trainingOutputContainer!=null)
		    {
		    	Model model = trainingOutputContainer.get(Model.class);
				predictDataContainer = process.run(new IOContainer(model,exampleSet));
		    }
			else
			{
				predictDataContainer = process.run(new IOContainer(exampleSet));
			}
		 	ExampleSet predictDataSet = predictDataContainer.get(ExampleSet.class);
		    Attribute xAttr = predictDataSet.getAttributes().getSpecial("x");
		    Attribute yAttr = predictDataSet.getAttributes().getSpecial("y");
		    Attribute predictAttr = isSupervised? predictDataSet.getAttributes().getPredictedLabel():predictDataSet.getAttributes().getCluster();;  
		    if((predictAttr!= null)&& xAttr !=null && yAttr !=null)
		    {
		    	NominalMapping predictMapping =predictAttr.getMapping();
				stack.beginUpdate();
			 	for(int i=0;i<stack.size();i++)
			 	{
			 		Mask m = stack.getByIndex(i);
			 		if(m.getLabel().contains("predict"))
			 			stack.remove(m);
			 		
			 	}
			 	for(String name : predictMapping.getValues())
			 	{
			 		if(stack.getByLabel("predict_"+name) !=null)
			 			stack.remove(stack.getByLabel("predict_"+name));
			 		stack.createNewMask("predict_"+name, false, getRandomColor(), MaskEditor.getRunningInstance(false).getGlobalOpacity());
			 	}
			 	
				 for(int i=0;i<predictDataSet.size();i++)
				 {
					 Example e =predictDataSet.getExample(i);
					 String predictlabel = predictMapping.mapIndex((int) e.getValue(predictAttr));
					 
					Mask m = stack.getByLabel("predict_"+predictlabel);
					m.getBinaryData().set((int)e.getValue(xAttr), (int)e.getValue(yAttr),true );
				 }
	
				 for(Mask m: stack)
					 m.getBinaryData().dataChanged();
					
				stack.endUpdate();
		    }
		    else
		    	return false;
			
		} catch (OperatorException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}
		catch(Exception e2)
		{
			e2.printStackTrace();
			return false;
		}
		return true;
	}
	@Override
	public void stopExecution() {
		stopFlag = true;
	}

	@Override
	public void clean() {
		
	}
	@Override
	protected void execute() {
		stopFlag = false;
		boolean isSupervised = varLearningType.getValue() == LearningTypeEnumeration.Supervised;
		if (varStage.getValue() == StageEnumeration.Training)
		{
			
			try
			{
				if(varTrainXmlFile.getValue() == null)
				{
					new AnnounceFrame("Please select valid process file first!",10);
				}
				if(varTrainXmlFile.getValue().exists() && varTrainXmlFile.getValue().getName().toLowerCase().endsWith(".xml"))
				{
					Sequence sequence = varTrainSequence.getValue();
					MaskEditor me = MaskEditor.getRunningInstance(true);
					MaskStack stack = me.getBackupObject();
					ExampleSet exampleSet = generateExampleSet(sequence,isSupervised,stack);
					if(stopFlag)
				    	return;
					Process process = new Process(varTrainXmlFile.getValue());
					if(train(process,exampleSet,stack,isSupervised))
					{
						varStage.setValue(StageEnumeration.Predicting);
						new AnnounceFrame("Training done!",10);
					}
					else
						new AnnounceFrame("Training failed!",10);
				}
				else
				{
					new AnnounceFrame("Please select valid process file first!",10);
				}
			}
			catch(Exception e2)
			{
				e2.printStackTrace();
			}
			
		}
		else if (varStage.getValue() == StageEnumeration.Predicting)
		{
			try 
			{
				if(varPredictXmlFile.getValue() == null)
				{
					new AnnounceFrame("Please select valid process file first!",10);
				}
				if(varPredictXmlFile.getValue().exists() && varPredictXmlFile.getValue().getName().toLowerCase().endsWith(".xml"))
				{
					Sequence sequence = varPredictSequence.getValue();
					MaskEditor me = MaskEditor.getRunningInstance(true);
					MaskStack stack = me.getBackupObject();
					ExampleSet exampleSet = generateExampleSet(sequence,false, null);
				    if(stopFlag)
				    	return;
				    Process process = new Process(varPredictXmlFile.getValue());
					if(predict(process,exampleSet,stack,isSupervised))
						new AnnounceFrame("Predicting done!",10);
					else
						new AnnounceFrame("Predicting failed!",10);
				}
				else
				{
					new AnnounceFrame("Please select valid process file first!",10);
				}
			}
			catch(Exception e2)
			{
				e2.printStackTrace();
			}
		}	
	}

	@Override
	protected void initialize() {
		varTrainXmlFile = new EzVarFile("Training process xml", null);
		varPredictXmlFile = new EzVarFile("Predicting process xml", null);
		varTrainSequence = new EzVarSequence("Training sequence");
		varPredictSequence = new EzVarSequence("Predicting sequence");

		varExportBtn = new EzButton("Export data to CSV file",this);
		varLearningType = new EzVarEnum<LearningTypeEnumeration>("Learning Type", LearningTypeEnumeration.values(), LearningTypeEnumeration.Supervised);
		
		varStage = new EzVarEnum<StageEnumeration>("Working Stage", StageEnumeration.values(), StageEnumeration.Training);
		
		
		EzGroup trainGroup = new EzGroup("Train",varExportBtn,varTrainXmlFile,varTrainSequence);
		EzGroup predictGroup = new EzGroup("Predict",varPredictXmlFile,varPredictSequence);
		
		varStage.addVisibilityTriggerTo(trainGroup,  StageEnumeration.Training);
		varStage.addVisibilityTriggerTo(predictGroup,  StageEnumeration.Predicting);
		
		
		super.addEzComponent(varLearningType);
		super.addEzComponent(varStage);
		super.addEzComponent(trainGroup);
		super.addEzComponent(predictGroup);

		try {
			MaskEditor me = MaskEditor.getRunningInstance(true);
			MaskStack stack = me.getBackupObject();
			stack.createNewMask("negative", false, getRandomColor(), MaskEditor.getRunningInstance(false).getGlobalOpacity());
			stack.createNewMask("positive", false, getRandomColor(), MaskEditor.getRunningInstance(false).getGlobalOpacity());
			stack.remove(stack.getByIndex(0));
		
		} catch (Exception e) {
			e.printStackTrace();
		}
		try
		{
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
		catch (Exception e) {
			new AnnounceFrame("Error occured when initializing RapidMiner!",10);
		}

	}

	@Override
	public void actionPerformed(ActionEvent e) {
		boolean isSupervised = varLearningType.getValue().equals("Supervised");
			if (((JButton)e.getSource()).getText().equals(varExportBtn.name))
			{
				try {
					final JFileChooser fc = new JFileChooser();
					int returnVal = fc.showOpenDialog(null);

			        if (returnVal == JFileChooser.APPROVE_OPTION) {
			        	Sequence sequence = varTrainSequence.getValue();
				    	MaskEditor me = MaskEditor.getRunningInstance(true);
						MaskStack stack = me.getBackupObject();
						ExampleSet exampleSet = generateExampleSet(sequence,isSupervised,stack);
						
			            File file = fc.getSelectedFile();
			            export(exampleSet,file.getAbsolutePath(),isSupervised);
			        }
				} catch (OperatorCreationException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
				catch(Exception e2)
				{
					e2.printStackTrace();
				}
			}
			
		}
		
}
