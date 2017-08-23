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
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.StringJoiner;

/* SPF imports */
import gov.nasa.jpf.symbc.numeric.SymbolicInteger;

/**
 * This class represents a constraint on the heap.  This is heavily
 * inspired from separation logic, but represents separation logic
 * constraints in an efficient way:
 *
 * This is basically a Union-Find structure, which makes the check for
 * aliases very efficient. There are quite a few modifications,
 * though, as each equivalence class representant carries some more
 * information with it:
 *
 * - it carries a set of forbidden other nodes, that is nodes that we
 *   know cannot be equal to it.  One can only add nodes to this set.
 *   During the union phase, we check that the forbidden sets are
 *   consistent.
 *
 * - it carries an 'Information' that can be either empty, the
 *   knowledge the the equivalence class is equal to Nil, the
 *   knowledge that it is different from Nil, the knowledge that it
 *   points to a record of other nodes, or the knowledge that it
 *   respects a predicate.  There can only be one information, which
 *   means that each time one adds information, or each time we
 *   proceed to the union of two equivalence classes, we have to unify
 *   these informations.  This may involve the unfolding of a
 *   predicate.
 *
 * A lot of methods of this class may raise the UnsatException,
 * meaning that the update the user just tried to perform just made it
 * unsatisfiable. This is possible to check that on-the-fly because of
 * how the structure is made, with the set of aliases and anti-aliases
 * easely accessible, and only one information per equivalence class.
 */

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
	this.nodes = nodes;
    }

    public Node getNode(SymbolicInteger e) {
	Node node = nodes.get(e);
	if (node == null) {
	    node = new Node(this, e);
	    nodes.put(e, node);
	}
	return node;
    }

    public Node freshNode() {
	return this.getNode(new SymbolicInteger());
    }

    /** Returns a deep copy of the whole constraint constraint. Does
     * not copy the SymbolicIntegers containened in the nodes. */
    public Constraint clone() {
	Constraint newConstraint = new Constraint();
	Map<SymbolicInteger,Node> clonedNodes = new HashMap<SymbolicInteger,Node>();

	for(Node node : nodes.values()) {
	    /* It is crucial that we pass the same map to all the
	     * nodes as they use it to deduce the global information
	     * that they need, such as their father and the forbidden
	     * nodes. To do so, they have to share a map together,
	     * that they will update with themselves when they're done
	     * cloning. */
	    node.clone(newConstraint, clonedNodes);
	}
	newConstraint.setNodes(clonedNodes);
	return newConstraint;
    }

    /* Constraint adders. They can all throw UnsatException. */

    public void addEq(SymbolicInteger e, SymbolicInteger f) throws UnsatException {
	getNode(e).union(getNode(f));
    }

    public void addNeq(SymbolicInteger e, SymbolicInteger f) throws UnsatException {
	getNode(e).addDistinctNode(getNode(f));
    }

    public void addNil(SymbolicInteger e) throws UnsatException {
	getNode(e).find().addInformation(new Nil());
    }

    public void addNonNil(SymbolicInteger e) throws UnsatException {
	getNode(e).find().addInformation(new Record());
    }

    public void addRecord(SymbolicInteger e, Map<String,SymbolicInteger> fields) throws UnsatException {

	Map<String,Node> fieldsWithNodes = new HashMap<String,Node>();

	for (Map.Entry<String,SymbolicInteger> field : fields.entrySet())
	    fieldsWithNodes.put(field.getKey(), getNode(field.getValue()));

	getNode(e).find().addInformation(new Record(fieldsWithNodes));
    }

    public void updateField(SymbolicInteger variable, String field, SymbolicInteger content) throws UnsatException {
	getNode(variable).find().updateField(field, getNode(content));
    }

    public void addPredicate(SymbolicInteger variable, Predicate predicate) throws UnsatException {
	getNode(variable).find().addInformation(predicate);
    }

    /* Printer */

    public String toString() {
	StringJoiner starJoiner = new StringJoiner(" * ");

	for (Node node : nodes.values())
	    for (String repr : node.toStrings())
		starJoiner.add(repr);

	return starJoiner.toString();
    }
}
