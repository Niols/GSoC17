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

import gov.nasa.jpf.Config;
import gov.nasa.jpf.symbc.SymbolicInstructionFactory;
import gov.nasa.jpf.symbc.arrays.ArrayExpression;
import gov.nasa.jpf.symbc.heap.HeapNode;
import gov.nasa.jpf.symbc.heap.Helper;
import gov.nasa.jpf.symbc.numeric.Comparator;
import gov.nasa.jpf.symbc.numeric.IntegerConstant;
import gov.nasa.jpf.symbc.numeric.IntegerExpression;
import gov.nasa.jpf.symbc.numeric.SymbolicInteger;
import gov.nasa.jpf.symbc.string.StringExpression;
import gov.nasa.jpf.symbc.string.SymbolicStringBuilder;
import gov.nasa.jpf.vm.ChoiceGenerator;
import gov.nasa.jpf.vm.ClassInfo;
import gov.nasa.jpf.vm.ClassLoaderInfo;
import gov.nasa.jpf.vm.ElementInfo;
import gov.nasa.jpf.vm.Instruction;
import gov.nasa.jpf.vm.KernelState;
import gov.nasa.jpf.vm.MJIEnv;
import gov.nasa.jpf.vm.StackFrame;
import gov.nasa.jpf.vm.SystemState;
//import gov.nasa.jpf.symbc.uberlazy.TypeHierarchy;
import gov.nasa.jpf.vm.ThreadInfo;
import gov.nasa.jpf.symbc.heap.seplogic.HeapChoiceGenerator;
import gov.nasa.jpf.symbc.seplogic.HeapPathCondition;


public class ALOAD extends gov.nasa.jpf.jvm.bytecode.ALOAD {

    public ALOAD(int localVarIndex) {
	super(localVarIndex);
    }
	
    //private int numNewRefs = 0; // # of new reference objects to account for polymorphism -- work of Neha Rungta -- needs to be updated
    boolean abstractClass = false;

    @Override
    public Instruction execute (ThreadInfo th) {
	HeapNode[] prevSymRefs = null; // previously initialized objects of same type: candidates for lazy init
        int numSymRefs = 0; // # of prev. initialized objects
        ChoiceGenerator<?> prevHeapCG = null;

	Config conf = th.getVM().getConfig();
	String[] lazy = conf.getStringArray("symbolic.lazy");
	if (lazy == null || !lazy[0].equalsIgnoreCase("true"))
	    return super.execute(th);

	// TODO: fix handle polymorphism
		
	StackFrame sf = th.getModifiableTopFrame();
	int objRef = sf.peek();
	ElementInfo ei = th.getElementInfo(objRef);
	Object attr = sf.getLocalAttr(index);
	String typeOfLocalVar = super.getLocalVariableType();

	if(attr == null || typeOfLocalVar.equals("?") || attr instanceof SymbolicStringBuilder || attr instanceof StringExpression || attr instanceof ArrayExpression) {
	    return super.execute(th);
	}
		
	ClassInfo typeClassInfo = ClassLoaderInfo.getCurrentResolvedClassInfo(typeOfLocalVar);

	int currentChoice;
	ChoiceGenerator<?> thisHeapCG;

	// FIXME. Lots of stuff to change in here.
	
	if(!th.isFirstStepInsn()) {
	    //System.out.println("the first time");

	    prevSymRefs = null;
	    numSymRefs = 0;
	    prevHeapCG = null;

	    prevHeapCG = th.getVM().getLastChoiceGeneratorOfType(HeapChoiceGenerator.class);

	    int increment = 2;
	    if(typeClassInfo.isAbstract() || (((IntegerExpression)attr).toString()).contains("this")) {
		abstractClass = true;
		increment = 1; // only null for abstract, non null for this
	    }
			
	    // TODO fix: subtypes

	    thisHeapCG = new HeapChoiceGenerator(numSymRefs+increment);  //+null,new
			
	    th.getVM().setNextChoiceGenerator(thisHeapCG);
	    return this;
	} else { 
	    //this is what returns the results
	    thisHeapCG = th.getVM().getChoiceGenerator();
	    assert(thisHeapCG instanceof HeapChoiceGenerator) :
	    "expected HeapChoiceGenerator, got:" + thisHeapCG;
	    currentChoice = ((HeapChoiceGenerator) thisHeapCG).getNextChoice();
	}

	HeapPathCondition PC;

        prevHeapCG = thisHeapCG.getPreviousChoiceGeneratorOfType(HeapChoiceGenerator.class);

	if(prevHeapCG == null) {
	    PC = new HeapPathCondition();
	} else {
	    PC =  ((HeapChoiceGenerator) prevHeapCG).getCurrentPC();
	}

	assert PC != null;
		
	int daIndex = 0; //index into JPF's dynamic area

	sf.setLocalVariable(index, daIndex, true);
	sf.setLocalAttr(index, null);
	sf.push(daIndex, true);

	((HeapChoiceGenerator)thisHeapCG).setCurrentPC(PC);
	
	if (SymbolicInstructionFactory.debugMode) {
	    System.out.println("[seplogic] ALOAD PC: " + PC);
	}
	
	return getNext(th);
    }
}
