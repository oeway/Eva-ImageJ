package i18n;

import java.awt.Font;

import javax.swing.JPanel;
import javax.swing.JSplitPane;

public class JTabbedPane extends javax.swing.JTabbedPane {
	/**
	 * 
	 */
	private static final long serialVersionUID = 5198198332503046786L;

	/**
	 * 
	 */
	public void addTab(String jn ,JPanel jp){
		super.addTab(translation.tr(jn), jp);
		this.setFont(new   Font( "sansserif",Font.PLAIN,12));
	}
	public void addTab(String jn ,JSplitPane jp){
		super.addTab(translation.tr(jn), jp);
		this.setFont(new   Font( "sansserif",Font.PLAIN,12));
	}
}