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
 * This class represents a node in the twicked Union-Find data
 * structure.  It is used to carry a variable (i.e. a SymbolicInteger)
 * and to store efficiently the set of nodes that are equal as well as
 * their informations (that is, if they are nil, a record, if they
 * have predicate talking about them...).
 *
 * This class is based on an efficiently written Union-Find structure,
 * allowing a very fast test of equality between variables.  This is
 * pretty important, as there are a lot of equal variables created by
 * the constraints of SPF.
 *
 * It also carries a set of 'distinctNodes', that is a set of nodes
 * that cannot be set equal to this one.  Whenever we call the 'union'
 * of two nodes, we check that they do not appear in the other's
 * 'distinctNode' set, allowing us to find clashes of disequality
 * right away.
 *
 * Finally, it carries an 'Information', telling us what we know about
 * the variable: is it equal to nil? to a record? do we have
 * predicates talking about it?  Whenever we call the 'union'
 * operation, their informations are merged.  This process detects
 * clashes due, for instance, to a violation of the separation of a
 * list.
 */
public class Node
{
    private SymbolicInteger variable;
    private Set<Node> distinctNodes;
    private Information information;

    private Node father;
    private int rank;

    public final Constraint constraint;

    /**
     * Build a node from scratch. Careful while using that, as it can
     * break the structure's consistency.
     */
    public Node(Constraint constraint, SymbolicInteger variable, Set<Node> distinctNodes, Information information, Node father, int rank) {
	this.constraint = constraint;
	this.variable = variable;
	this.distinctNodes = distinctNodes;
	this.information = information;
	this.father = father;
	this.rank = rank;
    }

    /**
     * Build a node with absolutely no information except for the
     * variable.
     */
    public Node(Constraint constraint, SymbolicInteger variable) {
	this(constraint, variable, new HashSet<Node>(), null, null, 0);
    }

    /**
     * Clone this node. This has to be done in a new environment: that
     * is a new constraint, and a group of nodes that have already
     * been cloned.  The 'clonedNodes' map is modified in * place.
     */
    public Node clone(Constraint newConstraint, Map<SymbolicInteger,Node> clonedNodes) {
	/* It is crucial that the same map is used by every node,
	 * as they will update it! */

	Node thisNode = clonedNodes.get(this.variable);

	/* Check if we haven't been cloned already, in which case
	 * we can just return the cloned node. */

	if (thisNode != null)
	    return thisNode;

	/* For now on, we have not been cloned. Let us do this! */

	Set<Node> clonedDistinctNodes;
	if (this.distinctNodes == null) {
	    clonedDistinctNodes = null;
	} else {
	    clonedDistinctNodes = new HashSet<Node>();
	    for (Node distinctNode : this.distinctNodes) {
		clonedDistinctNodes.add(distinctNode.clone(newConstraint, clonedNodes));
	    }
	}

	Node clonedFather;
	if (this.father == null) {
	    clonedFather = null;
	} else {
	    clonedFather = this.father.clone(newConstraint, clonedNodes);
	}

	Node clonedNode = new Node(newConstraint, this.variable, clonedDistinctNodes,
				   ((this.information == null) ? null : this.information.clone()),
				   clonedFather, this.rank);
	clonedNodes.put(this.variable, clonedNode);
	return clonedNode;
    }

    /* ********** Union-Find ********** */

    /**
     * The find operation of the Union-Find data structure: returns a
     * node that is the representant of the equivalence class of the
     * current node.
     */
    public Node find() {
	if (father == null) {
	    return this;
	} else {
	    Node ancestor = father.find();
	    this.father = ancestor;
	    return ancestor;
	}
    }


