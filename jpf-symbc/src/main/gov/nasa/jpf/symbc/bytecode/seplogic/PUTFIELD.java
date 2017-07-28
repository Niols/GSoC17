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

package gov.nasa.jpf.symbc.bytecode.seplogic;

/* JPF imports */
import gov.nasa.jpf.Config;
import gov.nasa.jpf.vm.ChoiceGenerator;
import gov.nasa.jpf.vm.ElementInfo;
import gov.nasa.jpf.vm.FieldInfo;
import gov.nasa.jpf.vm.Instruction;
import gov.nasa.jpf.vm.MJIEnv;
import gov.nasa.jpf.vm.StackFrame;
import gov.nasa.jpf.vm.ThreadInfo;

/* SPF imports */
import gov.nasa.jpf.symbc.SymbolicInstructionFactory;
import gov.nasa.jpf.symbc.heap.HeapNode;
import gov.nasa.jpf.symbc.heap.SymbolicInputHeap;
import gov.nasa.jpf.symbc.numeric.SymbolicInteger;

/* SPF+SL imports */
import gov.nasa.jpf.symbc.heap.seplogic.HeapChoiceGenerator;
import gov.nasa.jpf.symbc.heap.seplogic.PathCondition;
import gov.nasa.jpf.symbc.seplogic.SL;

public class PUTFIELD extends gov.nasa.jpf.jvm.bytecode.PUTFIELD {

    public PUTFIELD(String fieldName, String clsDescriptor, String fieldDescriptor) {
	super(fieldName, clsDescriptor, fieldDescriptor);
    }

    @Override
    public Instruction execute (ThreadInfo ti) {

	Config conf = ti.getVM().getConfig();
	if (! conf.getBoolean("symbolic.seplogic", false)) {
	    return super.execute(ti);
	}

	StackFrame frame = ti.getModifiableTopFrame();
	int objRef = frame.peek(size);
	lastThis = objRef;

	if (objRef == MJIEnv.NULL) {
	    return ti.createAndThrowException("java.lang.NullPointerException", "referencing field '" + fname + "' on null object");
	}

	ElementInfo ei = ti.getModifiableElementInfo(objRef);
	FieldInfo fi = getFieldInfo();
	if (fi == null) {
	    return ti.createAndThrowException("java.lang.NoSuchFieldError", "no field " + fname + " in " + ei);
	}

	/* Perform the update */

	Object opAttr = frame.getOperandAttr();
	int opVal = frame.peek();

	ei.set1SlotField(fi, opVal);
	ei.setFieldAttr(fi, opAttr);

	lastValue = opVal;

	/* Get previous PC and symInputHeap */

	ChoiceGenerator<?> thisHeapCG = ti.getVM().getSystemState().getLastChoiceGeneratorOfType(HeapChoiceGenerator.class);

	if (thisHeapCG != null && thisHeapCG instanceof HeapChoiceGenerator) {
	    PathCondition PC = ((HeapChoiceGenerator) thisHeapCG).getCurrentPC();
	    SymbolicInputHeap symInputHeap = ((HeapChoiceGenerator) thisHeapCG).getCurrentSymInputHeap();

	    assert PC != null;
	    assert symInputHeap != null;

	    SymbolicInteger objRefNode = symInputHeap.getNode(objRef);
	    SymbolicInteger  opValNode = symInputHeap.getNode(opVal);

	    /* Update constraint to add:
             *
             *     objRefNode . fname -> opValNode
             *
	     * To do so, we have to find a record definition of
	     * objRefNode. We can assume that there is an atom of the
	     * form:
             *
             *     objRefNode -> {| ... ; fname = ? |}
             *
             * FIXME: is that true? we have to take care about
             * aliases. */

	    /* Four cases:
	     *
             * - both objRef and opVal represent concrete nodes. In
             *   that case, I guess we can just ignore them: they do
             *   not have any impact on the PC.
             *
             * - both objRef and opVal represent symbolic nodes. In
             *   that case, we have to update the PC.
             *
             * - objRef is concrete and opVal is symbolic. In that
             *   case, we don't care?
	     *
	     * - objRef in symbolic and opVal is concrete. In that
	     *   case, we can add a fresh variable to fill in the
	     *   blanks. However, by doing so, we loose information!
	     *   We could at least try to detect when this value is
	     *   Nil, because this is what happens most of the
	     *   time. This will allow us to avoid loosing too much
	     *   information.
	     *
	     * FIXME: is that valid? */

	    // if (objRefNode != null && opValNode == null) {
	    // 	System.out.println("objRefNode is symbolic, opValNode is concrete, opVal=" + opVal + ", NULL=" + MJIEnv.NULL);
	    // }
	    
	    if (objRefNode != null) {
		/* If objRef is symbolic, get a representation for
		 * opVal (either opValNode is not null, or a fresh
		 * one), and update the field in the PC. */

		/* Since we are updating a field, we know that both
		 * our variables are of type int. */
		
		PC.updateField(objRefNode, fname, (opValNode != null) ? opValNode : new SymbolicInteger());
	    }

	    if (SL.debugMode)
		System.out.println("PUTFIELD: " + PC);
	    
	    if (PC.isUnsat()) {
		if (SL.debugMode)
		    System.out.println("PUTFIELD: PC is not satisfiable; ignoring state.");
		
		ti.getVM().getSystemState().setIgnored(true);
		return getNext(ti);
	    }
	    
	    ((HeapChoiceGenerator) thisHeapCG).setCurrentPC(PC);
	    ((HeapChoiceGenerator) thisHeapCG).setCurrentSymInputHeap(symInputHeap);

	}

	/* FIXME: handle shared objects, handle 'long' operands. */

	popOperands(frame);
	return getNext(ti);
    }
}
