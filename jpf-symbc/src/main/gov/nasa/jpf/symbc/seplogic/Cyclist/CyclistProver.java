/*
 * Copyright (C) 2014, United States Government, as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All rights reserved.
 *
 * Symbolic Pathfinder (jpf-symbc) is licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
//Copyright (C) 2006 United States Government as represented by the
//Administrator of the National Aeronautics and Space Administration
//(NASA).  All Rights Reserved.
//
//This software is distributed under the NASA Open Source Agreement
//(NOSA), version 1.3.  The NOSA has been approved by the Open Source
//Initiative.  See the file NOSA-1.3-JPF at the top of the distribution
//directory tree for the complete NOSA document.
//
//THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF ANY
//KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT
//LIMITED TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO
//SPECIFICATIONS, ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
//A PARTICULAR PURPOSE, OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT
//THE SUBJECT SOFTWARE WILL BE ERROR FREE, OR ANY WARRANTY THAT
//DOCUMENTATION, IF PROVIDED, WILL CONFORM TO THE SUBJECT SOFTWARE.
//

package gov.nasa.jpf.symbc.seplogic.Cyclist;

/* Java imports */
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.BufferedReader;
import java.io.InputStreamReader;

import java.io.FileReader;

/* SPL+SL imports */
import gov.nasa.jpf.symbc.seplogic.SeplogicExpression;
import gov.nasa.jpf.symbc.seplogic.SeplogicProver;
import gov.nasa.jpf.symbc.seplogic.SL;
import gov.nasa.jpf.symbc.seplogic.SatResult;

public class CyclistProver implements SeplogicProver {
    public CyclistProver() {
    }

    @Override
    public SatResult isSatisfiable(SeplogicExpression e) {

	CyclistConvertible c = (CyclistConvertible) e;

	String formula = c.toCyclistString();

	String contents = "goal {\n  " + formula + " => goal(";
	Object[] vars = e.getFreeVariables().toArray();
	if (vars.length > 0) {
	    for (int i = 0; i < vars.length - 1; i++)
		contents += ((CyclistConvertible) vars[i]).toCyclistString() + ",";
	    contents += ((CyclistConvertible) vars[vars.length-1]).toCyclistString();
	}
	contents += ")\n}";
	for (String predicateDefinition : c.cyclistPredicateDefinitions()) {
	    contents += ";\n" + predicateDefinition;
	}

	//if (SL.debugMode) System.out.println("CyclistProver: content is:\n" + contents);

	String filename = "/tmp/satcheck";

	/* Write formula to file */
	BufferedWriter bw = null;
	FileWriter fw = null;
	try {
	    fw = new FileWriter(filename);
	    bw = new BufferedWriter(fw);
	    bw.write(contents);
	}
	catch (IOException ex) {
	    ex.printStackTrace();
	}
	finally {
	    try {
		if (bw != null) bw.close();
		if (fw != null) fw.close();
	    } catch (IOException ex) {
		ex.printStackTrace();
	    }
	}

	SatResult satResult = SatResult.ERROR;

	/* Run sl_satcheck on that file */
	try {
	    /* -f tells cyclist to check only the first definition;
             * -D tells cyclist that we will provide a filename */
	    String[] cmd = {"sl_satcheck.native", "-f", "-D", filename};
	    ProcessBuilder procBuilder = new ProcessBuilder(cmd);
	    procBuilder.redirectErrorStream(true);
	    Process proc = procBuilder.start();

	    BufferedReader stdout = new BufferedReader(new InputStreamReader(proc.getInputStream()));
	    String line = stdout.readLine();
	    //if (SL.debugMode) System.out.println("CyclistProver: Cyclist's output was:");
	    while (line != null) {
		if (line.startsWith("SAT")) satResult = SatResult.SAT;
		if (line.startsWith("UNSAT")) satResult = SatResult.UNSAT;
		if (line.startsWith("UNKNOWN")) satResult = SatResult.UNKNOWN;

		//if (SL.debugMode) System.out.println("CyclistProver:   " + line);
		line = stdout.readLine();
	    }
	    stdout.close();
	}
	catch (IOException ex) {
	    ex.printStackTrace();
	}

	return satResult;
    }
}
