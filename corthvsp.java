import java.io.*;
import java.util.*;
public class corthvsp {  
    public static void main(String[] args) {  
        if (args.length != 2) {
	    System.out.println("Error 0x01\n"+
			       "usage: java corthvsp <source> <target>");
	    System.exit(0x01);
	}
	try (source = new BufferedReader(new InputStreamReader
					 (new FileInputStream(args[0])));
	     target = new FileOutputStream(args[1]);) {
	ArrayList<lineobj> line = new ArrayList<lineobj>();
	while (source.available() > 0)
	    line.add(source.getLine());
	}
    }
}  
