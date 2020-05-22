import java.io.*;
import java.util.*;
public class corthvsp {  
    public static void main(String[] args) {  
	if (args.length != 2) {
	    System.out.println("Error 0x01\nNeed source and target");
	    System.exit(0x01);
	}
	ArrayList<String> rawLines = new ArrayList<String>();
	try {
		FileInputStream source = new FileInputStream(args[0]);
		String buff = new String();
		int c = ' ';
		while (c != -1) {
			while ((c = source.read()) != '\n' && c != -1)
				buff += Character.toString(c);
			rawLines.add(buff);
			buff = new String();
		}
	}
	catch (Exception e) {
		System.out.println("Error 0x02\nUnable to open source");
		System.exit(0x02);
	}
	Program progLines = new Program(rawLines);
    }
}
class Tokens {
	private boolean typeif;
	private boolean typeelse;
	private boolean typeasn;
	private boolean typeAdrNotIO;
	private String[] args;
	public Tokens(String line) {

}
class Program {
	private ArrayList<Tokens> tokenLines;
	public Program(ArrayList<String> rawlines) {
		tokenLines = new ArrayList<Tokens>();
		for (String line : rawlines)
			tokenLines.add(new Tokens(line));
	}
}
