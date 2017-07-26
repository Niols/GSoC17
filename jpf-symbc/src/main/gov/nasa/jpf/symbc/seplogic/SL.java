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

import java.util.Arrays;
import java.util.LinkedList;
import java.util.Set;
import java.util.HashSet;

/* JPF imports */
import gov.nasa.jpf.Config;
import gov.nasa.jpf.vm.FieldInfo;

/* SPF imports */
import gov.nasa.jpf.symbc.numeric.SymbolicInteger;

public class SL {

    public static boolean enabled;
    public static boolean debugMode;
    private static ProverBackend backend = ProverBackend.None;

    private static LinkedList<SeplogicVariable> knownVariables = null;

    public static void init (Config conf) {

	enabled = conf.getBoolean("symbolic.seplogic", false);

	debugMode = conf.getBoolean("symbolic.debug", false)
	    || conf.getBoolean("symbolic.seplogic.debug", false);

	String[] confBackend = conf.getStringArray("symbolic.seplogic.backend");
	if (confBackend != null) {
	    switch(confBackend[0].toLowerCase()) {

	    case "none":
		backend = ProverBackend.None;
		break;

	    case "cvc4":
		backend = ProverBackend.CVC4;
		System.loadLibrary("cvc4jni");
		break;

	    case "cyclist":
		backend = ProverBackend.Cyclist;
		break;
		
	    default:
		System.out.println("Unknown prover backend: " + confBackend[0]);
	    }
	}

	if (debugMode) {
	    System.out.println("symbolic.seplogic="+enabled);
	    System.out.println("symbolic.seplogic.debug=" + debugMode);
	    System.out.println("symbolic.seplogic.backend=" + backend);

	    System.out.println("Smoke detector:");

	    knownVariables = new LinkedList<SeplogicVariable>();

	    SeplogicVariable p = SL.Variable(new SymbolicInteger(), IntType());
	    SeplogicVariable q = SL.Variable(new SymbolicInteger(), IntType());
	    SeplogicVariable a = SL.Variable(new SymbolicInteger(), IntType());
	    SeplogicVariable b = SL.Variable(new SymbolicInteger(), IntType());

	    SeplogicExpression sat = SL.Star(Pointsto(p, a), Pointsto(q, a));
	    System.out.println("[" + getProver().isSatisfiable(sat) + "] " + sat);
	    
	    SeplogicExpression unsat = SL.Star(SL.Star(Pointsto(p, a),Pointsto(p, b)),Ne(a, b));
	    System.out.println("[" + getProver().isSatisfiable(unsat) + "] " + unsat);
	}
    }

    public static ProverBackend getBackend() {
	return backend;
    }

    public static SeplogicProver getProver() {
	switch(getBackend()) {
	case CVC4: return new gov.nasa.jpf.symbc.seplogic.CVC4.CVC4Prover();
	case Cyclist: return new gov.nasa.jpf.symbc.seplogic.Cyclist.CyclistProver();
	case None: default: return new DummyProver();
	}
    }

    /* Seplogic expressions and values constructors */

    public static PointstoExpr Pointsto(SeplogicVariable l, SeplogicValue v) {
	switch(getBackend()) {
	case CVC4: return new gov.nasa.jpf.symbc.seplogic.CVC4.PointstoExpr(l, v);
	case Cyclist: return new gov.nasa.jpf.symbc.seplogic.Cyclist.PointstoExpr(l, v);
	case None: default: return new PointstoExpr(l, v);
	}
    }

    public static StarExpr Star(SeplogicExpression[] exprs) {
	switch(getBackend()) {
	case CVC4: return new gov.nasa.jpf.symbc.seplogic.CVC4.StarExpr(exprs);
	case Cyclist: return new gov.nasa.jpf.symbc.seplogic.Cyclist.StarExpr(exprs);
	case None: default: return new StarExpr(exprs);
	}
    }

    public static StarExpr Star(SeplogicExpression P, SeplogicExpression Q) {
	switch(getBackend()) {
	case CVC4: return new gov.nasa.jpf.symbc.seplogic.CVC4.StarExpr(P, Q);
	case Cyclist: return new gov.nasa.jpf.symbc.seplogic.Cyclist.StarExpr(P, Q);
	case None: default: return new StarExpr(P, Q);
	}
    }

    public static BinopExpr Binop(SeplogicBinop b, SeplogicVariable l, SeplogicValue v) {
	switch(getBackend()) {
	case CVC4: return new gov.nasa.jpf.symbc.seplogic.CVC4.BinopExpr(b, l, v);
	case Cyclist: return new gov.nasa.jpf.symbc.seplogic.Cyclist.BinopExpr(b, l, v);
	case None: default: return new BinopExpr(b, l, v);
	}
    }

    public static SeplogicVariable Variable(SymbolicInteger n, SeplogicType t) {
	SeplogicVariable v;

	switch(getBackend()) {
	case CVC4: v = new gov.nasa.jpf.symbc.seplogic.CVC4.SeplogicVariable(n, t); break;
	case Cyclist: v = new gov.nasa.jpf.symbc.seplogic.Cyclist.SeplogicVariable(n, t); break;
	case None: default: v = new SeplogicVariable(n, t); break;
	}

	knownVariables.add(v);
	return v;
    }

    public static SeplogicRecord Record(String[] keys, SeplogicVariable[] values) {
	switch(getBackend()) {
	case CVC4: return new gov.nasa.jpf.symbc.seplogic.CVC4.SeplogicRecord(keys, values);
	case Cyclist: return new gov.nasa.jpf.symbc.seplogic.Cyclist.SeplogicRecord(keys, values);
	case None: default: return new SeplogicRecord(keys, values);
	}
    }

