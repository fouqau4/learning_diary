import java.awt.*;
import javax.swing.*;


public class hw5 {
	public static void main(String[] args)
	{
		JFrame demo = new JFrame();
        demo.setSize(400, 300);
        demo.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
         
        JCheckBox checkbox = new JCheckBox("JCheckBox");
        JRadioButton radiobutton = new JRadioButton("JRadiobutton");
        JButton button = new JButton("JButton");
        JLabel label = new JLabel("JLabel");
        JTextArea textarea = new JTextArea( String.valueOf( summation( 10 ) ) );
        
        button.doClick();
        textarea.append( "\nhello~~\n" );
         
        demo.getContentPane().add(BorderLayout.EAST, checkbox);
        demo.getContentPane().add(BorderLayout.WEST, radiobutton);
        demo.getContentPane().add(BorderLayout.SOUTH, button);
        demo.getContentPane().add(BorderLayout.NORTH , label);
        demo.getContentPane().add(BorderLayout.CENTER, textarea);
        
        
        
        demo.setVisible(true);
		
	}
	public static int summation(int n)
	{
		int result = ( n * ( n + 1 ) ) / 2;
		return result;
	}
}
