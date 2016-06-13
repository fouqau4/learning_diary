import java.io.*;
import java.net.*;
import javax.swing.*;
import java.awt.*;



public class client{	
	public static void main( String argv[] ) throws Exception
	{
		String sentence;
		String modifiedSentence;
		
		BufferedReader inFromUser =
				new BufferedReader( new InputStreamReader( System.in ) );
		
		//建立視窗
		JFrame window = new JFrame( "OT2" );
		window.setSize( 800, 600 );
		window.setDefaultCloseOperation( JFrame.EXIT_ON_CLOSE );
		
		JPanel Panel_button = new JPanel();
		JButton button_start = new JButton( "Start!!!" );
		JButton button_exit = new JButton( "Exit!!!" );
		Panel_button.setLayout( null );
		button_start.setBounds(350, 390, 100, 50);
		button_exit.setBounds(350, 450, 100, 50);
		Panel_button.add(button_start);
		Panel_button.add(button_exit);
		
		

		window.getContentPane().add(Panel_button);
		window.setVisible( true );
		
		/*
		Socket clientSocket = new Socket( argv[0], Integer.parseInt( argv[1] ) );
		
		DataOutputStream outToServer =
				new DataOutputStream( clientSocket.getOutputStream() );
		
		BufferedReader inFromServer = new BufferedReader( new
				InputStreamReader( clientSocket.getInputStream() ) );
		
		sentence = inFromUser.readLine();
		
		outToServer.writeBytes( sentence + '\n' );
		
		modifiedSentence = inFromServer.readLine();
		
		System.out.println( "FROM SERVER: " + modifiedSentence );
		
		clientSocket.close();
		*/
	}
}
