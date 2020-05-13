import java.io.*;
import java.util.*;
public class corthvsp throws Exception {  
    public static void main(String[] args) {  
	if (args.length != 2) {
	    System.out.println("need source and target");
	    System.exit(0x01);
	}
	ArrayList<line> sourceline = new ArrayList<line>();
	ArrayList<Character> expandedline = new ArrayList<Character>();
	FileInputStream source = new FileInputStream(args[0]);
	int c;
	do {
	    c = source.read();
	    if (c == '\n' || c == -1) {
		sourceline.add(new line(expandedline));
		expandedline.clear();
	    }
	    else
		expandedline.add((char)c);
	} while (c != -1);
    }
}

class line {
    char[] line;
    public void line(ArrayList<Character> expandedline) {
	for (int i = 0; i < expandedline.size(); i++)
	    
    }
}
