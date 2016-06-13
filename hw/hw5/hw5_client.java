
import javax.swing.JFrame;
import javax.swing.JButton;
import java.awt.BorderLayout;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

import java.net.*;
import java.io.*;
import javax.swing.JTextPane;
import java.awt.Color;
import javax.swing.JLabel;
import javax.swing.border.LineBorder;
import java.awt.Font;
import java.awt.Window;

import javax.swing.JPanel;
import java.awt.CardLayout;
import javax.swing.ImageIcon;

public class hw5_client {

	private static JFrame window;
	public static String server_addr;
	public static String server_port;
	public static int status = 0;
	public static String[] icons = new String[2];
	public static int steps = 0;

	public static Socket serverSocket;
	static DataOutputStream outToServer;
	static DataInputStream inFromServer;
	private JButton button_start;
	private JTextPane text_userName;
	private JLabel label_playerName;
	private JButton button_exit;
	private static JLabel label_status;
	private JPanel panel_welcome_inner;
	private JPanel panel_larbyPage;
	private JButton button_buildRoom;
	private JButton button_backToWPage;
	private JPanel panel;
	private JButton btnEnterRoom;
	private static JButton button_pos1;
	private static JButton button_pos2;
	private static JButton button_pos3;
	private static JButton button_pos4;
	private static JButton button_pos5;
	private static JButton button_pos6;
	private static JButton button_pos7;
	private static JButton button_pos8;
	private static JButton button_pos9;
	private JPanel panel_1;
	

	/**
	 * Launch the application.
	 */
	@SuppressWarnings("deprecation")
	public static void main(String[] args) throws Exception {
		server_addr = args[0];
		server_port = args[1];

		hw5_client window = new hw5_client();
		window.window.setVisible(true);

		serverSocket = new Socket(server_addr, Integer.parseInt(server_port));
		outToServer = new DataOutputStream(serverSocket.getOutputStream());
		inFromServer = new DataInputStream(serverSocket.getInputStream());
		icons[0] = "circle.jpg";
		icons[1] = "cross.jpg";
		
		
		String temp = "";
		while (!temp.equals("bye")) {
			temp = inFromServer.readLine();
			switch (temp) {
			case "_MATCH_":
				temp = inFromServer.readLine();
				label_status.setText(temp);
				temp = inFromServer.readLine();
				if (temp.equals("_FIRST_")) {
					label_status.setText(temp);
					icons[0] = "circle.jpg";
					icons[1] = "cross.jpg";
				} else if (temp.equals("_SECOND_")) {
					label_status.setText(temp);
					icons[1] = "circle.jpg";
					icons[0] = "cross.jpg";
				}
				while (true) {
					temp = inFromServer.readLine();
					switch (temp) {
					case "1":
						button_pos1.setIcon(new ImageIcon(hw5_client.class.getResource(icons[1])));
						button_pos1.setEnabled(false);
						steps++;
						break;
					case "2":
						button_pos2.setIcon(new ImageIcon(hw5_client.class.getResource(icons[1])));
						button_pos2.setEnabled(false);
						steps++;
						break;
					case "3":
						button_pos3.setIcon(new ImageIcon(hw5_client.class.getResource(icons[1])));
						button_pos3.setEnabled(false);
						steps++;
						break;
					case "4":
						button_pos4.setIcon(new ImageIcon(hw5_client.class.getResource(icons[1])));
						button_pos4.setEnabled(false);
						steps++;
						break;
					case "5":
						button_pos5.setIcon(new ImageIcon(hw5_client.class.getResource(icons[1])));
						button_pos5.setEnabled(false);
						steps++;
						break;
					case "6":
						button_pos6.setIcon(new ImageIcon(hw5_client.class.getResource(icons[1])));
						button_pos6.setEnabled(false);
						steps++;
						break;
					case "7":
						button_pos7.setIcon(new ImageIcon(hw5_client.class.getResource(icons[1])));
						button_pos7.setEnabled(false);
						steps++;
						break;
					case "8":
						button_pos8.setIcon(new ImageIcon(hw5_client.class.getResource(icons[1])));
						button_pos8.setEnabled(false);
						steps++;
						break;
					case "9":
						button_pos9.setIcon(new ImageIcon(hw5_client.class.getResource(icons[1])));
						button_pos9.setEnabled(false);
						steps++;
						break;
					}
					if(steps==9){
						outToServer.writeBytes("_OVER_");
						break;
					}
				}
				break;
			default:
				label_status.setText("???" + temp);
			}
		}
		serverSocket.close();
		label_status.setText("byebye~~~~~~~");
		Thread.sleep(1500);
		window.window.dispose();
	}