    /**
     * The union operation of the Union-Find structure: takes an other
     * node and merge the equivalence classes.
     */
    public void union(Node other) throws UnsatException {
	if (this.father != null) {
	    this.find().union(other);
	    return;
	}

	/* Now we know that we are the ancestor. */

	Node otherAncestor = other.find();

	if (this.equals(otherAncestor))
	    return;

	/* Now we know that we are distinct from the other
	 * ancestor: we have to merge. */

	if (this.getRank() < otherAncestor.getRank()) {
	    otherAncestor.union(this);
	    return;
	}

	/* Now we know that we have the biggest rank: it is our
	 * job to proceed to the merge. */

	/* We first check that the other equivalence class is not
	 * in the forbidden ones. */

	for (Node nodeDistinctFromOther : this.distinctNodes) {
	    Node ancestorDistinctFromOther = nodeDistinctFromOther.find();

	    if (otherAncestor.equals(ancestorDistinctFromOther)) {
		throw new UnsatException("Variable " + otherAncestor.getVariable() + " cannot be equal to " + ancestorDistinctFromOther);
	    }

	    if (! nodeDistinctFromOther.isAncestor()) {
		/* Keep the set of distinct nodes as small as possible
		 * by keeping only the ancestors. */
		this.distinctNodes.remove(nodeDistinctFromOther);
		this.distinctNodes.add(ancestorDistinctFromOther);
	    }
	}

	/* Now, we know that we accept the merge.  We thus merge
	 * the two forbidden check. While doing so, we check that
	 * the other ancestor accepts the merge too. */

	for (Node nodeDistinctFromThis : otherAncestor.getDistinctNodes()) {
	    Node ancestorDistinctFromThis = nodeDistinctFromThis.find();

	    if (this.equals(ancestorDistinctFromThis))
		throw new UnsatException("Variable " + this.getVariable() + " cannot be equal to " + ancestorDistinctFromThis);

	    this.distinctNodes.add(ancestorDistinctFromThis);
	}

	/* We update the information. The 'true' boolean means that
	 * the other node and this one are considered separated. */

	this.addInformation(otherAncestor.getInformation(), true);

	/* Finally, we tell to the other node that we are its
	 * father now, and we add its rank to ours. */

	this.rank += otherAncestor.getRank();
	otherAncestor.setFather(this, true);
    }

    public void setFather(Node father, boolean clearFields) {
	this.father = father;
	if (clearFields) {
	    this.distinctNodes.clear();
	    this.information = null;
	}
    }

    /* ********** Getters ********** */

    /**
     * Return true whether this node is the ancestor / the
     * representant of its equivalence class (property of the
     * Union-Find).
     */
    public boolean isAncestor() {
	return (this.father == null);
    }

    /**
     * Return the rank of this node (property of the Union-Find).
     */
    public int getRank() {
	return rank;
    }

    /**
     * Return the variable hold by this node.
     */
    public SymbolicInteger getVariable() {
	return variable;
    }

    /**
     * Return the set of nodes that cannot be set equal to this
     * one.
     */
    public Set<Node> getDistinctNodes() {
	return this.distinctNodes;
    }

    /**
     * Return the information carried by this node.
     */
    public Information getInformation() {
	return information;
    }

    /* ********** Setters ********** */

    /**
     * Add a node to the set of nodes that cannot be set equal to this
     * one.
     */
    public void addDistinctNode(Node other) {
	/* Propagate this to ancestor. */
	if (father != null) {
	    find().addDistinctNode(other);
	    return;
	}
	/* Add the other ancestor as a forbidden one for us. */
	this.distinctNodes.add(other.find());
    }

    /**
     * Add the given information to this node.
     */
    public void addInformation(Information otherInformation) throws UnsatException {
	this.addInformation(otherInformation, false);
    }

    /**
     * Add the given information to this node.  If this node already
     * possesses information, they have to be unified.  This process
     * might raise 'UnsatException', and needs an other information:
     * whether the node where the new information comes from is
     * separated from this one.
     */
    private void addInformation(Information otherInformation, boolean areSeparated) throws UnsatException {
	if (this.information == null) {
	    this.information = otherInformation;
	} else {
	    this.information = this.information.unify(otherInformation, this, areSeparated);
	}
    }

    /**
     * Update the constraint to reflect an update of the object
     * represented by this node.
     */
    public void updateField(String field, Node content) throws UnsatException {
	if (this.information == null) {
	    /* Whenever we have a PUTFIELD, the information cannot be
	     * empty, if the symbolic engine has done its job
	     * correctly. */

	    throw new UnsoundException();
	}
	else if (this.information.isNil()) {
	    throw new UnsatException();
	}
	else if (this.information.isRecord()) {
	    ((Record) this.information).setField(field, content);
	    return;
	}
	else {
	    throw new UnsoundException();
	}
    }

    public Set<String> toStrings() {
	Set<String> repr = new HashSet<String>();

	if (father == null) {
	    if (information != null)
		repr.add(information.toString(this.variable));

	    for (Node distinctNode : this.distinctNodes)
		repr.add(variable.hashCode() + " != " + distinctNode.getVariable().hashCode());
	} else {
	    repr.add(variable.hashCode() + " = " + father.getVariable().hashCode());
	}

	return repr;
    }
}
