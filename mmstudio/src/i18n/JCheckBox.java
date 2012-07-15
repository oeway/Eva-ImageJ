package i18n;

import java.awt.Font;

public class JCheckBox extends javax.swing.JCheckBox {
	/**
	 * 
	 */
	private static final long serialVersionUID = 9125132819873534423L;
	public JCheckBox(){
		super();
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}
	public JCheckBox( String t,Boolean b){
		super();
		super.setSelected(b);
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}	
	public JCheckBox( String t){
		super(t);
	}	
	public void setText(String arg0){
		super.setText(translation.tr(arg0));
		this.setActionCommand(arg0);
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}
}