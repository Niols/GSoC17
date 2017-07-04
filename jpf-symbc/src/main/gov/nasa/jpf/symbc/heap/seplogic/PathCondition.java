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

import java.util.LinkedList;

import gov.nasa.jpf.symbc.seplogic.*;

public class PathCondition {
    private LinkedList<SeplogicExpression> constraints;

    public PathCondition(LinkedList<SeplogicExpression> constraints) {
	this.constraints = constraints;
    }
    
    public PathCondition() {
	this(new LinkedList<SeplogicExpression>());
    }

    public SeplogicExpression toSeplogicExpression() {
	SeplogicExpression[] dummy = {};
	return SL.Star(constraints.toArray(dummy));
    }

    public PathCondition copy() {
	return new PathCondition((LinkedList<SeplogicExpression>) constraints.clone());
    }

    public String toString() {
	System.out.println("isSatisfiabel: " + SL.getProver().isSatisfiable(toSeplogicExpression()));
	return "PC: " + toSeplogicExpression().simplify().toString();
	
	//FIXME: write a 'public void simplify()' function
    }

    public void updateField(SeplogicVariable l, String f, SeplogicVariable v) {

	for (int i = 0; i < constraints.size(); i++) {
	    SeplogicExpression e = constraints.get(i);
	    
	    if (e instanceof BinopExpr) {
		BinopExpr be = (BinopExpr) e;

		if (be.getOp() == SeplogicBinop.EQ && be.getLhs().equals(l)) {
		    SeplogicRecord r = (SeplogicRecord) be.getRhs();
		    constraints.set(i, SL.Eq(l, r.update(f, v)));
		    return;
		}
	    }
	}

	assert false;
    }
    
    public void _star(SeplogicExpression e) {
	constraints.add(e);
    }

    public boolean isSatisfiable() {
	return true;
    }
}