    private static NullValue nullValue = null;
    public static NullValue Null() {
	if (nullValue == null) {
	    switch(getBackend()) {
	    case CVC4: nullValue = new gov.nasa.jpf.symbc.seplogic.CVC4.NullValue(); break;
	    case Cyclist: nullValue = new gov.nasa.jpf.symbc.seplogic.Cyclist.NullValue(); break;
	    case None: default: nullValue = new NullValue(); break;
	    }
	}
	return nullValue.copy();
    }

    private static TrueExpr trueExpr = null;
    public static TrueExpr True() {
	if (trueExpr == null) {
	    switch(getBackend()) {
	    case CVC4: trueExpr = new gov.nasa.jpf.symbc.seplogic.CVC4.TrueExpr(); break;
	    case Cyclist: trueExpr = new gov.nasa.jpf.symbc.seplogic.Cyclist.TrueExpr(); break;
	    case None: default: trueExpr = new TrueExpr(); break;
	    }
	}
	return trueExpr.copy();
    }

    private static FalseExpr falseExpr = null;
    public static FalseExpr False() {
	if (falseExpr == null) {
	    switch(getBackend()) {
	    case CVC4: falseExpr = new gov.nasa.jpf.symbc.seplogic.CVC4.FalseExpr(); break;
	    case Cyclist: falseExpr = new gov.nasa.jpf.symbc.seplogic.Cyclist.FalseExpr(); break;
	    case None: default: falseExpr = new FalseExpr(); break;
	    }
	}
	return falseExpr.copy();
    }

    /* **********************[ Predicates ]********************** */

    public static TreePredicate TreePredicate(Set<String> labels) {
	switch(getBackend()) {
	case None: default: return new TreePredicate(labels);
	}
    }
    
    public static TreeExpr Tree(SeplogicVariable variable, Set<String> labels) {
	switch(getBackend()) {
	case Cyclist: return new gov.nasa.jpf.symbc.seplogic.Cyclist.TreeExpr(variable, labels);
	case None: default: return new TreeExpr(variable, labels);
	}
    }
    
    /* Types */

    public static IntType IntType() {
	switch(getBackend()) {
	case CVC4: return new gov.nasa.jpf.symbc.seplogic.CVC4.IntType();
	case Cyclist: case None: default: return new IntType();
	}
    }

    public static RecordType RecordType(String[] keys) {
	switch(getBackend()) {
	case CVC4: return new gov.nasa.jpf.symbc.seplogic.CVC4.RecordType(keys);
	case Cyclist: case None: default: return new RecordType(keys);
	}
    }

    /* Convenient overloaded constructors */

    /* Variable */

    public static SeplogicVariable Variable(SymbolicInteger n) throws UnknownVariableException {
	for (SeplogicVariable v : knownVariables) {
	    if (v.getSymbolic().equals(n))
		return v;
	}
	throw new UnknownVariableException();
    }

    public static SeplogicVariable freshVariable(SeplogicType t) {
	return Variable(new SymbolicInteger(), t);
    }
    
    /* PointstoExpr */

    public static PointstoExpr Pointsto(SymbolicInteger n, SeplogicValue v) {
	return Pointsto(Variable(n, SL.IntType()), v);
    }

    public static PointstoExpr Pointsto(SymbolicInteger n, SymbolicInteger v, SeplogicType t) {
	return Pointsto(n, Variable(v, t));
    }

    public static PointstoExpr Pointsto(SymbolicInteger n, SymbolicInteger v) throws UnknownVariableException {
	return Pointsto(n, Variable(v));
    }

    /* BinopExpr */

    public static BinopExpr Eq(SeplogicVariable l, SeplogicValue v) {
	return Binop(SeplogicBinop.EQ, l, v);
    }

    public static BinopExpr Eq(SymbolicInteger n, SeplogicType t, SeplogicValue v) {
	return Eq(Variable(n, t), v);
    }

    public static BinopExpr Eq(SymbolicInteger n, SeplogicType t, SymbolicInteger m, SeplogicType s) {
	return Eq(n, t, Variable(m, s));
    }

    public static BinopExpr Eq(SymbolicInteger n, SeplogicValue v) throws UnknownVariableException {
	return Eq(Variable(n), v);
    }

    public static BinopExpr Eq(SymbolicInteger n, SymbolicInteger m) throws UnknownVariableException {
	return Eq(n, Variable(m));
    }

    public static BinopExpr Ne(SeplogicVariable l, SeplogicValue v) {
	return Binop(SeplogicBinop.NE, l, v);
    }

    /* SeplogicRecord */

    public static SeplogicRecord Record(FieldInfo[] fields, SymbolicInteger[] integers) {
	String[] keys = new String[fields.length];
	for (int i = 0; i < fields.length; i++)
	    keys[i] = fields[i].getName();

	SeplogicVariable[] values = new SeplogicVariable[integers.length];
	for (int i = 0; i < integers.length; i++)
	    values[i] = Variable(integers[i], IntType());

	return Record(keys, values);
    }

    /* Parser for predicates */

    public static SeplogicPredicate predicateOfString(String repr) {

	String name = repr.substring(0, repr.indexOf('('));
	String[] arguments = repr.substring(repr.indexOf('(') + 1, repr.indexOf(')')).split(",");

	if (name.equalsIgnoreCase("tree")) {
	    return TreePredicate(new HashSet<String>(Arrays.asList(arguments)));
	}
	else {
	    System.out.println("ERROR: Unknown predicate: " + name);
	    return null;
	}

	// this point should never be reached
    }
}
