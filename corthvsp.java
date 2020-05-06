import java.io.*;
import java.util.*;
public class corthvsp {  
    public static void main(String[] args) {  
        if (args.length != 2) {
	    System.out.println("Error 0x01\n"+
			       "usage: java corthvsp <source> <target>");
	    System.exit(0x01);
	}
	FileInputStream source = new FileInputStream(args[0]);
	FileOutputStream target = new FileOutputStream(args[1]);
	ArrayList<line> content = new ArrayList<line>();
	
    }
}  
