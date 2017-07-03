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

/* SPF+SL imports */
import gov.nasa.jpf.symbc.seplogic.SeplogicBinop;
import gov.nasa.jpf.symbc.seplogic.SeplogicExpression;
import gov.nasa.jpf.symbc.seplogic.SeplogicValue;
import gov.nasa.jpf.symbc.seplogic.SeplogicVariable;

/* CVC4 imports */
import edu.nyu.acsys.CVC4.Expr;
import edu.nyu.acsys.CVC4.ExprManager;
import edu.nyu.acsys.CVC4.Kind;

public class BinopExpr extends gov.nasa.jpf.symbc.seplogic.BinopExpr implements SeplogicExpression, CVC4Convertible {

    public BinopExpr(SeplogicBinop b, SeplogicVariable l, SeplogicValue v) {
	super(b, l, v);
    }
    
    public Expr toCVC4Expr(ExprManager em) {

	Expr lExpr = ((CVC4Convertible) getLhs()).toCVC4Expr(em);
	Expr vExpr = ((CVC4Convertible) getRhs()).toCVC4Expr(em);
	
	switch(getOp()) {
	case EQ: return em.mkExpr(Kind.EQUAL, lExpr, vExpr);
	case NE: return em.mkExpr(Kind.NOT, em.mkExpr(Kind.EQUAL, lExpr, vExpr));
	default: return null; // FIXME: throw exception
	}
    }
}
