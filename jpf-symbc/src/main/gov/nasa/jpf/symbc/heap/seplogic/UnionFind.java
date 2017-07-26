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

import java.util.Set;

/**
 * An interface for a Union-Find structure (a.k.a. a disjoint-set data
 * structure, or a merge-find set). 
 *
 * This is a data structure that keeps track of a set of elements
 * partitioned into a number of disjoint (non-overlapping) subsets. 
 */
public interface UnionFind<T> {
    
    class Entry<T> {
	private final T nonRepresentant;
	private final T representant;
	
	public Entry(T nonRepresentant, T representant) {
	    this.nonRepresentant = nonRepresentant;
	    this.representant = representant;
	}

	public T getNonRepresentant() {
	    return nonRepresentant;
	}

	public T getRepresentant() {
	    return representant;
	}
    }
    
    // /** Creates a fresh Union-Find structure. */
    // public UnionFind();

    // /** Creates the Union-Find structure that represents the same sets
    //  * as the given one. */
    // public UnionFind(UnionFind other);
    
    /** Returns the representent of the given element. */
    public T find(T e);

    /** Takes two elements and merges their equivalence classes. */
    public void union(T e, T f);
    
    /** Returns the set of all the representants. */
    public Set<T> getAllRepresentants();
    
    /** Returns the set of all the mappings. */
    public Set<Entry<T>> getAll();
}
