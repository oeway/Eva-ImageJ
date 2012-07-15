package i18n;

import java.awt.Font;
public class CheckboxMenuItem extends java.awt.CheckboxMenuItem{
	/**
	 * 
	 */
	private static final long serialVersionUID = 7863528554438812917L;
	public  CheckboxMenuItem(String lab){
		super(lab);
		setLabel(lab);
	}
	public  CheckboxMenuItem(String lab,boolean b){
		super(lab,b);
		setLabel(lab);
	}	
	public void setLabel(String lab){
		super.setLabel(translation.tr(lab));
		this.setActionCommand(lab);
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}
}
