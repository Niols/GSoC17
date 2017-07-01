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

import gov.nasa.jpf.vm.FieldInfo;
import gov.nasa.jpf.symbc.numeric.SymbolicInteger;
//import config

public class SL {

    private static ProverBackend backend = null;
    public static ProverBackend getBackend() {
	if (backend == null) {
	    // read config
	    backend = ProverBackend.None;
	}
	return backend;
    }

    /* Seplogic expressions and values constructors */

    public static PointstoExpr Pointsto(SeplogicVariable l, SeplogicValue v) {
	switch(getBackend()) {
	case None: return new      PointstoExpr(l, v);
	    //case CVC4: return new CVC4.PointstoExpr(l, v);
	}
	return null; //FIXME: throw exception
    }
    //FIXME: do the same for the others constructors


    /* Convenient overloaded constructors */
    
    /* PointstoExpr */


    public static PointstoExpr Pointsto(SymbolicInteger l, SeplogicValue v) {
	return Pointsto(Variable(l), v);
    }
    
    public static PointstoExpr Pointsto(SymbolicInteger l, SymbolicInteger v) {
	return Pointsto(l, Variable(v));
    }
    
    /* StarExpr */
    
    public static StarExpr Star(SeplogicExpression[] exprs) {
	return new StarExpr(exprs);
    }

    public static StarExpr Star(SeplogicExpression P, SeplogicExpression Q) {
	return new StarExpr(P, Q);
    }

    /* BinopExpr */
    
    public static BinopExpr Binop(SeplogicBinop b, SeplogicVariable l, SeplogicValue v) {
	return new BinopExpr(b, l, v);
    }

    public static BinopExpr Eq(SeplogicVariable l, SeplogicValue v) {
	return Binop(SeplogicBinop.EQ, l, v);
    }

    public static BinopExpr Eq(SymbolicInteger l, SeplogicValue v) {
	return Eq(Variable(l), v);
    }

    public static BinopExpr Eq(SymbolicInteger l, SymbolicInteger v) {
	return Eq(l, Variable(v));
    }

    public static BinopExpr Ne(SeplogicVariable l, SeplogicValue v) {
	return Binop(SeplogicBinop.NE, l, v);
    }

    /* SeplogicValue */

    public static SeplogicVariable Variable(SymbolicInteger n) {
	return new SeplogicVariable(n);
    }

    /* SeplogicRecord */
    
    public static SeplogicRecord Record(String[] keys, SeplogicVariable[] values) {
	return new SeplogicRecord(keys, values);
    }

    public static SeplogicRecord Record(FieldInfo[] fields, SymbolicInteger[] integers) {
	String[] keys = new String[fields.length];
	for (int i = 0; i < fields.length; i++)
	    keys[i] = fields[i].getName();

	SeplogicVariable[] values = new SeplogicVariable[integers.length];
	for (int i = 0; i < integers.length; i++)
	    values[i] = Variable(integers[i]);

	return Record(keys, values);
    }
    
    private static NullValue nullValue = null;
    public static NullValue Null() {
	if (nullValue == null)
	    nullValue = new NullValue();

	return nullValue.copy();
    }
    
    private static TrueExpr trueExpr = null;
    public static TrueExpr True() {
	if (trueExpr == null)
	    trueExpr = new TrueExpr();

	return trueExpr.copy();
    }
}
