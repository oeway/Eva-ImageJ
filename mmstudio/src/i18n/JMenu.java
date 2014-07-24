package i18n;

import java.awt.Font;

public class JMenu extends javax.swing.JMenu {
	/**
	 * 
	 */
	private static final long serialVersionUID = 3767593450686880300L;
	public JMenu(){
		super();
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}	
	public JMenu( String t){
		super(t);
	}	
	public void setText(String arg0){
		super.setText(translation.tr(arg0));
		this.setActionCommand(arg0);
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}
}