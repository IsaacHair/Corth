import java.io.*;
public class corthvsp {  
    public static void main(String[] args) {  
	if (args.length != 2) {
	    System.out.print("Error 0x01\n"+
			     "usage: java cvm <source> <target>\n");
	    System.exit(0x01);
	}
	try (InputStream sfd = new FileInputStream(args[0]);
	     OutputStream tfd = new FileOutputStream(args[1]);) {
	    String s = ": bitch\n";
	    byte[] arr = new byte[s.length()];
	    for (int i = 0; i < s.length(); i++)
		arr[i] = (byte)s.charAt(i);
	    for (int c = sfd.read(); c != -1; c = sfd.read()) {
		tfd.write(c);
		tfd.write(arr);
	    }
	}
	catch (Exception e) {
	    System.out.println("Error 0x02\nunable to open files");
	    System.exit(0x02);
	}
    }
}  