	/**
	 * Create the application.
	 */
	public hw5_client() {
		initialize();
	}

	/**
	 * Initialize the contents of the frame.
	 */
	private void initialize() {
		window = new JFrame();
		window.setTitle("OOXX --- OO feels like good ");
		window.setBounds(100, 100, 588, 586);
		window.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		window.getContentPane().setLayout(new BorderLayout(0, 0));

		panel = new JPanel();
		window.getContentPane().add(panel, BorderLayout.CENTER);
		panel.setLayout(new CardLayout(0, 0));

		JPanel panel_welcomePage = new JPanel();
		panel.add(panel_welcomePage, "name_284728164739502");
		panel_welcomePage.setLayout(new BorderLayout(0, 0));

		panel_welcome_inner = new JPanel();
		panel_welcomePage.add(panel_welcome_inner, BorderLayout.CENTER);
		panel_welcome_inner.setLayout(null);

		button_start = new JButton("Start !!");
		button_start.setBounds(230, 408, 109, 25);
		panel_welcome_inner.add(button_start);
		button_start.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (text_userName.getText().length() == 0)
					label_status.setText("Please enter Player's Name!!");
				else {
					try {
						outToServer.writeBytes(text_userName.getText() + '\n');
						label_status.setText("Send : " + text_userName.getText());
						panel_welcomePage.setVisible(false);
						panel_larbyPage.setVisible(true);
					} catch (IOException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
				}
			}
		});
		button_start.setBackground(Color.YELLOW);
		button_start.setBorder(new LineBorder(Color.BLACK, 5, true));

		button_exit = new JButton("exit");
		button_exit.setBounds(230, 445, 109, 25);
		panel_welcome_inner.add(button_exit);
		button_exit.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				try {
					outToServer.writeBytes("bye\n");
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
			}
		});

		text_userName = new JTextPane();
		text_userName.setBounds(230, 371, 109, 25);
		panel_welcome_inner.add(text_userName);
		text_userName.setBorder(new LineBorder(new Color(0, 0, 0), 2, true));
		text_userName.setBackground(Color.PINK);
		text_userName.setToolTipText("user name");

		label_playerName = new JLabel("Player Name :");
		label_playerName.setBounds(111, 371, 109, 17);
		panel_welcome_inner.add(label_playerName);
		label_playerName.setFont(new Font("Dialog", Font.BOLD, 14));
		label_playerName.setOpaque(true);

		panel_larbyPage = new JPanel();
		panel.add(panel_larbyPage, "name_284728171647183");
		panel_larbyPage.setVisible(false);

		button_buildRoom = new JButton("Build room");
		button_buildRoom.setBounds(213, 430, 152, 25);

		btnEnterRoom = new JButton("Enter room");
		btnEnterRoom.setBounds(213, 467, 152, 25);

		button_backToWPage = new JButton("Back");
		button_backToWPage.setBounds(213, 504, 152, 25);
		panel_larbyPage.setLayout(null);
		panel_larbyPage.add(button_buildRoom);
		panel_larbyPage.add(btnEnterRoom);
		panel_larbyPage.add(button_backToWPage);

		panel_1 = new JPanel();
		panel_1.setBounds(85, 42, 406, 376);
		panel_larbyPage.add(panel_1);

		button_pos1 = new JButton("");
		button_pos1.setFocusTraversalKeysEnabled(false);
		button_pos1.setFocusPainted(false);
		button_pos1.setFocusable(false);
		button_pos1.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				button_pos1.setIcon(new ImageIcon(getClass().getResource(icons[0])));
				button_pos1.setEnabled(false);
