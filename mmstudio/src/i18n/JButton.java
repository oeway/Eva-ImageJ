package i18n;

import java.awt.Font;

public class JButton extends javax.swing.JButton {
	/**
	 * 
	 */
	private static final long serialVersionUID = -8463254144216813729L;
	/**
	 * 
	 */

	public JButton(){
		super();
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}	
	public JButton( String t){
		super(t);
		this.setActionCommand(t);
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}	
	public void setText(String arg0){
		super.setText(translation.tr(arg0));
		this.setActionCommand(arg0);
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}
}
