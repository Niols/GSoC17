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

/* SPF+SL imports */
import gov.nasa.jpf.symbc.seplogic.PointstoExpr;
import gov.nasa.jpf.symbc.seplogic.SatResult;
import gov.nasa.jpf.symbc.seplogic.SeplogicExpression;
import gov.nasa.jpf.symbc.seplogic.SeplogicRecord;
import gov.nasa.jpf.symbc.seplogic.SeplogicValue;
import gov.nasa.jpf.symbc.seplogic.SeplogicVariable;
import gov.nasa.jpf.symbc.seplogic.SL;

public class PathCondition {
    
    private static Set<SeplogicExpression> staticConstraints = null;
    private Set<SeplogicExpression> constraints;
    private UnionFind<SeplogicVariable> aliases;

    public PathCondition(Set<SeplogicExpression> constraints,
			 UnionFind<SeplogicVariable> aliases) {
	this.constraints = constraints;
	this.aliases = aliases;
    }
    
    public PathCondition() {

	if (staticConstraints == null)
	    this.constraints = new HashSet<SeplogicExpression>();
	else
	    this.constraints = new HashSet<SeplogicExpression>(staticConstraints);

	this.aliases = new HashMapUnionFind<SeplogicVariable>();
    }
    
    public static void addStaticConstraint(SeplogicExpression constraint) {
	if (staticConstraints == null)
	    staticConstraints = new HashSet<SeplogicExpression>();
	staticConstraints.add(constraint);
    }

    /** Returns the separation logic expression corresponding to that
     * path condition. */
    public SeplogicExpression toSeplogicExpression() {

	/* We first recover the constraints, and add all the equalities. */
	Set<SeplogicExpression> allConstraints = new HashSet<SeplogicExpression>(constraints);
	for (UnionFind.Entry<SeplogicVariable> equality : aliases.getAll()) {
	    allConstraints.add(SL.Eq(equality.getNonRepresentant(),
				     equality.getRepresentant()));
	}
	
	SeplogicExpression[] dummy = {};
	return SL.Star(allConstraints.toArray(dummy));
    }
    
    /**
     * Shallow cloning of the path condition. Deep cloning is not
     * needed because the only modifications that are authorised on
     * separation logic expressions are local.
     */
    public PathCondition clone() {
	return new PathCondition(new HashSet<SeplogicExpression>(constraints),
				 new HashMapUnionFind(aliases));
    }

    public String toString() {
	return "PC" + (isUnsat() ? "[UNSAT]" : "") + ": " + toSeplogicExpression().toString();
    }

    /**
     * Finds anything that talks about l in the path condition. It
     * then updates the constraint to represent the new constraint,
     * where the field f of l is the reference v.
     */
    public void updateField(SeplogicVariable l, String f, SeplogicVariable v) {

	SeplogicVariable lRepr = aliases.find(l);
	SeplogicVariable vRepr = aliases.find(v);
	
	//FIXME: handle unfolding

	for (SeplogicExpression constraint : constraints) {

	    if (constraint instanceof PointstoExpr) {
		PointstoExpr pe = (PointstoExpr) constraint;

		if (aliases.find(pe.getPointer()).equals(lRepr)) {
		    SeplogicRecord r = (SeplogicRecord) pe.getTarget();

		    constraints.remove(constraint);
		    constraints.add(SL.Pointsto(lRepr, r.update(f, vRepr)));
		    
		    return;
		}
	    }
	}

	assert (false);
    }

    public void addEq(SeplogicVariable x, SeplogicValue v) {
	if (v instanceof SeplogicVariable) {
	    aliases.union(x, (SeplogicVariable) v);
	} else {
	    _star(SL.Eq(x,v));
	}
    }

    public void addPointsto(SeplogicVariable x, SeplogicValue e) {
	_star(SL.Pointsto(x, e));
    }

    private void _star(SeplogicExpression e) {
	constraints.add(e);
    }

    /** Test the unsatisfiability of the constraint. */
    public boolean isUnsat() {
	return false;
    }
    
    // /**
    //  * Tests the satisfiability of the constraint. When in doubt, this
    //  * is considered satisfiable, because we only want to prune pathes
    //  * if we are sure that they are unsatisfiable. 
    //  */
    // public boolean isSatisfiable() {
    // 	return true;
    // 	// //FIXME: replace by isUnsatisfiable, which makes more sense.
    // 	// switch(SL.getProver().isSatisfiable(toSeplogicExpression())) {
    // 	// case UNSAT: return false;
    // 	// case SAT: case UNKNOWN: return true;
    // 	// case ERROR: default: return true; //FIXME:throw exception
    // 	// }
    // }
}
