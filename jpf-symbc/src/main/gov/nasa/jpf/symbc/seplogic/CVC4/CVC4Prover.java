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

package gov.nasa.jpf.symbc.seplogic.CVC4;

/* SPL+SL imports */
import gov.nasa.jpf.symbc.seplogic.SeplogicExpression;
import gov.nasa.jpf.symbc.seplogic.SeplogicProver;
import gov.nasa.jpf.symbc.seplogic.SL;

/* CVC4 imports */
import edu.nyu.acsys.CVC4.Expr;
import edu.nyu.acsys.CVC4.ExprManager;
import edu.nyu.acsys.CVC4.SExpr;
import edu.nyu.acsys.CVC4.Result;
import edu.nyu.acsys.CVC4.SmtEngine;

public class CVC4Prover implements SeplogicProver {
    public CVC4Prover() {
	// FIXME
    }

    /* Getters for the ExprManager and SmtEngine of CVC4. We cache
     * them, because we don't want to spawn a new prover everytime. */
    
    private static ExprManager em = null;
    public static ExprManager getExprManager() {
	if (em == null)
	    em = new ExprManager();
	return em;
    }

    public static SmtEngine getSmtEngine() {
	/* Notes:
	 *
	 * 1. We disable the incremental mode, because it is not
	 *    compatible with separation logic.
	 *
	 * 2. We do not cache the SmtEngine, because querying it
	 *    multiple times requires the incremental mode. 
	 */
	
	SmtEngine smt = new SmtEngine(getExprManager());
	smt.setOption("incremental", new SExpr("false"));
	smt.setLogic("QF_ALL_SUPPORTED");
	return smt;
    }

    /* And what we waited for... the isSatisfiable method. */
    
    @Override
    public boolean isSatisfiable(SeplogicExpression e) {
	Expr formula = ((CVC4Convertible) e).toCVC4Expr(getExprManager());
	Result result = getSmtEngine().checkSat(formula);
	boolean isSat = (result.isSat() == Result.Sat.SAT);

	if (SL.debugMode)
	    System.out.println("CVC4Prover: formula " + formula + " " + (isSat ? "IS" : "IS NOT") + " satisfiable.");
	
	return isSat;
    }
}
