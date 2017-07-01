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
import gov.nasa.jpf.symbc.seplogic.SeplogicExpression;
import gov.nasa.jpf.symbc.seplogic.SeplogicVariable;
import gov.nasa.jpf.symbc.seplogic.SL;

public class PathCondition {
    private LinkedList<SeplogicExpression> separatedConstraints;

    public PathCondition(LinkedList<SeplogicExpression> separatedConstraints) {
	this.separatedConstraints = separatedConstraints;
    }
    
    public PathCondition() {
	this(new LinkedList<SeplogicExpression>());
    }

    public SeplogicExpression toSeplogicExpression() {
	SeplogicExpression[] dummy = {};
	return SL.Star(separatedConstraints.toArray(dummy));
    }

    public PathCondition copy() {
	return new PathCondition((LinkedList<SeplogicExpression>) separatedConstraints.clone());
    }

    public String toString() {
	return "PC: " + toSeplogicExpression().simplify().toString();
	//FIXME: write a 'public void simplify()' function
    }

    public void updateField(SeplogicVariable l, String f, SeplogicVariable v) {
	System.out.println("In PC: Here, we should update the constraint to reflect the fact that, now:\n    " + l + "." + f + " -> " + v);
    }
    
    public void _star(SeplogicExpression e) {
	separatedConstraints.add(e);
    }
}
