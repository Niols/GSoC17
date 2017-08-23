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
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

/* SPF imports */
import gov.nasa.jpf.symbc.SymbolicInstructionFactory;
import gov.nasa.jpf.symbc.numeric.SymbolicInteger;

public class PathCondition
{
    /* Static part of the path condition */
 
    private static Map<String,SymbolicInteger> staticVariables = null;
    private static String[] preconditionStrings = null;
    private static Constraint staticConstraint = null;

    public static void addStaticVariable(String string, SymbolicInteger symint) {
	if (staticVariables == null) {
	    staticVariables = new HashMap<String,SymbolicInteger>();
	}
	staticVariables.put(string, symint);
    }

    public static void addPreconditions(String[] strings) {
	preconditionStrings = strings;
    }

    private static SymbolicInteger getStaticVariable(String representation) {
	SymbolicInteger variable = staticVariables.get(representation);
	if (variable == null) {
	    throw new ParseException("Unknown variable: " + representation);
	} else {
	    return variable;
	}
    }

    private static String[] getPreconditionStrings() {
	if (preconditionStrings == null) {
	    return new String[0];
	} else {
	    return preconditionStrings;
	}
    }
    
    private static Constraint getStaticConstraint() {
	if (staticConstraint == null) {
	    staticConstraint = new Constraint();

	    try {
	    
		for (String precondition : getPreconditionStrings()) {
		    String trimedPrecondition = precondition.replaceAll("\\s", "");
		    int index;

		    /* Is this a disequality? */
		    index = trimedPrecondition.indexOf("!=");
		    if (index >= 0) {
			String var1 = trimedPrecondition.substring(0, index);
			String var2 = trimedPrecondition.substring(index+2); //2

			if (var1.toLowerCase().equals("nil")) {
			    staticConstraint.addNonNil(getStaticVariable(var2));
			} else if (var2.toLowerCase().equals("nil")) {
			    staticConstraint.addNonNil(getStaticVariable(var1));
			} else {
			    staticConstraint.addNeq(getStaticVariable(var1), getStaticVariable(var2));
			}
		    
			continue;
		    }

		    /* Is this an equality? */
		    index = trimedPrecondition.indexOf('=');
		    if (index >= 0) {
			String var1 = trimedPrecondition.substring(0, index);
			String var2 = trimedPrecondition.substring(index+1);
		    
			if (var1.toLowerCase().equals("nil")) {
			    staticConstraint.addNil(getStaticVariable(var2));
			} else if (var2.toLowerCase().equals("nil")) {
			    staticConstraint.addNil(getStaticVariable(var1));
			} else {
			    staticConstraint.addEq(getStaticVariable(var1), getStaticVariable(var2));
			}

			continue;
		    }

		    /* Is this a predicate? */
		    index = trimedPrecondition.indexOf("->");
		    if (index >= 0) {
			String var = trimedPrecondition.substring(0, index);
			String pred = trimedPrecondition.substring(index+2); //2
			
			int indexParen = pred.indexOf('(');
			String predName = pred.substring(0, indexParen).toLowerCase();
			String[] arguments = pred.substring(indexParen+1,pred.length()-1).split(",");

			if (predName.equals("tree")) {
			    staticConstraint.addPredicate(getStaticVariable(var), new Tree(arguments));
			} else {
			    throw new ParseException("unknown predicate: " + predName);
			}
							
			continue;
		    }

		    /* Error: could not parse. */
		    throw new ParseException("could not parse: " + precondition);
		}
	    } catch(UnsatException e) {
		throw new UnsatRuntimeException(e.getMessage());
	    }

	    if (SymbolicInstructionFactory.debugMode) {
		System.out.println("Precondition is: " + staticConstraint.toString());
	    }
	}
	return staticConstraint.clone();
    }
    
    /* Dynamic part */

    private Constraint constraint;
    private boolean unsat;

    public PathCondition(Constraint constraint, boolean unsat) {
	this.constraint = constraint;
	this.unsat = unsat;
    }
    
    public PathCondition() {
	this(getStaticConstraint(), false);
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
