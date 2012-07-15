package i18n;
import java.awt.Font;

public class JToggleButton extends javax.swing.JToggleButton {
	/**
	 * 
	 */
	private static final long serialVersionUID = 3811545088428213701L;
	/**
	 * 
	 */

	public JToggleButton(){
		super();
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}	
	public JToggleButton( String t){
		super(t);
	}	
	public void setText(String arg0){
		super.setText(translation.tr(arg0));
		this.setActionCommand(arg0);
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
	}
}
