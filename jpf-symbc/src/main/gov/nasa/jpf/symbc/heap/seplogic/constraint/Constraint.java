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

import java.util.Map;
import java.util.HashMap;
import java.util.StringJoiner;

import gov.nasa.jpf.symbc.numeric.SymbolicInteger;

public class Constraint
{
    private Map<SymbolicInteger,Node> nodes;

    public Constraint(Map<SymbolicInteger,Node> nodes) {
	this.nodes = nodes;
    }

    public Constraint() {
	this(new HashMap<SymbolicInteger,Node>());
    }

    public void setNodes(Map<SymbolicInteger,Node> nodes) {
	if (this.nodes.isEmpty()) {
	    this.nodes = nodes;
	}
	//FIXME: else
    }
    

    public Node getNode(SymbolicInteger variable) {
	Node node = nodes.get(variable);

	if (node == null) {
	    node = new Node(this, variable);
	    nodes.put(variable, node);
	}

	return node;
    }

    public Node newNode() {
	return this.getNode(new SymbolicInteger());
    }

    /**
     * Use only if necessary as it takes a very long time.
     */
    public Constraint clone() {
	Constraint newConstraint = new Constraint();
	Map<SymbolicInteger,Node> newNodes = new HashMap<SymbolicInteger,Node>();
	
	for (Map.Entry<SymbolicInteger,Node> entry : this.nodes.entrySet()) {
	    entry.getValue().clone(newConstraint, newNodes);
	}

	newConstraint.setNodes(newNodes);
	return newConstraint;
    }
    
    public void addEq(SymbolicInteger e, SymbolicInteger f) throws UnsatException {
	getNode(e).union(getNode(f));
    }

    public void addNil(SymbolicInteger variable) throws UnsatException {
	getNode(variable).find().addInformation(new Nil());
    }
    
    public void addRecord(SymbolicInteger variable, Map<String,SymbolicInteger> fields) throws UnsatException {
	Map<String,Node> fieldsWithNodes = new HashMap<String,Node>();
	for (Map.Entry<String,SymbolicInteger> field : fields.entrySet())
	    fieldsWithNodes.put(field.getKey(), getNode(field.getValue()));
	
	getNode(variable).find().addInformation(new Record(fieldsWithNodes));
    }

    public void updateField(SymbolicInteger variable, String field, SymbolicInteger content) throws UnsatException {
	getNode(variable).find().updateField(field, getNode(content));
    }

    public void addPredicate(SymbolicInteger variable, Predicate predicate) throws UnsatException {
	getNode(variable).find().addInformation(new Predicates(predicate));
    }

    /* Printer */

    public String toString() {
	StringJoiner starJoiner = new StringJoiner(" * ");
	for (Node node : nodes.values()) {
	    for (String repr : node.toStrings()) {
		starJoiner.add(repr);
	    }
	}
	return starJoiner.toString();
    }

}
