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

package gov.nasa.jpf.symbc.seplogic;

import edu.nyu.acsys.CVC4.ExprManager;
import edu.nyu.acsys.CVC4.SmtEngine;

public class PathConstraint {

    /* Static part, CVC4's binding.
     *
     * FIXME: This should only be implemented by classes inheriting
     * from PathConstraint. */
    
    private static boolean loaded = false;
    private static ExprManager em = null;
    private static SmtEngine smt = null;

    public static void loadLibrary() {
	if (! loaded) {
	    System.loadLibrary("cvc4jni");
	    loaded = true;
	}
    }
    
    public static ExprManager getExprManager() {
	if (em == null) {
	    loadLibrary();
	    em = new ExprManager();
	}
	return em;
    }

    public static SmtEngine getSmtEngine() {
	if (smt == null) {
	    smt = new SmtEngine(getExprManager());
	}
	return smt;
    }

    /* Dynamic part, the actual path constraint. */

    protected SeplogicExpression constraint;
    
    public PathConstraint(SeplogicExpression constraint) {
	this.constraint = constraint;
    }

    public PathConstraint() {
	this(new EmpExpression());
    }

    public void setConstraint(SeplogicExpression constraint) {
	this.constraint = constraint;
    }

    public SeplogicExpression getConstraint() {
	if (constraint == null) {
	    return null;
	} else {
	    return this.constraint.copy();
	}
    }

    public PathConstraint copy() {
	return new PathConstraint(getConstraint());
    }

    public String toString() {
	return "PathConstraint(" + constraint + ")";
    }
    
    /* Helpers */

    public void _and(SeplogicExpression e) {
	constraint = new AndExpression(getConstraint(), e);
    }

    public void _star(SeplogicExpression e) {
	constraint = new StarExpression(getConstraint(), e);
    }

    public void _wand(SeplogicExpression e) {
	constraint = new WandExpression(getConstraint(), e);
    }

    public void _wand_rev(SeplogicExpression e) {
	constraint = new WandExpression(e, getConstraint());
    }

    /* The real work */

    public boolean isSatisfiable() {
	if (debugMode)
	    System.out.println("Querying satisfiability of `" + constraint + "`");

	boolean result = smt.query(constraint.translate());

	if (debugMode)
	    System.out.println("Result is: " + result);
	
	return result;
    }
}
