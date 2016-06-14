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
import java.awt.Graphics;
import java.awt.Image;

import javax.swing.JPanel;
import java.awt.CardLayout;

import javax.imageio.ImageIO;
import javax.swing.ImageIcon;
import javax.swing.JInternalFrame;
import javax.swing.JProgressBar;
import javax.swing.JLayeredPane;
import java.awt.SystemColor;
import javax.swing.Icon;
import java.awt.Dimension;
import javax.swing.UIManager;

public class hw5_client {
	Image image;

	private static JFrame window;
	public static String server_addr;
	public static String server_port;
	public static int status = 0;
	public static String[] icons = new String[2];
	public static int[] map = new int[9];
	public static int steps = 0;
	public static int score = 0;

	public static Socket serverSocket;
	static DataOutputStream outToServer;
	static DataInputStream inFromServer;
	private static JButton button_start;
	private JTextPane text_userName;
	private JLabel label_playerName;
	private JButton button_exit;
	private static JLabel label_status;
	private JPanel panel_welcome_inner;
	private JPanel panel_larbyPage;
	private static JButton button_buildRoom;
	private static JButton button_backToWPage;
	private static JButton button_enterRoom;
	private JPanel panel;
	private static JButton button_pos1;
	private static JButton button_pos2;
	private static JButton button_pos3;
	private static JButton button_pos4;
	private static JButton button_pos5;
	private static JButton button_pos6;
	private static JButton button_pos7;
	private static JButton button_pos8;
	private static JButton button_pos9;
	private static JPanel panel_gameBoard;
	private static JLabel lblScore;
	private JLabel label_player;
	private static JLayeredPane panel_loading;
	private JLabel label_bgp;
	private JLabel label_bgp_welcome;
	private JLabel label_bgp_larby;

