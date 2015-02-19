package edu.usfca.vas.app;

import edu.usfca.vas.machine.fa.FAMachine;
import edu.usfca.vas.machine.fa.FAState;
import edu.usfca.xj.appkit.app.XJApplication;
/*

[The "BSD licence"]
Copyright (c) 2005 Jean Bovet
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

public class UnitTests {

    public static void test() {
        FAMachine machine = new FAMachine();
        machine.setType(FAMachine.MACHINE_TYPE_NFA);
        machine.addState(new FAState("A", true, false));
        machine.addState(new FAState("B", false, true));
        machine.addTransitionPattern("A", "#", "A");
        machine.addTransitionPattern("A", "1", "B");
        machine.setSymbolsString("01");
        System.out.println(machine.check());
        System.out.println(machine.accept("010"));
    }

    public static void main(String args[]) {
//        XJApplication.setPropertiesPath("edu/usfca/vas/properties/");
        test();
    }
}
