import java.io.*;
import java.util.*;
public class corthvsp {  
    public static void main(String[] args) {  
        if (args.length != 2) {
	    System.out.println("Error 0x01\n"+
			       "usage: java corthvsp <source> <target>");
	    System.exit(0x01);
	}
	test obj = new test();
	obj.f2();
	obj.a.b.f2();
    }
}

class test {
    private void f1() {
	System.out.print("f1\n");
    }
    class a { class b { public void f2() {
	f1();
        super.super.f1();
	System.out.println("f2");
    } } }
}
