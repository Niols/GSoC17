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
import java.util.Map;
import java.util.Set;
import java.util.StringJoiner;

/* SPF imports */
import gov.nasa.jpf.symbc.numeric.SymbolicInteger;

public class Record implements Information
{
    private Map<String,Node> fields;

    public Record(Map<String,Node> fields) {
	this.fields = fields;
    }

    public boolean setField(String field, Node content) {
	if (fields.containsKey(field)) {
	    fields.put(field, content);
	    return true;
	} else {
	    return false;
	}
    }

    public Node getField(String field) {
	return fields.get(field);
    }
	
    public Set<String> getKeys() {
	return fields.keySet();
    }
	
    @Override
    public String toString() {
	StringJoiner commaJoiner = new StringJoiner(" , ");

	for (Map.Entry<String,Node> entry : fields.entrySet())
	    commaJoiner.add(entry.getKey() + " = " + entry.getValue().getVariable().hashCode());

	return "-> {| " + commaJoiner.toString() + " |}";
    }

    public Information unify(Information other, boolean unifyRecordsWithPredicates) throws UnsatException {
	if (other == null) {
	    return this;
	} else if (other instanceof Nil || other instanceof Record) {
	    throw new UnsatException();
	} else {
	    Predicate otherAsPredicate = (Predicate) other;
	    return otherAsPredicate.unifyPredicate(this, unifyRecordsWithPredicates);
	}
    }
}
