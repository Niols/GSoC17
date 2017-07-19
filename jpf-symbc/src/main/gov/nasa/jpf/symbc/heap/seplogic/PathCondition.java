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
import java.util.Set;
import java.util.HashSet;

import gov.nasa.jpf.symbc.seplogic.*;

public class PathCondition {
    private static Set<SeplogicExpression> staticConstraints = null;
    private LinkedList<SeplogicExpression> constraints;

    public PathCondition(LinkedList<SeplogicExpression> constraints) {
	this.constraints = constraints;
    }

    public PathCondition() {
	this(new LinkedList<SeplogicExpression>());
    }

    public static void addStaticConstraint(SeplogicExpression constraint) {
	if (staticConstraints == null)
	    staticConstraints = new HashSet<SeplogicExpression>();
	
	staticConstraints.add(constraint);
    }
    
    private void reset() {
	constraints = new LinkedList<SeplogicExpression>();
    }
    
    public SeplogicExpression toSeplogicExpression() {
	SeplogicExpression[] dummy = {};
	
	return SL.Star(SL.Star(constraints.toArray(dummy)), SL.Star(staticConstraints.toArray(dummy)));
    }

    public PathCondition copy() {
	return new PathCondition((LinkedList<SeplogicExpression>) constraints.clone());
    }

    public String toString() {
	boolean sat = SL.getProver().isSatisfiable(toSeplogicExpression());
	//if (! sat) { reset(); _star(SL.False()); }
	
	return "PC[" + (sat ? " sat " : "unsat") + "]: " + toSeplogicExpression().simplify().toString();
    }

    public void updateField(SeplogicVariable l, String f, SeplogicVariable v) {
	//FIXME: handle aliasing.
	
	for (int i = 0; i < constraints.size(); i++) {
	    SeplogicExpression e = constraints.get(i);

	    if (e instanceof PointstoExpr) {
		PointstoExpr pe = (PointstoExpr) e;

		if (pe.getPointer().equals(l)) {
		    SeplogicRecord r = (SeplogicRecord) pe.getTarget();
		    constraints.set(i, SL.Pointsto(l, r.update(f, v)));
		    return;
		}
	    }
	}
	assert (false);
    }
    
    public void _star(SeplogicExpression e) {
	constraints.add(e);
    }

    public boolean isSatisfiable() {
	return true;
    }
}
