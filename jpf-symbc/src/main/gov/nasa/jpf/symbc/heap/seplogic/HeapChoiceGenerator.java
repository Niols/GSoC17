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

import gov.nasa.jpf.symbc.heap.SymbolicInputHeap;
import gov.nasa.jpf.symbc.seplogic.HeapPathCondition;
import gov.nasa.jpf.vm.choice.IntIntervalGenerator;

public class HeapChoiceGenerator extends IntIntervalGenerator {
    /* We use the IntIntervalGenerator to generate array indices. */

    /* We maintain "pairs" of symbolic input heaps and path
     * conditions. */
    
    protected HeapPathCondition[] PC;
    protected SymbolicInputHeap[] symInputHeap;
        
    @SuppressWarnings("deprecation")
    public HeapChoiceGenerator(int size) {
	/* Initialize the IntIntervalGenerator to generate indices for
	 * our array. */
	super(0, size-1);

	this.PC = new HeapPathCondition[size];
	this.symInputHeap = new SymbolicInputHeap[size];
    }

    public void setCurrentPC(HeapPathCondition hpc) {
	PC[getNextChoice()] = hpc;
    }

    public HeapPathCondition getCurrentPC() {
	/* For now, it's OK. We might need to copy this PC before
	 * returning it. */
	return PC[getNextChoice()];
    }

    public void setCurrentSymInputHeap(SymbolicInputHeap ih) {
	symInputHeap[getNextChoice()] = ih;
    }

    public SymbolicInputHeap getCurrentSymInputHeap() {
	SymbolicInputHeap ih;

	ih = symInputHeap[getNextChoice()];
	if (ih != null) {
	    return ih.make_copy();
	} else {
	    return null;
	}
    }
}
