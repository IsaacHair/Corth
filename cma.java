import java.io.*;
import java.util.*;

public class cma {
	public static int main(String[] args) {
		if (args.length != 2) {
			System.out.println("Error 0x01\n"+
			  "Please enter source and target files");
			System.exit(0x01);
		}
		FileInputStream source = new FileInputStream(args[0]);
		RawTokenStream rawTokens = new RawTokenStream(source);
	}
}
class RawTokenStream {
	public RawTokenStream() {

