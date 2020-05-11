import java.io.*;
import java.util.*;
public class corthvsp {  
    public static void main(String[] args) throws Exception {  
        if (args.length != 2) {
	    System.out.println("Error 0x01\n"+
			       "usage: java corthvsp <source> <target>");
	    System.exit(0x01);
	}
	InputStream source = new FileInputStream(args[0]);
	ArrayList<line> program = new ArrayList<line>();
        int c, i, pos;
	pos = 0;
	while (source.available() > 0) {
	    i = 0;
	    for ((c = source.read()) != '\n' && c != -1)
		i++;
	    
	source.close();
    }
}

class line {
    public void line() {

    }
}
