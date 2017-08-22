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

    /**
     * This function tries to come up with an information
     * consistent with the given and the current informations.
     *
     * The following table presents the cases where we know for
     * sure the information to deduce (filled in), the cases where
     * we know for sure that this is unsatisfiable (crossed out),
     * and the other cases that will require more subtle handling.
     *
     *            |         / |       Nil |    Record | Predicate  *
     * -----------|-----------|-----------|-----------|----------- *
     *          / |         / |       Nil |    Record | Predicate  *
     * -----------|-----------|-----------|-----------|----------- *
     *        Nil |       Nil |       Nil | \ \ \ \ \ |            *
     * -----------|-----------|-----------|-----------|----------- *
     *     Record |    Record | \ \ \ \ \ | \ \ \ \ \ |            *
     * -----------|-----------|-----------|-----------|----------- *
     *  Predicate | Predicate |           |           |            *
     *
     * Some of these cells might be surprising, as the one that
     * states that the result of two records in unsatisfiable.
     * This is because all the clauses represented by this
     * structure are separated (because of the separation logic).
     *
     * However, the informations about being Nil or not are pure.
     *
     * FIXME: the case of predicates is quite surprising
     */
    public abstract Information unify(Information other, boolean unifyRecordsWithPredicates) throws UnsatException;
}
