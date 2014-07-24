package i18n;

import java.awt.Font;

public class Label extends java.awt.Label{

	/**
	 * 
	 */
	private static final long serialVersionUID = 8182281124583502912L;
	public  Label(String lab){
		super(lab);
		setText(lab);
	}
	public  Label(String lab,int i){
		super(lab,i);
		setText(lab);
	}	
	public void setText(String lab){
		super.setText(translation.tr(lab));
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}
	
}
