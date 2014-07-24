package i18n;
import java.awt.Font;

public class JLabel extends javax.swing.JLabel {
	/**
	 * 
	 */
	private static final long serialVersionUID = 5572962135517762914L;
	/**
	 * 
	 */

	public JLabel(){
		super();
		this.setFont(new   Font( "sansserif",Font.PLAIN,12));
	}	
	public JLabel( String t){
		super(t);
	}	
	public void setText(String arg0){
		super.setText(translation.tr(arg0));
		this.setFont(new   Font( "sansserif",Font.PLAIN,12));
	}
}
