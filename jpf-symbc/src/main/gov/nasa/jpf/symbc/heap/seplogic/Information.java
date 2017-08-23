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
import java.util.HashSet;

/* SPF imports */
import gov.nasa.jpf.symbc.numeric.SymbolicInteger;

public abstract class Information
{
    public boolean isNil() {
	return false;
    }

    public boolean isRecord() {
	return false;
    }

    public boolean isPredicate() {
	return false;
    }

    public Information clone() {
	return this;
    }

    public abstract String toString(SymbolicInteger symint);

    public Set<String> toStrings(SymbolicInteger symint) {
	Set<String> repr = new HashSet<String>();
	repr.add(this.toString(symint));
	return repr;
    }

    /**
     * This function tries to come up with the smallest information
     * that is greater than the current one and the given one.
     *
     * It takes as argument an other information, the node that will
     * receive the new information, and a boolean that is true whether
     * this node is separated from the other one.
     */
    public abstract Information unify(Information other, Node node, boolean areSeparated) throws UnsatException;
}
