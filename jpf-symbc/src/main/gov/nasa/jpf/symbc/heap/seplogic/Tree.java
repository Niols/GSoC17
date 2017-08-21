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
import java.util.HashSet;
import java.util.StringJoiner;

public class Tree implements Predicate
{
    private final Set<String> fields;
	
    public Tree(Set<String> fields) {
	this.fields = fields;
    }

    public Tree(String[] fieldsArray) {
	this.fields = new HashSet<String>();
	for (String field : fieldsArray) {
	    this.fields.add(field);
	}
    }
    
    @Override
    public String toString() {
	StringJoiner stringJoiner = new StringJoiner(", ");
	for (String field : fields)
	    stringJoiner.add(field);
	return "-> Tree(" + stringJoiner.toString() + ")";
    }

    public Information unify(Information other, boolean unifyRecordsWithPredicates) throws UnsatException {
	return unifyPredicate(other, unifyRecordsWithPredicates);
    }

    public Information unifyPredicate(Information other, boolean unifyRecordsWithPredicates) throws UnsatException {
	//System.out.println("Hello, I'm " + toString() + " and I'm trying to unify myself with " + other);
	
	if (other == null) {
	    return this;
	} else if (other instanceof Nil) {
	    return other;
	}
	else if (other instanceof Record) {
	    //System.out.println("This is a record!");
	    
	    Record record = (Record) other;

	    if (! unifyRecordsWithPredicates) {
		throw new UnsatException("Tried to unify Record " + record.toString() + " with Predicate " + toString() + " while unifyRecordsWithPredicates is false.");
	    }

	    //System.out.println("I have the right to unify myself with it.");
	    
	    if (! record.getKeys().containsAll(this.fields)) {
		throw new UnsatException("Tried to unify Record " + record.toString() + " with Predicate " + toString() + " when the predicate fields are not included in the records' ones.");
	    }

	    //System.out.println("Our keys are compatible");
	    
	    for (String field : fields) {
		record.getField(field).setInformation(this, unifyRecordsWithPredicates);
	    }

	    //System.out.println("I shall return the record: " + record);
	    
	    return record;
	}
	else {
	    System.out.println("I do not know how to handle two predicates on the same variable... so far.");
	    /* FIXME: that should not be too hard: we just keep all
	     * the predicates. And whenever we need to unfold, we
	     * unfold all of them. This might be a bit costly though,
	     * but there will hopefully never be more than 2-3
	     * predicates on a variable, and most cases will disapear
	     * by themselves. For harder cases, one should write a new
	     * predicate doing it efficiently. */

	    System.exit(1);
	    return null;
	}
    }
}
