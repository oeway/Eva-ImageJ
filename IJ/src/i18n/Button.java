package i18n;

import java.awt.Font;

public class Button extends java.awt.Button{
	/**
	 * 
	 */
	private static final long serialVersionUID = 7471664313451279763L;
	public  Button(String lab){
		super(lab);
		setLabel(lab);
	}
	public void setLabel(String lab){
		super.setLabel(translation.tr(lab));
		this.setActionCommand(lab);
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}
}
