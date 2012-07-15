package org.micromanager.oeway;

import java.awt.Color;
import java.awt.Dialog;
import java.awt.Font;
import java.awt.Frame;
import java.awt.GraphicsConfiguration;
import java.awt.Window;

import javax.swing.DefaultComboBoxModel;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JProgressBar;
import javax.swing.JSlider;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;

import org.dyno.visual.swing.layouts.Constraints;
import org.dyno.visual.swing.layouts.GroupLayout;
import org.dyno.visual.swing.layouts.Leading;

//VS4E -- DO NOT REMOVE THIS LINE!
public class AquisitionDialog extends JDialog {

	private static final long serialVersionUID = 1L;
	public JComboBox cobMode;
	public JProgressBar ProgBar;
	public JLabel lblCount;
	public JButton btnStart;
	public JSlider sldFrameNum;
	public JButton jButton1;
	public JButton jButton0;
	public JLabel jLabel0;
	public JLabel jLabel1;
	public JTextField jTextField0;
	private static final String PREFERRED_LOOK_AND_FEEL = "javax.swing.plaf.metal.MetalLookAndFeel";
	public AquisitionDialog() {
		initComponents();
	}

	public AquisitionDialog(Frame parent) {
		super(parent);
		initComponents();
	}

	public AquisitionDialog(Frame parent, boolean modal) {
		super(parent, modal);
		initComponents();
	}

	public AquisitionDialog(Frame parent, String title) {
		super(parent, title);
		initComponents();
	}

	public AquisitionDialog(Frame parent, String title, boolean modal) {
		super(parent, title, modal);
		initComponents();
	}

	public AquisitionDialog(Frame parent, String title, boolean modal,
			GraphicsConfiguration arg) {
		super(parent, title, modal, arg);
		initComponents();
	}

	public AquisitionDialog(Dialog parent) {
		super(parent);
		initComponents();
	}

	public AquisitionDialog(Dialog parent, boolean modal) {
		super(parent, modal);
		initComponents();
	}

	public AquisitionDialog(Dialog parent, String title) {
		super(parent, title);
		initComponents();
	}

	public AquisitionDialog(Dialog parent, String title, boolean modal) {
		super(parent, title, modal);
		initComponents();
	}

	public AquisitionDialog(Dialog parent, String title, boolean modal,
			GraphicsConfiguration arg) {
		super(parent, title, modal, arg);
		initComponents();
	}

	public AquisitionDialog(Window parent) {
		super(parent);
		initComponents();
	}

	public AquisitionDialog(Window parent, ModalityType modalityType) {
		super(parent, modalityType);
		initComponents();
	}

	public AquisitionDialog(Window parent, String title) {
		super(parent, title);
		initComponents();
	}

	public AquisitionDialog(Window parent, String title, ModalityType modalityType) {
		super(parent, title, modalityType);
		initComponents();
	}

	public AquisitionDialog(Window parent, String title,
			ModalityType modalityType, GraphicsConfiguration arg) {
		super(parent, title, modalityType, arg);
		initComponents();
	}

	private void initComponents() {
		setTitle("Aquisition");
		setFont(new Font("Dialog", Font.PLAIN, 12));
		setBackground(Color.white);
		setLocationByPlatform(true);
		setResizable(false);
		setForeground(Color.black);
		setAlwaysOnTop(true);
		setLayout(new GroupLayout());
		add(getCobMode(), new Constraints(new Leading(26, 170, 10, 10), new Leading(86, 10, 10)));
		add(getBtnStart(), new Constraints(new Leading(205, 94, 12, 12), new Leading(85, 12, 12)));
		add(getJButton0(), new Constraints(new Leading(26, 125, 10, 10), new Leading(18, 12, 12)));
		add(getJButton1(), new Constraints(new Leading(172, 127, 12, 12), new Leading(18, 12, 12)));
		add(getJLabel0(), new Constraints(new Leading(26, 12, 12), new Leading(59, 10, 10)));
		add(getlblCount(), new Constraints(new Leading(20, 10, 10), new Leading(166, 10, 10)));
		add(getProgBar(), new Constraints(new Leading(20, 281, 12, 12), new Leading(213, 19, 10, 10)));
		add(getSldFrameNum(), new Constraints(new Leading(12, 296, 12, 12), new Leading(189, 12, 12)));
		add(getJTextField0(), new Constraints(new Leading(26, 80, 12, 12), new Leading(139, 12, 12)));
		add(getJLabel1(), new Constraints(new Leading(26, 12, 12), new Leading(118, 12, 12)));
		setSize(320, 277);
	}

	private JTextField getJTextField0() {
		if (jTextField0 == null) {
			jTextField0 = new JTextField();
			jTextField0.setText("1000");
		}
		return jTextField0;
	}

	private JLabel getJLabel1() {
		if (jLabel1 == null) {
			jLabel1 = new JLabel();
			jLabel1.setText("曝光时间(ms)");
		}
		return jLabel1;
	}

	private JLabel getJLabel0() {
		if (jLabel0 == null) {
			jLabel0 = new JLabel();
			jLabel0.setText("多帧连续采集：");
		}
		return jLabel0;
	}

	private JButton getJButton0() {
		if (jButton0 == null) {
			jButton0 = new JButton();
			jButton0.setText("实时预览");
		}
		return jButton0;
	}

	private JButton getJButton1() {
		if (jButton1 == null) {
			jButton1 = new JButton();
			jButton1.setText("采集一帧");
		}
		return jButton1;
	}

	public JSlider getSldFrameNum() {
		if (sldFrameNum == null) {
			sldFrameNum = new JSlider();
			sldFrameNum.setValue(52);
		}
		return sldFrameNum;
	}

	public JButton getBtnStart() {
		if (btnStart == null) {
			btnStart = new JButton();
			btnStart.setText("开始");
		}
		return btnStart;
	}

	private JProgressBar getProgBar() {
		if (ProgBar == null) {
			ProgBar = new JProgressBar();
		}
		return ProgBar;
	}

	private JLabel getlblCount() {
		if (lblCount == null) {
			lblCount = new JLabel();
			lblCount.setText("Frame Num.");
		}
		return lblCount;
	}

	private JComboBox getCobMode() {
		if (cobMode == null) {
			cobMode = new JComboBox();
			cobMode.setModel(new DefaultComboBoxModel(new Object[] {}));
			cobMode.setDoubleBuffered(false);
			cobMode.setBorder(null);
		}
		return cobMode;
	}

	private static void installLnF() {
		try {
			String lnfClassname = PREFERRED_LOOK_AND_FEEL;
			if (lnfClassname == null)
				lnfClassname = UIManager.getCrossPlatformLookAndFeelClassName();
			UIManager.setLookAndFeel(lnfClassname);
		} catch (Exception e) {
			System.err.println("Cannot install " + PREFERRED_LOOK_AND_FEEL
					+ " on this platform:" + e.getMessage());
		}
	}

	/**
	 * Main entry of the class.
	 * Note: This class is only created so that you can easily preview the result at runtime.
	 * It is not expected to be managed by the designer.
	 * You can modify it as you like.
	 */
	public static void main(String[] args) {
		installLnF();
		SwingUtilities.invokeLater(new Runnable() {
			@Override
			public void run() {
				AquisitionDialog dialog = new AquisitionDialog();
				dialog.setDefaultCloseOperation(AquisitionDialog.DISPOSE_ON_CLOSE);
				dialog.setTitle("AquisitionDialog");
				dialog.setLocationRelativeTo(null);
				dialog.getContentPane().setPreferredSize(dialog.getSize());
				dialog.pack();
				dialog.setVisible(true);
			}
		});
	}

}
