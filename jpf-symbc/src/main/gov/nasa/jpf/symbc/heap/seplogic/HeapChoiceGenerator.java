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
// Copyright (C) 2007 United States Government as represented by the
// Administrator of the National Aeronautics and Space Administration
// (NASA).  All Rights Reserved.
//
// This software is distributed under the NASA Open Source Agreement
// (NOSA), version 1.3.  The NOSA has been approved by the Open Source
// Initiative.  See the file NOSA-1.3-JPF at the top of the distribution
// directory tree for the complete NOSA document.
//
// THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF ANY
// KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT
// LIMITED TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO
// SPECIFICATIONS, ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
// A PARTICULAR PURPOSE, OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT
// THE SUBJECT SOFTWARE WILL BE ERROR FREE, OR ANY WARRANTY THAT
// DOCUMENTATION, IF PROVIDED, WILL CONFORM TO THE SUBJECT SOFTWARE.
//

package gov.nasa.jpf.symbc.heap.seplogic;

/* JPF imports */
import gov.nasa.jpf.vm.choice.IntIntervalGenerator;

/* SPF imports */
import gov.nasa.jpf.symbc.heap.SymbolicInputHeap;

public class HeapChoiceGenerator extends IntIntervalGenerator {
    /* We use the IntIntervalGenerator to generate array indices. */

    /* We maintain "pairs" of symbolic input heaps and path
     * conditions. */
    
    protected PathCondition[] PCs;
    protected SymbolicInputHeap[] symInputHeap;
        
    @SuppressWarnings("deprecation")
    public HeapChoiceGenerator(int size) {
	/* Initialize the IntIntervalGenerator to generate indices for
	 * our array. */
	super(0, size-1);

	this.PCs = new PathCondition[size];
	this.symInputHeap = new SymbolicInputHeap[size];
    }

    public void setCurrentPC(PathCondition PC) {
	PCs[getNextChoice()] = PC;
    }

    public PathCondition getCurrentPC() {
	PathCondition PC = PCs[getNextChoice()];
	return (PC == null) ? null : PC.clone();
    }

    public void setCurrentSymInputHeap(SymbolicInputHeap ih) {
	symInputHeap[getNextChoice()] = ih;
    }

    public SymbolicInputHeap getCurrentSymInputHeap() {
	SymbolicInputHeap ih = symInputHeap[getNextChoice()];
	return (ih == null) ? null : ih.make_copy();
    }
}
