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

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/** This is a very easy implementation of the Union-Find
 * structure. However, its performances are not that good. */

public class HashMapUnionFind<T> implements UnionFind<T> {
    
    private final Map<T,T> fatherMap;

    public HashMapUnionFind() {
	fatherMap = new HashMap<T,T>();
    }

    public HashMapUnionFind(Set<Entry<T>> entries) {
	fatherMap = new HashMap<T,T>();
	
	for (Entry<T> entry : entries) {
	    fatherMap.put(entry.getNonRepresentant(),
			  entry.getRepresentant());
	}
    }
    
    public HashMapUnionFind(UnionFind other) {
	this(other.getAll());
    }

    public T find(T e) {
	T father = fatherMap.get(e);
	
	if (father == null) {
	    return e;
	} else {
	    T ancestor = find(father);
	    fatherMap.replace(e, ancestor);
	    return ancestor;
	}
    }

    public void union(T e, T f) {
	T ancestorE = find(e);
	T ancestorF = find(f);
	
	if (! ancestorE.equals(ancestorF)) {
	    fatherMap.put(ancestorE, ancestorF);
	}
    }

    public Set<T> getAllRepresentants() {
	Set<T> all = new HashSet<T>();
	
	for (T e : fatherMap.keySet()) {
	    all.add(find(e));
	}

	return all;
    }
    
    public Set<Entry<T>> getAll() {
	Set<Entry<T>> all = new HashSet<Entry<T>>();

	for (T e : fatherMap.keySet()) {
	    all.add(new Entry(e, find(e)));
	}
	return all;
    }
}
