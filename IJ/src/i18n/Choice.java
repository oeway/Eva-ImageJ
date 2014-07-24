package i18n;

import java.awt.Font;
import java.util.Hashtable;

public class Choice extends java.awt.Choice{

	/**
	 * 
	 */
	Hashtable<String, String> dict =new Hashtable<String, String>();
	private static final long serialVersionUID = 8100464896738267612L;
	public  void addItem(String lab){
		String t = translation.tr(lab);
		super.addItem(t);
		this.setFont(new   Font( "sansserif",Font.PLAIN,14));
		dict.put(t, lab);
	}
	public String getSelectedItem(){
		return dict.get(super.getSelectedItem());
	}

}
