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

package gov.nasa.jpf.symbc.heap.seplogic.constraint;

import java.util.Set;
import java.util.Map;
import java.util.HashMap;

public class Tree extends Predicate
{
    private final Set<String> fields;
    
    public Tree(Set<String> fields) {
	this.fields = fields;
    }

    @Override
    public Information unifyWith(Information other, Node node, boolean areSeparated) throws UnsatException {

	if (other.isNil()) {
	    return other;
	}
	else if (other.isRecord()) {
	    if (areSeparated) {
		throw new UnsatException();
	    }
	    else {
		Constraint constraint = node.getConstraint();
		Map<String,Node> recordFields = new HashMap<String,Node>();
		for (String field : fields) {
		    recordFields.put(field, node.getConstraint().newNode());
		}
		return (new Record(recordFields)).unifyWith(other, node, areSeparated);
	    }
	}
	else if (other.arePredicates()) {
	    throw new UnsoundException("This case should not happen, because 'Predicate.unifyWith' should only be called from 'Information.unifyWith' that makes sure that this case does not happen.");
	}
	else {
	    throw new UnsoundException("This case should not happen. It means that the 'Information' interface is implemented by something I have not seen coming.");
	}
    }

    @Override
    public String toString() {
	return "Tree(...)";
    }

    @Override
    public String toString(Node node) {
	return node.toString() + "->" + this.toString();
    }
}
