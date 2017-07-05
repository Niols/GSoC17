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
import gov.nasa.jpf.vm.ClassInfo;
import gov.nasa.jpf.vm.ClassLoaderInfo;
import gov.nasa.jpf.vm.ElementInfo;
import gov.nasa.jpf.vm.Instruction;
import gov.nasa.jpf.vm.KernelState;
import gov.nasa.jpf.vm.MJIEnv;
import gov.nasa.jpf.vm.StackFrame;
import gov.nasa.jpf.vm.SystemState;
import gov.nasa.jpf.vm.ThreadInfo;

/* SPF imports */
import gov.nasa.jpf.symbc.SymbolicInstructionFactory;
import gov.nasa.jpf.symbc.arrays.ArrayExpression;
import gov.nasa.jpf.symbc.numeric.Comparator;
import gov.nasa.jpf.symbc.numeric.IntegerConstant;
import gov.nasa.jpf.symbc.numeric.IntegerExpression;
import gov.nasa.jpf.symbc.numeric.SymbolicInteger;
import gov.nasa.jpf.symbc.string.StringExpression;
import gov.nasa.jpf.symbc.string.SymbolicStringBuilder;
import gov.nasa.jpf.symbc.heap.HeapNode;
import gov.nasa.jpf.symbc.heap.SymbolicInputHeap;

/* JPF+SL imports */
import gov.nasa.jpf.symbc.heap.seplogic.HeapChoiceGenerator;
import gov.nasa.jpf.symbc.heap.seplogic.Helper;
import gov.nasa.jpf.symbc.heap.seplogic.PathCondition;
import gov.nasa.jpf.symbc.seplogic.SL;

public class ALOAD extends gov.nasa.jpf.symbc.bytecode.ALOAD {

    public ALOAD (int localVarIndex) {
	super(localVarIndex);
    }

    boolean abstractClass = false;

