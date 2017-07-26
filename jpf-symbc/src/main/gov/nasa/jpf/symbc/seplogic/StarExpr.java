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
//Copyright (C) 2005 United States Government as represented by the
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

import java.util.HashSet;
import java.util.Set;
import java.util.StringJoiner;

public class StarExpr implements SeplogicExpression {
    private final SeplogicExpression[] exprs;

    public StarExpr(SeplogicExpression[] exprs) {
	this.exprs = exprs;
    }
    
    public StarExpr(SeplogicExpression P, SeplogicExpression Q) {
	exprs = new SeplogicExpression[2];
	exprs[0] = P;
	exprs[1] = Q;
    }

    public String toString(boolean withTypes) {
	StringJoiner joiner = new StringJoiner(" * ");
	for (SeplogicExpression expr : exprs) joiner.add(expr.toString(withTypes));
	return joiner.toString();
    }

    public String toString() {
	return toString(false);
    }

    public StarExpr copy() {
	return this;
    }

    public SeplogicExpression[] getExprs() {
	return exprs.clone();
    }

    public SeplogicExpression simplify() {
	SeplogicExpression[] new_exprs = new SeplogicExpression[exprs.length];

	for (int i = 0; i < exprs.length; i++)
	    new_exprs[i] = exprs[i].simplify();

	// Search for a False in the array
	boolean hasFalse = false;
	for (int i = 0; i < exprs.length; i++)
	    if (new_exprs[i] instanceof FalseExpr)
		hasFalse = true;
	if (hasFalse)
	    return SL.False();

	// Cleanup the True (and NULL) in the array FIXME: much easier
	// to implement by using a function Array -> Array that cleans
	// up the NULL cells (just transform all True into NULL and
	// call that method.)
	int nbTrue = 0;
	for (int i = 0; i < exprs.length; i++)
	    if (new_exprs[i] instanceof TrueExpr || new_exprs[i] == null)
		nbTrue++;
	if (nbTrue > 0) {
	    SeplogicExpression[] new_new_exprs = new SeplogicExpression[exprs.length - nbTrue];
	    int j = 0;
	    for (int i = 0; i < new_new_exprs.length; i++) {
		if (! (new_exprs[i] instanceof TrueExpr || new_exprs[i] == null)) {
		    new_new_exprs[j] = new_exprs[i];
		    j++;
		}
	    }
	    if (new_new_exprs.length == 0)
		return SL.True();
	    else if (new_new_exprs.length == 1)
		return new_new_exprs[0];
	    else
		return SL.Star(new_new_exprs);
	}

	return SL.Star(new_exprs);
    }
    
    // Specific to StarExpr
    public SeplogicExpression[] getExpressions() {
	return exprs;
    }
    
    // Specific to StarExpr
    public StarExpr flatten() {
	SeplogicExpression[][] exprs_flattened = new SeplogicExpression[exprs.length][];
	int new_length = 0;
	
	for (int i = 0; i < exprs.length; i++) {
	    if (exprs[i] instanceof StarExpr) {
		StarExpr expr = ((StarExpr) exprs[i]).flatten();
		exprs_flattened[i] = expr.getExpressions();
		new_length += expr.getExpressions().length;
	    }
	    else {
		exprs_flattened[i] = new SeplogicExpression[1];
		exprs_flattened[i][0] = exprs[i];
		new_length++;
	    }
	}

	SeplogicExpression[] new_exprs = new SeplogicExpression[new_length];
	int k = 0;
	for (int i = 0; i < exprs_flattened.length; i++) {
	    for (int j = 0; j < exprs_flattened[i].length; j++) {
		new_exprs[k] = exprs_flattened[i][j];
		k++;
	    }
	}

	return SL.Star(new_exprs);
    }

    public Set<SeplogicVariable> getFreeVariables() {
	Set<SeplogicVariable> fv = new HashSet<SeplogicVariable>();
	for (SeplogicExpression e : getExpressions())
	    fv.addAll(e.getFreeVariables());
	return fv;
    }

    public Set<SeplogicVariable> getConstrainedVariables() {
	Set<SeplogicVariable> cv = new HashSet<SeplogicVariable>();
	for (SeplogicExpression e : getExpressions())
	    cv.addAll(e.getConstrainedVariables());
	return cv;
    }
}