//				steps++;
//				try {
//					outToServer.writeBytes("1\n");
//				} catch (IOException e1) {
//					// TODO Auto-generated catch block
//					e1.printStackTrace();
//				}
			}
		});
		button_pos1.setBackground(new Color(255, 255, 255));
		button_pos1.setBounds(8, 7, 117, 104);
		panel_1.setLayout(null);
		panel_1.add(button_pos1);

		button_pos2 = new JButton("");
		button_pos2.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				button_pos2.setIcon(new ImageIcon(getClass().getResource(icons[0])));
				button_pos2.setEnabled(false);
				steps++;
				try {
					outToServer.writeBytes("1\n");
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
			}
		});
		button_pos2.setBackground(new Color(255, 255, 255));
		button_pos2.setBounds(133, 7, 117, 104);
		panel_1.add(button_pos2);

		button_pos3 = new JButton("");
		button_pos3.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				button_pos3.setIcon(new ImageIcon(getClass().getResource(icons[0])));
				button_pos3.setEnabled(false);
				steps++;
				try {
					outToServer.writeBytes("1\n");
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
			}
		});
		button_pos3.setBackground(new Color(255, 255, 255));
		button_pos3.setBounds(258, 7, 117, 104);
		panel_1.add(button_pos3);

		button_pos4 = new JButton("");
		button_pos4.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				button_pos4.setIcon(new ImageIcon(getClass().getResource(icons[0])));
				button_pos4.setEnabled(false);
				steps++;
				try {
					outToServer.writeBytes("1\n");
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
			}
		});
		button_pos4.setBackground(new Color(255, 255, 255));
		button_pos4.setBounds(8, 137, 117, 104);
		panel_1.add(button_pos4);

		button_pos5 = new JButton("");
		button_pos5.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				button_pos5.setIcon(new ImageIcon(getClass().getResource(icons[0])));
				button_pos5.setEnabled(false);
				steps++;
				try {
					outToServer.writeBytes("1\n");
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
			}
		});
		button_pos5.setBackground(new Color(255, 255, 255));
		button_pos5.setBounds(133, 137, 117, 104);
		panel_1.add(button_pos5);

		button_pos6 = new JButton("");
		button_pos6.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				button_pos6.setIcon(new ImageIcon(getClass().getResource(icons[0])));
				button_pos6.setEnabled(false);
				steps++;
				try {
					outToServer.writeBytes("1\n");
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
			}
		});
		button_pos6.setBackground(new Color(255, 255, 255));
		button_pos6.setBounds(258, 137, 117, 104);
		panel_1.add(button_pos6);

		button_pos7 = new JButton("");
		button_pos7.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				button_pos7.setIcon(new ImageIcon(getClass().getResource(icons[0])));
				button_pos7.setEnabled(false);
				steps++;
				try {
					outToServer.writeBytes("1\n");
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
			}
		});
		button_pos7.setBackground(new Color(255, 255, 255));
		button_pos7.setBounds(8, 260, 117, 104);
		panel_1.add(button_pos7);

		button_pos8 = new JButton("");
		button_pos8.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				button_pos8.setIcon(new ImageIcon(getClass().getResource(icons[0])));
				button_pos8.setEnabled(false);
				steps++;
				try {
					outToServer.writeBytes("1\n");
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
			}
		});
		button_pos8.setBackground(new Color(255, 255, 255));
		button_pos8.setBounds(133, 260, 117, 104);
		panel_1.add(button_pos8);

		button_pos9 = new JButton("");
		button_pos9.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				button_pos9.setIcon(new ImageIcon(getClass().getResource(icons[0])));
				button_pos9.setEnabled(false);
				steps++;
				try {
					outToServer.writeBytes("1\n");
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
			}
		});
		button_pos9.setBackground(new Color(255, 255, 255));
		button_pos9.setBounds(258, 260, 117, 104);
		panel_1.add(button_pos9);
		button_backToWPage.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				panel_welcomePage.setVisible(true);
				panel_larbyPage.setVisible(false);
			}
		});
		btnEnterRoom.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				try {
					outToServer.writeBytes("_ENTER_\n");
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
			}
		});
		button_buildRoom.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				try {
					outToServer.writeBytes("_BUILD_\n");
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
			}
		});

		label_status = new JLabel("welcome!!");
		window.getContentPane().add(label_status, BorderLayout.SOUTH);
		label_status.setBorder(new LineBorder(new Color(0, 0, 0)));
		label_status.setBackground(Color.BLACK);

	}
}
