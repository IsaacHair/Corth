import java.io.*;
import java.util.Scanner;
public class corthvsp {
    public static void main(String[] args) {
	try {
	    File source = new File(args[0]);
	}
	catch (FileNotFoundException ex) {
	    System.out.print("crap\n");
	}
	Scanner sourcescan = new Scanner(source);
	for (int i = 0; sourcescan.hasNextLine(); i++)
	    System.out.print("L"+i+sourcescan.nextLine());
	System.out.print("hola\n");
	random me = new random(669);
	me.doSomething();
    }
}

class random {
    private int val;
    public random(int num) {
	System.out.print("hola again\n");
	this.val = num;
    }
    void doSomething() {
	System.out.print("nigga\n");
	System.out.print(this.val+"\n");
    }
}
