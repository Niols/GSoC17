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

package gov.nasa.jpf.symbc.heap.seplogic;

/* Java imports */
import java.util.Set;
import java.util.Map;
import java.util.HashSet;
import java.util.StringJoiner;

/* SPF imports */
import gov.nasa.jpf.symbc.numeric.SymbolicInteger;

public class Several extends Predicate
{
    private final Set<Predicate> predicates;

    public Several(Set<Predicate> predicates) {
	this.predicates = predicates;
    }

    public Several(Predicate pred1, Predicate pred2) {
	this.predicates = new HashSet<Predicate>();
	this.predicates.add(pred1);
	this.predicates.add(pred2);
    }

    @Override
    public Information clone() {
	return new Several(new HashSet<Predicate>(this.predicates));
    }
    
    @Override
    public String toString() {
	StringJoiner commaJoiner = new StringJoiner(", ");
	for(Predicate predicate : this.predicates) {
	    commaJoiner.add(predicate.toString());
	}
	return "Several(" + commaJoiner.toString() + ")";
    }

    @Override
    public String toString(SymbolicInteger symint) {
	return symint.hashCode() + " -> " + this.toString();
    }

    // @Override
    // public Set<String> toStrings(SymbolicInteger symint) {
    // 	Set<String> repr = new HashSet<String>();
    // 	for (Predicate predicate : this.predicates) {
    // 	    repr.addAll(predicate.toStrings(symint));
    // 	}
    // 	return repr;
    // }
    
    @Override
    public Information unify(Information other, Node node, boolean areSeparated) throws UnsatException {
	if (other == null) {
	    return this;
	}
	else if (other.isNil() || other.isRecord()) {
	    Information current = other;
	    for (Predicate predicate : this.predicates) {
		current = predicate.unify(current, node, areSeparated);
	    }
	    return current;
	}
	else if (other.isPredicate()) {
	    this.predicates.add((Predicate) other);
	    return this;
	}
	else {
	    throw new UnsoundException();
	}
    }
}
