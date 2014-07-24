package i18n;
public class DefaultMutableTreeNode extends javax.swing.tree.DefaultMutableTreeNode{

	/**
	 * 
	 */
	public String Tag=new String();
	private static final long serialVersionUID = 5541324440151493619L;
	public  DefaultMutableTreeNode(String lab){
		super(lab);
		Tag=lab;
	}

}
