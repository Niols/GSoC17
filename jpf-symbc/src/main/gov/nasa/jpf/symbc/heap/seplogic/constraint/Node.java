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

/* Java imports */
import java.util.Set;
import java.util.Map;
import java.util.HashMap;
import java.util.HashSet;

/* SPF imports */
import gov.nasa.jpf.symbc.numeric.SymbolicInteger;

public class Node
{
    private Constraint constraint;

    private Node father;
    private int rank;

    private SymbolicInteger variable;
    private Information information;
    
    private Set<Node> unseparated;//FIXME: check propagation, I don't
				  //think this is happening. Actually,
				  //don't use this, this is probably
				  //more than unsound.
    private Set<Node> distinct;

    /* Constructor */

    public Node(Constraint constraint, Node father, int rank,
		SymbolicInteger variable, Information information,
		Set<Node> unseparated, Set<Node> distinct)
    {
	this.constraint = constraint;
	
	this.father = father;
	this.rank = rank;

	this.variable = variable;
	this.information = information;

	this.unseparated = unseparated;
	this.distinct = distinct;
    }
    
    public Node(Constraint constraint, SymbolicInteger variable) {
	this(constraint, null, 1, variable, null, new HashSet<Node>(), new HashSet<Node>());
    }

    public Node clone(Constraint newConstraint, Map<SymbolicInteger,Node> clonedNodes) {
	/* Warning: use it only if necessary as this takes a long
	 * time. */
	
	Node thisClonedNode = clonedNodes.get(this.getVariable());
	if (thisClonedNode == null) {
	    Set<Node> clonedUnseparated = new HashSet<Node>();
	    for (Node unseparatedNode : this.getUnseparated()) {
		clonedUnseparated.add(unseparatedNode.clone(newConstraint, clonedNodes));
	    }
	    Set<Node> clonedDistinct = new HashSet<Node>();
	    for (Node distinctNode : this.getDistinct()) {
		clonedDistinct.add(distinctNode.clone(newConstraint, clonedNodes));
	    }

	    Node thisFather = this.getFather();
	    
	    thisClonedNode = new Node(newConstraint,
				      ((thisFather == null) ? null : thisFather.clone(newConstraint, clonedNodes)),
				      this.getRank(), this.getVariable(), this.getInformation(),
				      clonedUnseparated, clonedDistinct);

	    clonedNodes.put(this.getVariable(), thisClonedNode);
	}
	
	return thisClonedNode;
    }
    
    /* Getters */

    public Node getFather() {
	return this.father;
    }

    private void setFather(Node father) {
	this.father = father;
    }

    public void acceptAsFather(Node father) {
	this.setFather(father);
	this.setInformation(null);
	this.setUnseparated(new HashSet<Node>());
	this.setDistinct(new HashSet<Node>());
    }
    
    public boolean isAncestor() {
	return (this.getFather() == null);
    }

    public int getRank() {
	return this.rank;
    }

    private void addRank(int rank) {
	this.rank += rank;
    }

    public SymbolicInteger getVariable() {
	return this.variable;
    }
    
    public Information getInformation() {
	return this.information;
    }

    private void setInformation(Information information) {
	this.information = information;
    }

    private void addInformation(Information otherInformation, boolean areSeparated) throws UnsatException {
	if (otherInformation == null) {
	    return;
	}
	else if (this.getInformation() == null) {
	    this.setInformation(otherInformation);
	    return;
	}
	else {
	    this.setInformation(this.getInformation().unifyWith(otherInformation, this, areSeparated));
	}
    }

    public void addInformation(Information information) throws UnsatException {
	this.addInformation(information, false);
    }
    
    public Set<Node> getDistinct() {
	return this.distinct;
    }

    private void setDistinct(Set<Node> distinct) {
	this.distinct = distinct;
    }
    
    private void addDistinct(Set<Node> distinct) {
	this.distinct.addAll(distinct);
    }

    public Set<Node> getUnseparated() {
	return this.unseparated;
    }

    private void setUnseparated(Set<Node> unseparated) {
	this.unseparated = unseparated;
    }
    
    private void addUnseparated(Set<Node> unseparated) {
	this.unseparated.addAll(unseparated);
    }

    public Constraint getConstraint() {
	return this.constraint;
    }

    /* Union-Find */

    public Node find() {
	if (this.isAncestor()) {
	    return this;
	}

	Node ancestor = this.getFather().find();
	this.setFather(ancestor);
	return ancestor;
    }

    public void union(Node other) throws UnsatException {

	/* Report the responsibility on our ancestor. */

	if (! this.isAncestor()) {
	    this.find().union(other);
	    return;
	}

	/* Take the other node's ancestor. */

	Node otherAncestor = other.find();

	/* If the ancestors are the same, there is nothing to do. */

	if (otherAncestor == this) {
	    return;
	}

	/* Compare the ranks. The bigger gets to handle the unification. */

	if (this.getRank() < otherAncestor.getRank()) {
	    otherAncestor.union(this);
	    return;
	}

	/* We now are the ancestor of bigger rank. The other ancestor
	 * is accessible in the variable 'otherAncestor'. */

	/* Check that we are not in the other's distinct nodes, and
	 * that the other is not in our distinct nodes. */

	Set<Node> newDistinctNodes = new HashSet<Node>();
	
	for (Node distinctNode : this.getDistinct()) {
	    newDistinctNodes.add(distinctNode.find());
	}
	for (Node distinctNode : otherAncestor.getDistinct()) {
	    newDistinctNodes.add(distinctNode.find());
	}

	if (newDistinctNodes.contains(this) || newDistinctNodes.contains(otherAncestor)) {
	    throw new UnsatException();
	}

	this.setDistinct(newDistinctNodes);
	
	/* Determine whether these two nodes are separated. */

	boolean areSeparated = true;

	for (Node unseparatedNode : this.getUnseparated()) {
	    if (unseparatedNode.find() == otherAncestor) {
		areSeparated = false;
	    }
	}
	for (Node unseparatedNode : otherAncestor.getUnseparated()) {
	    if (unseparatedNode.find() == this) {
		areSeparated = false;
	    }
	}

	/* Merge their informations. */

	this.addInformation(otherAncestor.getInformation());
	otherAncestor.acceptAsFather(this); //FIXME
    }

    /* */

    public void updateField(String field, Node node) throws UnsatException {

	if (this.getInformation() == null) {
	    Map<String,Node> fields = new HashMap<String,Node>();
	    fields.put(field, node);
	    this.setInformation(new Record(fields));
	}
	else if (this.getInformation().isNil()) {
	    throw new UnsatException();
	}
	else if (this.information.isRecord()) {
	    Record thisRecord = (Record) this.getInformation();
	    thisRecord.updateField(field, node);
	}
	else {
	    throw new UnsoundException("This case should not happen.");
	}
    }

    /* */

    public String toString() {
	return "" + this.getVariable().hashCode();
    }
    
    public Set<String> toStrings() {
	Set<String> output = new HashSet<String>();
	if (! this.isAncestor()) {
	    output.add(this.toString() + "=" + this.find().toString());
	}
	if (this.getInformation() != null) {
	    output.addAll(this.getInformation().toStrings(this));
	}
	for (Node distinctNode : this.getDistinct()) {
	    output.add(this.toString() + "!=" + distinctNode.toString());
	}
	for (Node unseparatedNode : this.getUnseparated()) {
	    output.add(this.toString() + "~" + unseparatedNode.toString());
	}
	if (output.isEmpty()) {
	    output.add(this.toString());
	}
	return output;
    }
}
