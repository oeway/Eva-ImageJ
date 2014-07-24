package i18n;

import java.awt.Font;

public class Menu extends java.awt.Menu{
	/**
	 * 
	 */
	private static final long serialVersionUID = -6473253827789806640L;
	public  Menu(String lab){
		super(lab);
		setLabel(lab);
	}
	public void setLabel(String lab){
		super.setLabel(translation.tr(lab));
		this.setActionCommand(lab);
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}
}
