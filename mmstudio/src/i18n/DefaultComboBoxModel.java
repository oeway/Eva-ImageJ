package i18n;
public class DefaultComboBoxModel extends javax.swing.DefaultComboBoxModel {

	/**
	 * 
	 */
	private static final long serialVersionUID = -2331018904253414826L;
	public DefaultComboBoxModel(){
		super();
	}	
	public DefaultComboBoxModel( String t[]){
		super();
		for (String s :t){
			super.addElement(translation.tr(s));
		}
	}	
}