    @Override
    public Instruction execute (ThreadInfo th) {

	/* We first read the configuration to check that
	 * symbolic.seplogic is set to true. If not, we use the usual
	 * lazy (or not) version. */

	Config conf = th.getVM().getConfig();
	if (! conf.getBoolean("symbolic.seplogic", false)) {
	    return super.execute(th);
	}

	/* We also don't want to handle a few other cases, we send
	 * them to super. */

	StackFrame sf = th.getModifiableTopFrame();
	int objRef = sf.peek();
	ElementInfo ei = th.getElementInfo(objRef);
	Object attr = sf.getLocalAttr(index);
	String typeOfLocalVar = super.getLocalVariableType();

	if(attr == null
	   || typeOfLocalVar.equals("?")
	   || attr instanceof SymbolicStringBuilder
	   || attr instanceof StringExpression
	   || attr instanceof ArrayExpression) {

	    return super.execute(th);
	}

	/* Now, let's get to the real stuff. */

	/* We get infos about the type of the object we are
	 * manipulating. */

	ClassInfo typeClassInfo = ClassLoaderInfo.getCurrentResolvedClassInfo(typeOfLocalVar);

	if(!th.isFirstStepInsn()) {
	    /* If we are called for the first time */

	    /* We find the previous ChoiceGenerator that is a
	     * HeapChoiceGenerator. This is important, because this
	     * HeapChoiceGenerator will contain a SymbolicHeap in
	     * which we will find the objects that have the type we
	     * are interested in -- canditates to lazy
	     * initialization. We count the number of candidates. */

	    int numSymRefs = 0;

	    ChoiceGenerator<?> prevHeapCG = th.getVM().getLastChoiceGeneratorOfType(HeapChoiceGenerator.class);

	    if (prevHeapCG != null) {
		SymbolicInputHeap symInputHeap = ((HeapChoiceGenerator) prevHeapCG).getCurrentSymInputHeap();

		HeapNode[] prevSymRefs = symInputHeap.getNodesOfType(typeClassInfo);
		numSymRefs = prevSymRefs.length;
	    }

	    /* We count the number of candidates to lazy
	     * initialization that we will have to add. The two
	     * candidates are NULL or a fresh object. In the case of
	     * an abstract class, this can only be NULL. In the case
	     * of a THIS object, this cannot. */

	    int increment = 2;
	    if(typeClassInfo.isAbstract()
	       || (((IntegerExpression)attr).toString()).contains("this")) {
		this.abstractClass = true;
		increment = 1;
	    }

	    /* We can now create a new HeapChoiceGenerator that will
	     * only store this number (the number of previously
	     * existing objects + the increment we just defined). We
	     * store this HeapChoiceGenerator as the next
	     * ChoiceGenerator and we return. */

	    ChoiceGenerator<?> thisHeapCG = new HeapChoiceGenerator(numSymRefs+increment);

	    th.getVM().setNextChoiceGenerator(thisHeapCG);
	    return this;
	}

	/* If this is not the first call; this is where we return results. */

	/* Let's first get our ChoiceGenerator. This must be a
	 * HeapChoiceGenerator. Otherwise, something went horribly
	 * wrong (OK, maybe not 'horribly'. 'Slightly'? */

	ChoiceGenerator<?> thisHeapCG = th.getVM().getChoiceGenerator();
	assert(thisHeapCG instanceof HeapChoiceGenerator)
	    : "expected HeapChoiceGenerator, got:" + thisHeapCG;

	/* We then get the current choice. This is an integer: the
	 * indice of the previously initialized object of the same
	 * type, or, if this indice is out of bound, NULL or a fresh
	 * object. */

	int currentChoice = ((HeapChoiceGenerator) thisHeapCG).getNextChoice();

	/* We initialize our HeapPathCondition. To do that, we take a
	 * look at a previously initiliazed HeapChoiceGenerator: if
	 * there is one, we take its PC. If not, we create a new
	 * one. Idem for our SymbolicInputHeap. */

	PathCondition PC;
	SymbolicInputHeap symInputHeap;

        ChoiceGenerator<?> prevHeapCG = thisHeapCG.getPreviousChoiceGeneratorOfType(HeapChoiceGenerator.class);
	if(prevHeapCG == null) {
	    PC = new PathCondition();
	    symInputHeap = new SymbolicInputHeap();
	} else {
	    PC =  ((HeapChoiceGenerator) prevHeapCG).getCurrentPC();
	    symInputHeap = ((HeapChoiceGenerator) prevHeapCG).getCurrentSymInputHeap();
	}
	assert PC != null;
	assert symInputHeap != null;

	/* We find the objects of the same type. */

	HeapNode[] prevSymRefs = symInputHeap.getNodesOfType(typeClassInfo);

	/* We define the index into JPF's dynamic area. */

	int daIndex = 0;

	if (currentChoice < prevSymRefs.length) {
	    /* If currentChoice is an indice in the array, we are
	     * handling the case of a previously initiliazed
	     * object. */

	    HeapNode candidateNode = prevSymRefs[currentChoice];

	    daIndex = candidateNode.getIndex();	    

	    PC._star(SL.Pointsto((SymbolicInteger) attr, candidateNode.getSymbolic()));
	}
	else if (currentChoice == prevSymRefs.length
		 && !(((IntegerExpression) attr).toString()).contains("this")) {
	    /* If currentChoice is just outside the bounds, and if we
	     * are not in the THIS case, then we are in the NULL
	     * case. */

	    daIndex = MJIEnv.NULL;

	    PC._star(SL.Eq((SymbolicInteger) attr, SL.Null()));
	}
	else if ((currentChoice == (prevSymRefs.length + 1) && !abstractClass)
		 || (currentChoice == prevSymRefs.length && (((IntegerExpression) attr).toString()).contains("this"))) {
	    /* If we are in the case of a fresh object, we create one
	     * with all fields symbolic. */

	    boolean shared = (ei == null ? false : ei.isShared());
	    daIndex = Helper.addNewHeapNode(typeClassInfo, th, attr, symInputHeap, shared, PC);

	    /* Get the newly created node. */

	    SymbolicInteger freshNode = symInputHeap.getNode(daIndex);
	    assert freshNode != null;

	    PC._star(SL.Pointsto((SymbolicInteger) attr, freshNode));
	}
	else {
	    /* Otherwise, we are in the case of subtypes, which is not
	     * currently handled by SPF. */

	    System.err.println("subtypes not handled");
	}

	/* Once all of this is done, we push what we need to on the
	 * stack, update our path condition and symbolic input heap,
	 * and return. */

	sf.setLocalVariable(index, daIndex, true);
	sf.setLocalAttr(index, null);
	sf.push(daIndex, true);

	if (SL.debugMode)
	    System.out.println("ALOAD:    " + PC.toString());

	((HeapChoiceGenerator) thisHeapCG).setCurrentPC(PC);
	((HeapChoiceGenerator) thisHeapCG).setCurrentSymInputHeap(symInputHeap);

	return getNext(th);
    }
}
