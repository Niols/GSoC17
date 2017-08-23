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

/** This class represents a node in the Union-Find structure. */
public class Node
{
    private SymbolicInteger variable;
    private Set<Node> distinctNodes;
    private Information information;

    private Node father;
    private int rank;

    public final Constraint constraint;

    /** Carefull while using that, as it can break the structure's
     * consistency. */
    public Node(Constraint constraint, SymbolicInteger variable, Set<Node> distinctNodes, Information information, Node father, int rank) {
	this.constraint = constraint;
	this.variable = variable;
	this.distinctNodes = distinctNodes;
	this.information = information;
	this.father = father;
	this.rank = rank;
    }

    public Node(Constraint constraint, SymbolicInteger variable) {
	this(constraint, variable, new HashSet<Node>(), null, null, 0);
    }

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

    public Node find() {
	if (father == null) {
	    return this;
	} else {
	    Node ancestor = father.find();
	    this.father = ancestor;
	    return ancestor;
	}
    }


    /** The union function takes an other node and merge our
     * equivalence class with theirs. */

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

	/* We update the information */

	//FIXME: for now, they are separated by default. This has to
	//change.
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

    public boolean isAncestor() {
	return (this.father == null);
    }

    public int getRank() {
	return rank;
    }

    public SymbolicInteger getVariable() {
	return variable;
    }

    public Set<Node> getDistinctNodes() {
	return this.distinctNodes;
    }

    public Information getInformation() {
	return information;
    }

    /* ********** Setters ********** */

    public void addDistinctNode(Node other) {
	/* Propagate this to ancestor. */
	if (father != null) {
	    find().addDistinctNode(other);
	    return;
	}
	/* Add the other ancestor as a forbidden one for us. */
	this.distinctNodes.add(other.find());
    }

    public void addInformation(Information otherInformation) throws UnsatException {
	this.addInformation(otherInformation, false);
    }

    private void addInformation(Information otherInformation, boolean areSeparated) throws UnsatException {
	if (this.information == null) {
	    this.information = otherInformation;
	} else {
	    this.information = this.information.unify(otherInformation, this, areSeparated);
	}
    }

    //FIXME: doc
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
	    //FIXME: in place add? does the cloning clone informations too?
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
