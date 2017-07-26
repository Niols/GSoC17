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

import java.util.HashSet;
import java.util.Set;
import java.util.StringJoiner;

public class TreeExpr implements AppliedPredicate {
    
    private final Set<String> labels;
    private final SeplogicVariable variable;

    public TreeExpr(SeplogicVariable variable, Set<String> labels) {
	this.variable = variable;
	this.labels = labels;
    }

    public String toString(boolean withTypes) {
	StringJoiner joiner = new StringJoiner(",");
	for (String label : labels) joiner.add(label);

	return variable.toString(withTypes) + " -> Tree(" + joiner.toString() + ")";
    }
    
    public String toString() {
	return toString(false);
    }
    
    public TreeExpr simplify() {
	return this;
    }
    
    public Set<SeplogicVariable> getFreeVariables() {
	Set<SeplogicVariable> s = new HashSet<SeplogicVariable>();
	s.add(variable);
	return s;
    }

    public Set<SeplogicVariable> getConstrainedVariables() {
	Set<SeplogicVariable> s = new HashSet<SeplogicVariable>();
	s.add(variable);
	return s;
    }

    public SeplogicVariable getVariable() {
	return variable;
    }

    public Set<String> getLabels() {
	return labels;
    }
    
    public TreePredicate getPredicate() {
	return SL.TreePredicate(getLabels());
    }

    public Set<SeplogicExpression> unfold() {
	
	/* Get all the labels as an array. */
	String[] dummy = {};
	String[] labelsArray = labels.toArray(dummy);

	/* Associate a fresh variable to each label. */
	SeplogicVariable[] freshVars = new SeplogicVariable[labelsArray.length];
	for (int i = 0; i < freshVars.length; i++)
	    freshVars[i] = SL.freshVariable(SL.IntType());
	
	SeplogicExpression[] se = new SeplogicExpression[1 + freshVars.length];

	//FIXME: se[0] = SL.Pointsto(variable, SL.Record());
	
	/* Add a predicate for each fresh variable introduced. */
	for (int i = 1 ; i < freshVars.length+1 ; i++)
	    se[i] = getPredicate().apply(freshVars[i]);

	/* Create the set containing this expression and the dummy
	 * var=nil expression. */
	Set<SeplogicExpression> s = new HashSet<SeplogicExpression>();
	s.add(SL.Eq(variable, SL.Null()));
	s.add(SL.Star(se));
	return s;
    }
}
