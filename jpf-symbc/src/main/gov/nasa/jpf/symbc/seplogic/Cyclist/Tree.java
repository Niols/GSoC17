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

package gov.nasa.jpf.symbc.seplogic.Cyclist;

import java.util.Set;
import java.util.HashSet;

/* SPF+SL imports */
import gov.nasa.jpf.symbc.seplogic.SeplogicExpression;
import gov.nasa.jpf.symbc.seplogic.SeplogicVariable;

public class Tree extends gov.nasa.jpf.symbc.seplogic.Tree implements SeplogicExpression, CyclistConvertible {

    public Tree(SeplogicVariable variable, Set<String> labels) {
	super(variable, labels);
    }
    
    public String toCyclistString() {
	return getPredicate().uniqueName() + "(" + ((CyclistConvertible) getVariable()).toCyclistString() + ")";
    }

    public Set<String> cyclistPredicateDefinitions() {
	String name = getPredicate().uniqueName();

	//FIXME: wont work if there are other fields. we have to be
	//clever to discover them. but we also have to know how to
	//write that in Cyclist.
	
	String repr = name + " {\n  p=nil => " + name + "(p)\n}"; //FIXME: incomplete, lacks the most important rule

	Set<String> s = new HashSet<String>();
	s.add(repr);

	return s;
    }
}
