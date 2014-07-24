package i18n;

import java.awt.Font;
import java.awt.MenuShortcut;

public class MenuItem extends java.awt.MenuItem{

	/**
	 * 
	 */
	private static final long serialVersionUID = -6539129848504637773L;
	public  MenuItem(String lab){
		super(lab);
		setLabel(lab);
	}
	public MenuItem(String lab, MenuShortcut ms ){
		super(lab,ms);
		setLabel(lab);		
	}
	public void setLabel(String lab){
		super.setLabel(translation.tr(lab));
		this.setActionCommand(lab);
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}
}
