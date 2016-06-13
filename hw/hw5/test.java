
public class test extends Thread {
	public static int num = 0;
	int n;
	public static Object lock = new Object();

	public test() {
		System.out.println("Thread No. " + ++num + "\n");
		n = num;
	}

	public void run() {
		int k = 2;
		while (k-- > 0) {
			try {
				wait();
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block				
			}
			synchronized (lock) {
				System.out.println("I'm no. " + n + "\n");
			}
		}
	}

	public static void main(String[] args) {
		for (int i = 0; i < 5; i++) {
			new test().start();
		}

	}

}
