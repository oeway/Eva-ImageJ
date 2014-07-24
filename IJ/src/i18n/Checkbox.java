package i18n;

import java.awt.Font;

public class Checkbox extends java.awt.Checkbox{

	/**
	 * 
	 */
	private static final long serialVersionUID = 8100464896738267612L;

	public  Checkbox(String lab){
		super(lab);
		this.setLabel(translation.tr(lab));
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}
	public  Checkbox(String lab,boolean b){
		super(lab,b);
		this.setLabel(translation.tr(lab));
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}
}