	/**
	 * Launch the application.
	 */
	@SuppressWarnings("deprecation")
	public static void main(String[] args) throws Exception {
		// server_addr = args[0];
		// server_port = args[1];
		server_addr = "127.0.0.1";
		server_port = "8787";

		for (int i = 0; i < 9; i++)
			map[i] = 87;

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
				panel_loading.setVisible(false);
				button_pos1.setVisible(true);
				button_pos2.setVisible(true);
				button_pos3.setVisible(true);
				button_pos4.setVisible(true);
				button_pos5.setVisible(true);
				button_pos6.setVisible(true);
				button_pos7.setVisible(true);
				button_pos8.setVisible(true);
				button_pos9.setVisible(true);
				panel_gameBoard.setVisible(true);
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
				steps = 0;
				// inner game!!!
				for (int i = 0; i < 9; i++)
					map[i] = 0;
				button_buildRoom.setEnabled(false);
				button_enterRoom.setEnabled(false);
				button_backToWPage.setEnabled(false);
				while (true) {
					temp = inFromServer.readLine();
					switch (temp) {
					case "1":
						button_pos1.setIcon(new ImageIcon(icons[1]));
						map[0] = -1;
						steps++;
						break;
					case "2":
						button_pos2.setIcon(new ImageIcon(icons[1]));
						map[1] = -1;
						steps++;
						break;
					case "3":
						button_pos3.setIcon(new ImageIcon(icons[1]));
						map[2] = -1;
						steps++;
						break;
					case "4":
						button_pos4.setIcon(new ImageIcon(icons[1]));
						map[3] = -1;
						steps++;
						break;
					case "5":
						button_pos5.setIcon(new ImageIcon(icons[1]));
						map[4] = -1;
						steps++;
						break;
					case "6":
						button_pos6.setIcon(new ImageIcon(icons[1]));
						map[5] = -1;
						steps++;
						break;
					case "7":
						button_pos7.setIcon(new ImageIcon(icons[1]));
						map[6] = -1;
						steps++;
						break;
					case "8":
						button_pos8.setIcon(new ImageIcon(icons[1]));
						map[7] = -1;
						steps++;
						break;
					case "9":
						button_pos9.setIcon(new ImageIcon(icons[1]));
						map[8] = -1;
						steps++;
						break;
					case "_END_":
						steps = -1;
					}
					if (lose()) {
						outToServer.writeBytes("_OVER_\n");
						break;
					}
					if (steps == 9) {
						outToServer.writeBytes("_OVER_\n");
						initIcon();
						label_status.setText("PEACE~~~");
						break;
					} else if (steps == -1)
						break;
				}
				// leave game
				button_pos1.setVisible(false);
				button_pos2.setVisible(false);
				button_pos3.setVisible(false);
				button_pos4.setVisible(false);
				button_pos5.setVisible(false);
				button_pos6.setVisible(false);
				button_pos7.setVisible(false);
				button_pos8.setVisible(false);
				button_pos9.setVisible(false);
				panel_gameBoard.setVisible(false);
				label_status.setText("Leave ROOM!!");
				break;
			default:
				label_status.setText("???" + temp);
			}
			// next input
		}
		serverSocket.close();
		label_status.setText("byebye~~~~~~~");
		Thread.sleep(1500);
		window.window.dispose();
	}

	public static void initIcon() {
		lblScore.setText("Score : " + score);
		button_pos1.setIcon(null);
		button_pos2.setIcon(null);
		button_pos3.setIcon(null);
		button_pos4.setIcon(null);
		button_pos5.setIcon(null);
		button_pos6.setIcon(null);
		button_pos7.setIcon(null);
		button_pos8.setIcon(null);
		button_pos9.setIcon(null);
		button_buildRoom.setEnabled(true);
		button_enterRoom.setEnabled(true);
		button_backToWPage.setEnabled(true);
		for (int i = 0; i < 9; i++)
			map[i] = 87;
	}

	public Boolean win() {
		if (map[0] == 1 && map[1] == 1 && map[2] == 1) {
			label_status.setText("YOU WIN!!!!!!!!!!!");
			score += 100;
			initIcon();
			return true;
		}
		if (map[3] == 1 && map[4] == 1 && map[5] == 1) {
			label_status.setText("YOU WIN!!!!!!!!!!!");
			score += 100;
			initIcon();
			return true;
		}
		if (map[6] == 1 && map[7] == 1 && map[8] == 1) {
			label_status.setText("YOU WIN!!!!!!!!!!!");
			score += 100;
			initIcon();
			return true;
		}
		if (map[0] == 1 && map[4] == 1 && map[8] == 1) {
			label_status.setText("YOU WIN!!!!!!!!!!!");
			score += 100;
			initIcon();
			return true;
		}
		if (map[2] == 1 && map[4] == 1 && map[6] == 1) {
			label_status.setText("YOU WIN!!!!!!!!!!!");
			score += 100;
			initIcon();
			return true;
		}
		if (map[0] == 1 && map[3] == 1 && map[6] == 1) {
			label_status.setText("YOU WIN!!!!!!!!!!!");
			score += 100;
			initIcon();
			return true;
		}
		if (map[1] == 1 && map[4] == 1 && map[7] == 1) {
			label_status.setText("YOU WIN!!!!!!!!!!!");
			score += 100;
			initIcon();
			return true;
		}
		if (map[2] == 1 && map[5] == 1 && map[8] == 1) {
			label_status.setText("YOU WIN!!!!!!!!!!!");
			score += 100;
			initIcon();
			return true;
		}
		return false;
	}

	public static Boolean lose() {
		if (map[0] == -1 && map[1] == -1 && map[2] == -1) {
			label_status.setText("YOU LOSE!!!!!!!!!!!");
			score -= 50;
			initIcon();
			return true;
		}
		if (map[3] == -1 && map[4] == -1 && map[5] == -1) {
			label_status.setText("YOU LOSE!!!!!!!!!!!");
			score -= 50;
			initIcon();
			return true;
		}
		if (map[6] == -1 && map[7] == -1 && map[8] == -1) {
			label_status.setText("YOU LOSE!!!!!!!!!!!");
			score -= 50;
			initIcon();
			return true;
		}
		if (map[0] == -1 && map[4] == -1 && map[8] == -1) {
			label_status.setText("YOU LOSE!!!!!!!!!!!");
			score -= 50;
			initIcon();
			return true;
		}
		if (map[2] == -1 && map[4] == -1 && map[6] == -1) {
			label_status.setText("YOU LOSE!!!!!!!!!!!");
			score -= 50;
			initIcon();
			return true;
		}
		if (map[0] == -1 && map[3] == -1 && map[6] == -1) {
			label_status.setText("YOU LOSE!!!!!!!!!!!");
			score -= 50;
			initIcon();
			return true;
		}
		if (map[1] == -1 && map[4] == -1 && map[7] == -1) {
			label_status.setText("YOU LOSE!!!!!!!!!!!");
			score -= 50;
			initIcon();
			return true;
		}
		if (map[2] == -1 && map[5] == -1 && map[8] == -1) {
			label_status.setText("YOU LOSE!!!!!!!!!!!");
			score -= 50;
			initIcon();
			return true;
		}
		return false;
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
		window.setMaximumSize(new Dimension(500, 500));
		window.setTitle("OOXX --- OO feels good ");
		window.setBounds(100, 100, 500, 500);
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
		button_start.setBounds(203, 361, 109, 25);
		panel_welcome_inner.add(button_start);
		button_start.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (text_userName.getText().length() == 0)
					label_status.setText("Please enter Player's Name!!");
				else {
					try {
						outToServer.writeBytes(text_userName.getText() + '\n');
						label_status.setText("Send : " + text_userName.getText());
						label_player.setText("Player : " + text_userName.getText());
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
		button_exit.setBounds(203, 398, 109, 25);
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
		text_userName.setBounds(203, 324, 109, 25);
		panel_welcome_inner.add(text_userName);
		text_userName.setBorder(new LineBorder(new Color(0, 0, 0), 2, true));
		text_userName.setBackground(Color.PINK);
		text_userName.setToolTipText("user name");

		label_playerName = new JLabel("Player Name :");
		label_playerName.setBackground(SystemColor.text);
		label_playerName.setBounds(84, 324, 109, 17);
		panel_welcome_inner.add(label_playerName);
		label_playerName.setFont(new Font("Dialog", Font.BOLD, 14));
		label_playerName.setOpaque(true);
		
		label_bgp_welcome = new JLabel(new ImageIcon("din.jpg"));
		label_bgp_welcome.setBounds(-11, 0, 618, 589);
		panel_welcome_inner.add(label_bgp_welcome);

		panel_larbyPage = new JPanel();
		panel.add(panel_larbyPage, "name_284728171647183");
		panel_larbyPage.setVisible(false);

		button_buildRoom = new JButton("Build room");
		button_buildRoom.setBounds(174, 375, 152, 25);

		button_enterRoom = new JButton("Enter room");
		button_enterRoom.setBounds(174, 401, 152, 25);
		panel_larbyPage.setLayout(null);
		panel_larbyPage.add(button_buildRoom);
		panel_larbyPage.add(button_enterRoom);

		panel_loading = new JLayeredPane();		
		panel_loading.setBorder(new LineBorder(new Color(0, 0, 0), 6, true));
		panel_loading.setForeground(Color.CYAN);
		panel_loading.setBackground(Color.CYAN);
		panel_loading.setVisible(false);

		button_backToWPage = new JButton("Back");
		button_backToWPage.setBounds(174, 427, 152, 25);
		panel_larbyPage.add(button_backToWPage);
		button_backToWPage.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				panel_welcomePage.setVisible(true);
				panel_larbyPage.setVisible(false);
			}
		});
		
				label_player = new JLabel("Player :");
				label_player.setFont(new Font("Dialog", Font.BOLD, 16));
				label_player.setBackground(Color.RED);
				label_player.setAutoscrolls(true);
				label_player.setBounds(21, 400, 152, 19);
				panel_larbyPage.add(label_player);
		
				lblScore = new JLabel("Score :");
				lblScore.setFont(new Font("Dialog", Font.BOLD, 16));
				lblScore.setAutoscrolls(true);
				lblScore.setBounds(25, 420, 152, 19);
				panel_larbyPage.add(lblScore);
		panel_loading.setBounds(10, 100, 477, 226);
		panel_larbyPage.add(panel_loading);

		JLabel label_waitingMsg = new JLabel("Waiting For Another Player ^_^");
		label_waitingMsg.setForeground(new Color(51, 255, 0));
		label_waitingMsg.setFont(new Font("Dialog", Font.BOLD, 20));
		label_waitingMsg.setBounds(44, 147, 411, 57);
		panel_loading.add(label_waitingMsg);
		
		label_bgp = new JLabel(new ImageIcon("giphy.gif"));
		label_bgp.setBounds(5, 5, 467, 216);
		panel_loading.add(label_bgp);

		panel_gameBoard = new JPanel();
		panel_gameBoard.setVisible(false);
		panel_gameBoard.setBounds(50, 12, 395, 359);
		panel_larbyPage.add(panel_gameBoard);

		button_pos1 = new JButton("");
		button_pos1.setVisible(false);
		button_pos1.setFocusTraversalKeysEnabled(false);
		button_pos1.setFocusPainted(false);
		button_pos1.setFocusable(false);
		button_pos1.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (map[0] == 0) {
					label_status.setText("pos 1 press");
					map[0] = 1;
					button_pos1.setIcon(new ImageIcon(icons[0]));
					button_pos1.updateUI();
					steps++;
					try {
						outToServer.writeBytes("1\n");
					} catch (IOException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
					if (win()) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
					}
					if (steps == 9) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
						initIcon();
						label_status.setText("PEACE~~~");
					}
				}
			}
		});
		button_pos1.setBackground(new Color(255, 255, 255));
		button_pos1.setBounds(8, 10, 117, 104);
		panel_gameBoard.setLayout(null);
		panel_gameBoard.add(button_pos1);

		button_pos2 = new JButton("");
		button_pos2.setVisible(false);
		button_pos2.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (map[1] == 0) {
					map[1] = 1;
					button_pos2.setIcon(new ImageIcon(icons[0]));
					button_pos2.updateUI();
					steps++;
					try {
						outToServer.writeBytes("2\n");
					} catch (IOException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
					if (win()) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
					}
					if (steps == 9) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
						initIcon();
						label_status.setText("PEACE~~~");
					}
				}
			}
		});
		button_pos2.setBackground(new Color(255, 255, 255));
		button_pos2.setBounds(133, 10, 117, 104);
		panel_gameBoard.add(button_pos2);

		button_pos3 = new JButton("");
		button_pos3.setVisible(false);
		button_pos3.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (map[2] == 0) {
					map[2] = 1;
					button_pos3.setIcon(new ImageIcon(icons[0]));
					button_pos3.updateUI();
					steps++;
					try {
						outToServer.writeBytes("3\n");
					} catch (IOException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
					if (win()) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
					}
					if (steps == 9) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
						initIcon();
						label_status.setText("PEACE~~~");
					}
				}
			}
		});
		button_pos3.setBackground(new Color(255, 255, 255));
		button_pos3.setBounds(258, 10, 117, 104);
		panel_gameBoard.add(button_pos3);

		button_pos4 = new JButton("");
		button_pos4.setVisible(false);
		button_pos4.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (map[3] == 0) {
					map[3] = 1;
					button_pos4.setIcon(new ImageIcon(icons[0]));
					button_pos4.updateUI();
					steps++;
					try {
						outToServer.writeBytes("4\n");
					} catch (IOException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
					if (win()) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
					}
					if (steps == 9) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
						initIcon();
						label_status.setText("PEACE~~~");
					}
				}
			}
		});
		button_pos4.setBackground(new Color(255, 255, 255));
		button_pos4.setBounds(8, 130, 117, 104);
		panel_gameBoard.add(button_pos4);

		button_pos5 = new JButton("");
		button_pos5.setVisible(false);
		button_pos5.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (map[4] == 0) {
					map[4] = 1;
					button_pos5.setIcon(new ImageIcon(icons[0]));
					button_pos5.updateUI();
					steps++;
					try {
						outToServer.writeBytes("5\n");
					} catch (IOException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
					if (win()) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
					}
					if (steps == 9) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
						initIcon();
						label_status.setText("PEACE~~~");
					}
				}
			}
		});
		button_pos5.setBackground(new Color(255, 255, 255));
		button_pos5.setBounds(133, 130, 117, 104);
		panel_gameBoard.add(button_pos5);

		button_pos6 = new JButton("");
		button_pos6.setVisible(false);
		button_pos6.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (map[5] == 0) {
					map[5] = 1;
					button_pos6.setIcon(new ImageIcon(icons[0]));
					button_pos6.updateUI();
					steps++;
					try {
						outToServer.writeBytes("6\n");
					} catch (IOException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
					if (win()) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
					}
					if (steps == 9) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
						initIcon();
						label_status.setText("PEACE~~~");
					}
				}
			}
		});
		button_pos6.setBackground(new Color(255, 255, 255));
		button_pos6.setBounds(258, 130, 117, 104);
		panel_gameBoard.add(button_pos6);

		button_pos7 = new JButton("");
		button_pos7.setVisible(false);
		button_pos7.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (map[6] == 0) {
					map[6] = 1;
					button_pos7.setIcon(new ImageIcon(icons[0]));
					button_pos7.updateUI();
					steps++;
					try {
						outToServer.writeBytes("7\n");
					} catch (IOException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
					if (win()) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
					}
					if (steps == 9) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
						initIcon();
						label_status.setText("PEACE~~~");
					}
				}
			}
		});
		button_pos7.setBackground(new Color(255, 255, 255));
		button_pos7.setBounds(8, 250, 117, 104);
		panel_gameBoard.add(button_pos7);

		button_pos8 = new JButton("");
		button_pos8.setVisible(false);
		button_pos8.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (map[7] == 0) {
					map[7] = 1;
					button_pos8.setIcon(new ImageIcon(icons[0]));
					button_pos8.updateUI();
					steps++;
					try {
						outToServer.writeBytes("8\n");
					} catch (IOException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
					if (win()) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
					}
					if (steps == 9) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
						initIcon();
						label_status.setText("PEACE~~~");
					}
				}
			}
		});
		button_pos8.setBackground(new Color(255, 255, 255));
		button_pos8.setBounds(133, 250, 117, 104);
		panel_gameBoard.add(button_pos8);

		button_pos9 = new JButton("");
		button_pos9.setVisible(false);
		button_pos9.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (map[8] == 0) {
					map[8] = 1;
					button_pos9.setIcon(new ImageIcon(icons[0]));
					button_pos9.updateUI();
					steps++;
					try {
						outToServer.writeBytes("9\n");
					} catch (IOException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
					if (win()) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
					}
					if (steps == 9) {
						try {
							outToServer.writeBytes("_OVER_\n");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
						initIcon();
						label_status.setText("PEACE~~~");
					}
				}
			}
		});
		button_pos9.setBackground(new Color(255, 255, 255));
		button_pos9.setBounds(258, 250, 117, 104);
		panel_gameBoard.add(button_pos9);
		
		label_bgp_larby = new JLabel(new ImageIcon("cat.gif"));
		label_bgp_larby.setBounds(0, 0, 500, 500);
		panel_larbyPage.add(label_bgp_larby);
		button_enterRoom.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				try {
					outToServer.writeBytes("_ENTER_\n");
					panel_loading.setVisible(true);
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
					panel_loading.setVisible(true);
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
