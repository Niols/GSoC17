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

package gov.nasa.jpf.symbc.heap.seplogic;

import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/* SPF imports */
import gov.nasa.jpf.symbc.SymbolicInstructionFactory;
import gov.nasa.jpf.symbc.numeric.SymbolicInteger;

public class PathCondition
{
    /* Static part of the path condition */

    private static Constraint staticConstraint = null;

    public static void addStaticConstraint(FullPredicate fullPredicate) {
	if (staticConstraint == null)
	    staticConstraint = new Constraint();

	try {
	    staticConstraint.addPredicate(fullPredicate);
	} catch (UnsatException e) {
	    System.err.println("Inconsistent preconditions");
	    //FIXME: here, we must kill SPF and complain that the
	    //preconditions are not sat.
	}
    }
    
    /* Dynamic part */

    private Constraint constraint;
    private boolean unsat;

    public PathCondition(Constraint constraint, boolean unsat) {
	this.constraint = constraint;
	this.unsat = unsat;
    }
    
    public PathCondition() {
	this((staticConstraint != null) ? staticConstraint.clone() : new Constraint(), false);
    }

    /**
     * Shallow cloning of the path condition. Deep cloning is not
     * needed because the only modifications that are authorised on
     * separation logic expressions are local.
     */
    public PathCondition clone() {
	return new PathCondition(constraint.clone(), unsat);
    }

    public String toString() {
	return "PC" + (isUnsat() ? " is now UNSAT. Last consistent state" : "") + ": " + constraint.toString();
    }

    /**
     * Finds anything that talks about l in the path condition. It
     * then updates the constraint to represent the new constraint,
     * where the field f of l is the reference v.
     */
    public void updateField(SymbolicInteger l, String f, SymbolicInteger v) {
	try {
	    this.constraint.updateField(l, f, v);
	} catch (UnsatException e) {
	    if (SymbolicInstructionFactory.debugMode) {
		System.out.println("Adding " + l.hashCode() + "." + f + "->" + v.hashCode() + " made unsatisfiable " + toString());
	    }
	    this.unsat = true;
	}
    }

    public void addNil(SymbolicInteger x) {
	try {
	    this.constraint.addNil(x);
	} catch (UnsatException e) {
	    if (SymbolicInstructionFactory.debugMode) {
		System.out.println("Adding " + x.hashCode() + "=nil made unsatisfiable " + toString());
	    }
	    this.unsat = true;
	}
    }

    public void addEq(SymbolicInteger x, SymbolicInteger y) {
	try {
	    this.constraint.addEq(x, y);
	} catch (UnsatException e) {
	    if (SymbolicInstructionFactory.debugMode) {
		System.out.println("Adding " + x.hashCode() + "=" + y.hashCode() + " made unsatisfiable " + toString());
	    }
	    this.unsat = true;
	}
    }

    public void addNeq(SymbolicInteger x, SymbolicInteger y) {
	try {
	    this.constraint.addNeq(x, y);
	} catch (UnsatException e) {
	    if (SymbolicInstructionFactory.debugMode) {
		System.out.println("Adding " + x.hashCode() + "!=" + y.hashCode() + " made unsatisfiable " + toString());
	    }
	    this.unsat = true;
	}
    }

    public void addRecord(SymbolicInteger x, Map<String,SymbolicInteger> fieldsMap) {
	try {
	    this.constraint.addRecord(x, fieldsMap);
	} catch (UnsatException e) {
	    if (SymbolicInstructionFactory.debugMode) {
		System.out.println("Adding " + x.hashCode() + "->{|...|} made unsatisfiable " + toString());
	    }
	    this.unsat = true;
	}
    }
    
    /** Test the unsatisfiability of the constraint. */
    public boolean isUnsat() {
	return this.unsat;
    }
}
