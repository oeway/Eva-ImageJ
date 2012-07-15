package i18n;
import java.awt.Font;

import i18n.translation;
public class JMenuItem extends javax.swing.JMenuItem {
	/**
	 * 
	 */
	private static final long serialVersionUID = -679624817637540041L;
	/**
	 * 
	 */
	public JMenuItem( ){
		super();
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}		
	public JMenuItem( String t){
		super(t);
	}	
	public void setText(String arg0){
		super.setText(translation.tr(arg0));
		this.setActionCommand(arg0);
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}
	
}
