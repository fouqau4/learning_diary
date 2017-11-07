import java.net.*;
import java.io.*;

public class server implements Runnable {

	Socket playerSocket;
	int user_num;
	public static Object lock_players_status = new Object();
	public static Object lock_room = new Object();
	private final static int MAX_PLAYERS = 6;
	public static String[] players_name = new String[MAX_PLAYERS];
	public static String[] messages = new String[MAX_PLAYERS];
	public static int[] opponents = new int[MAX_PLAYERS];
	/*
	 * -1 : no opponent 0 ~ MAX_PLAYERS -1 : opponent's user_num
	 */
	public static int[] players_status = new int[MAX_PLAYERS];
	/*
	 * -1 : no player 0 : connect 1 : build room 2 : enter room 3 : pair matched
	 */
	public static Thread[] t = new Thread[MAX_PLAYERS];
	public static DataInputStream[] inFromClient = new DataInputStream[MAX_PLAYERS];
	public static DataOutputStream[] outTOClient = new DataOutputStream[MAX_PLAYERS];

	server(Socket connectSocket, int i) {
		this.playerSocket = connectSocket;
		this.user_num = i;
	}

	public static void main(String[] argv) throws Exception {
		// initial
		for (int i = 0; i < MAX_PLAYERS; i++) {
			players_status[i] = -1;
			opponents[i] = -1;
			messages[i] = null;
		}

		// ServerSocket welcomeSocket = new
		// ServerSocket(Integer.parseInt(argv[0]));
		ServerSocket welcomeSocket = new ServerSocket(8787);

		while (true) {
			System.out.println("Listening~~\n");
			Socket connectionSocket = welcomeSocket.accept();
			System.out.println("A client connects to here!!\n");

			for (int i = 0; i < MAX_PLAYERS; i++) {
				if (players_name[i] == null) {
					players_name[i] = "";
					players_status[i] = 0;
					t[i] = new Thread(new server(connectionSocket, i));
					t[i].start();
					break;
				}
			}
		}
	}

	@SuppressWarnings("deprecation")
	public void run() {
		int steps = 0;
		int prev_opponent = -1;
		System.out.println(playerSocket + "\nuser_num : " + user_num + "\n");
		System.out.println("this thread's id is : " + t[user_num].getId());
		try {
			DataInputStream inFromClient = new DataInputStream(playerSocket.getInputStream());
			DataOutputStream outToClient = new DataOutputStream(playerSocket.getOutputStream());

			players_name[user_num] = inFromClient.readLine();
			System.out.println("Player : " + players_name[user_num] + " login !!\n");

			outToClient.writeBytes("[Server] : waiting for another player ^_^\n");
			System.out.println("wait player : [" + players_name[user_num] + "] to do operation\n");
			// ����
			String temp;

			while (true) {
				System.out.println(players_name[user_num] + "waits another operation!!\n");
				temp = inFromClient.readLine();
				if (temp != null) {
					switch (temp) {
					case "_BUILD_":
						steps = 0;
						System.out.println(players_name[user_num] + "build a room!!\n");
						synchronized (lock_players_status) {
							players_status[user_num] = 1;
						}
						while (true) {
							try {
								System.out.println(players_name[user_num] + " is waiting~~\n");
								Thread.sleep(3000);
							} catch (InterruptedException e) {
								// TODO Auto-generated catch block
								e.printStackTrace();
							}
							if (players_status[user_num] == 3)
								break;
						}
						System.out.println("send _MATCH_ to " + players_name[user_num] + "\n");
						outToClient.writeBytes("_MATCH_\n");
						// outToClient.writeBytes("Match to opponent [" +
						// players_name[opponents[user_num]] + "]!!!\n");
						outToClient.writeBytes("_FIRST_\n");
						// enter game
						while (true) {
							temp = inFromClient.readLine();
							if (temp.equals("_OVER_")) {
								synchronized(lock_room){
									messages[opponents[user_num]] = "end";
								}
								synchronized (lock_players_status) {
									opponents[user_num] = -1;
									messages[user_num] = null;
									players_status[user_num] = 0;
								}
								break;
							}
							synchronized (lock_room) {
								messages[opponents[user_num]] = temp;
								messages[user_num] = "wait";
								steps++;
								System.out.println(players_name[user_num] + " steps " + steps +"\n");
							}
							if (steps == 9){
								System.out.println("*************************\n"+players_name[user_num] + "PEACE PEACE PEACE \n*************************\n");
								inFromClient.readLine();
								outToClient.writeBytes("_END_\n");
								break;
							}
							System.out.println(players_name[user_num] + "waiting opponent to move!\n");

							while (true) {
								Thread.sleep(500);
								synchronized (lock_room) {
									if (!messages[user_num].equals("wait")) {
										System.out.println(
												players_name[user_num] + "receive pos : " + messages[user_num] + "\n");
										if(messages[user_num].equals("end")){
											outToClient.writeBytes("_END_\n");
										}
										else
											outToClient.writeBytes(messages[user_num] + "\n");
										steps++;
										break;
									}
								}
							}

						}						
						// game over
						break;
					case "_ENTER_":
						steps = 0;
						// ��ж�
						while (true) {
							synchronized (lock_players_status) {
								players_status[user_num] = 2;
								for (int i = 0; i < MAX_PLAYERS; i++) {
									if (i != user_num && i != prev_opponent) {
										if (players_status[i] == 1) {
											System.out.println(players_name[user_num] + " find a room!!\n");
											players_status[user_num] = 3;
											players_status[i] = 3;
											prev_opponent = i;
											opponents[user_num] = i;
											opponents[i] = user_num;
											break;
										}
									}
								}
								if(prev_opponent >= 0){
									if (players_status[prev_opponent] == 1) {
										System.out.println(players_name[user_num] + " find a room!!\n");
										players_status[user_num] = 3;
										players_status[prev_opponent] = 3;										
										opponents[user_num] = prev_opponent;
										opponents[prev_opponent] = user_num;										
									}
								}
							}
							if (opponents[user_num] != -1) {
								System.out.println(players_name[user_num] + " sends match message!!\n");
								break;
							}

						}
						//���F
						outToClient.writeBytes("_MATCH_\n");
						// outToClient.writeBytes("Match to opponent [" +
						// players_name[opponents[user_num]] + "]!!!\n");
						outToClient.writeBytes("_SECOND_\n");
						// game start
						while (true) {
							synchronized (lock_room) {
								messages[opponents[user_num]] = temp;
								messages[user_num] = "wait";
							}
							while (true) {
								Thread.sleep(500);
								synchronized (lock_room) {
									if (!messages[user_num].equals("wait")) {
										System.out.println(
												players_name[user_num] + "receive pos : " + messages[user_num] + "\n");
										if(messages[user_num].equals("end")){
											System.out.println(players_name[user_num] +"ENDDDDDDDDD\n");
											outToClient.writeBytes("_END_\n");
										}
										else
											outToClient.writeBytes(messages[user_num] + "\n");
										break;
									}
								}
							}

							temp = inFromClient.readLine();
							if (temp.equals("_OVER_")) {
								synchronized(lock_room){
									messages[opponents[user_num]] = "end";
								}
								synchronized (lock_players_status) {
									opponents[user_num] = -1;
									messages[user_num] = null;
									players_status[user_num] = 0;
								}
								break;
							}
						}
						// game over
						break;
					case "bye":
						outToClient.writeBytes("bye" + "\n");
						break;
					}
				} else {
					playerSocket.close();
					System.out.println("Player : [" + players_name[user_num] + "] leaves!!\n");
					break;
				}
			}
			// next operation
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
}