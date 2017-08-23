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
import java.util.HashMap;
import java.util.Set;
import java.util.StringJoiner;

/* SPF imports */
import gov.nasa.jpf.symbc.numeric.SymbolicInteger;

public class Record extends Information
{
    private Map<String,Node> fields;

    public Record(Map<String,Node> fields) {
	this.fields = fields;
    }

    /** Create a record with no information in it. */
    public Record() {
	this(new HashMap<String,Node>());
    }
    
    @Override
    public boolean isRecord() {
	return true;
    }

    public void setField(String field, Node content) {
	this.fields.put(field, content);
    }

    public Map<String,Node> getFields() {
	return this.fields;
    }

    @Override
    public Record clone() {
	return new Record(new HashMap<String,Node>(this.fields));
    }
    
    @Override
    public String toString() {
	StringJoiner commaJoiner = new StringJoiner(" , ");
	
	for (Map.Entry<String,Node> entry : fields.entrySet())
	    commaJoiner.add(entry.getKey() + " = " + entry.getValue().getVariable().hashCode());
	
	return "{| " + commaJoiner.toString() + " |}";
    }

    @Override
    public String toString(SymbolicInteger symint) {
	return symint.hashCode() + " -> " + this.toString();
    }
    
    @Override
    public Information unify(Information other, Node node, boolean areSeparated) throws UnsatException {
	if (other == null) {
	    return this;
	} else if (other.isNil()) {
	    throw new UnsatException("Information: trying to unify a record with nil");
	} else if (other.isRecord()) {
	    if (areSeparated) {
		throw new UnsatException("Information: trying to unify two records while separated");
	    } else {
		/* We merge the two records: we take all the fields of
		 * this one and we add all the others.  In case of
		 * clash, we add the equality to the constraint. */
		
		for (Map.Entry<String,Node> otherEntry : ((Record) other).getFields().entrySet()) {
		    Node thisNode = this.fields.get(otherEntry.getKey());
		    if (thisNode == null) {
			this.fields.put(otherEntry.getKey(), otherEntry.getValue());
		    } else {
			thisNode.union(otherEntry.getValue());
		    }
		}
		return this;
	    }
	} else if (other.isPredicate()) {
	    return other.unify(this, node, areSeparated);
	} else {
	    throw new UnsoundException();
	}
    }
}
