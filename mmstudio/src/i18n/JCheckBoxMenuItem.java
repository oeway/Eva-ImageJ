package i18n;
import java.awt.Font;

import i18n.translation;
public class JCheckBoxMenuItem extends javax.swing.JCheckBoxMenuItem {
	/**
	 * 
	 */
	private static final long serialVersionUID = -679624817637540041L;
	/**
	 * 
	 */
	public JCheckBoxMenuItem( ){
		super();
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}		
	public JCheckBoxMenuItem( String t){
		super(t);
	}	
	public void setText(String arg0){
		super.setText(translation.tr(arg0));
		this.setActionCommand(arg0);
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}
	
}
