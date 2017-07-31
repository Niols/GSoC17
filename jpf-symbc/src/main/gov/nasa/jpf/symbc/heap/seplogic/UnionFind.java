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


public class UnionFind<T>
{
    public interface Mergeable {
	public void merge(Mergeable other);
    }

    private class Node<T>
    {
	private T content;
	private Mergeable attr;
	private Node<T> father;
	private int rank;
	
	public Node(T content, Node<T> father, int rank, Mergeable attr) {
	    this.content = content;
	    this.father = father;
	    this.rank = rank;
	    this.attr = attr;
	}

	public Node(T content) {
	    this(content, null, 0, null);
	}
	
	/* Getters and setters for 'content' */

	public T getContent() {
	    return this.content;
	}

	/* Getters and setters for 'attr' */

	public Mergeable getAttr() {
	    return attr;
	}

	public void setAttr(Mergeable attr) {
	    this.attr = attr;
	}
	
	private void mergeAttr(Mergeable otherAttr) {
	    this.attr = this.attr.merge(otherAttr);
	}
	
	/* Getters and setters for 'father' */
	
	public boolean isAncestor() {
	    return (this.father == null);
	}

	private void setFather(Node<T> father) {
	    this.father = father;
	}

	/* Getters and setters for 'rank' */
		
	public int getRank() {
	    return this.rank;
	}
	
	private void addRank(int rank) {
	    this.rank += rank;
	}

	/* Union-Find structure */
	
	public Node<T> find() {
	    if (this.isAncestor()) {
		return this;
	    } else {
		Node<T> ancestor = this.father.find();
		this.father = ancestor;
		return ancestor;
	    }
	}

	public void union(Node<T> other) {
	    if (! this.isAncestor()) {
		return find().union(other);
	    } else {
		Node<T> otherAncestor = other.find();
		if (this.getRank() > otherAncestor.getRank()) {
		    this.addRank(otherAncestor.getRank());
		    this.mergeAttr(otherAncestor.getAttr());
		    return otherAncestor.union(this);
		} else {
		    this.setFather(otherAncestor);
		}
	    }
	}
    }

    /* */

    private Map<T,Node<T>> nodes;

    public UnionFind() {
	this.nodes = new HashMap<T,Node<T>>();
    }
    
    public UnionFind(Set<Map.Entry<T,Node<T>>> entries) {
	this.nodes = new HashMap<T,Node<T>>();
	
	for(Map.Entry<T,Node<T>> entry : entries) {
	    nodes.put(entry.getKey(), entry.getValue());
	}
    }
    
    public UnionFind(UnionFind<T> other) {
	this(other.entrySet());
    }
    
    private Node<T> getNode(T content) {
    }
}
