package i18n;
import java.awt.Font;

public class JTextArea extends javax.swing.JTextArea {

	/**
	 * 
	 */
	private static final long serialVersionUID = 2992048980982637224L;
	public JTextArea(){
		super();
		this.setFont(new   Font( "sansserif",Font.PLAIN,12));
	}	
	public JTextArea( String t){
		super(t);
	}	
	public void setText(String arg0){
		super.setText(translation.tr(arg0));
		this.setFont(new   Font( "sansserif",Font.PLAIN,12));
	}
}
