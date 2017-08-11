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

package gov.nasa.jpf.symbc.heap.seplogic;

/* Java imports */
import java.util.Map;
import java.util.HashMap;

/* JPF imports */
import gov.nasa.jpf.vm.BooleanFieldInfo;
import gov.nasa.jpf.vm.ClassInfo;
import gov.nasa.jpf.vm.DoubleFieldInfo;
import gov.nasa.jpf.vm.ElementInfo;
import gov.nasa.jpf.vm.FieldInfo;
import gov.nasa.jpf.vm.FloatFieldInfo;
import gov.nasa.jpf.vm.IntegerFieldInfo;
import gov.nasa.jpf.vm.KernelState;
import gov.nasa.jpf.vm.LongFieldInfo;
import gov.nasa.jpf.vm.ReferenceFieldInfo;
import gov.nasa.jpf.vm.StaticElementInfo;
import gov.nasa.jpf.vm.ThreadInfo;

/* SPF imports */
import gov.nasa.jpf.symbc.arrays.ArrayExpression;
import gov.nasa.jpf.symbc.arrays.ArrayHeapNode;
import gov.nasa.jpf.symbc.arrays.HelperResult;
import gov.nasa.jpf.symbc.numeric.Comparator;
import gov.nasa.jpf.symbc.numeric.Expression;
import gov.nasa.jpf.symbc.numeric.IntegerConstant;
import gov.nasa.jpf.symbc.numeric.IntegerExpression;
import gov.nasa.jpf.symbc.numeric.PCChoiceGenerator;
import gov.nasa.jpf.symbc.numeric.SymbolicInteger;
import gov.nasa.jpf.symbc.numeric.SymbolicReal;
import gov.nasa.jpf.symbc.string.StringSymbolic;
import gov.nasa.jpf.symbc.heap.HeapNode;
import gov.nasa.jpf.symbc.heap.SymbolicInputHeap;

/* SPF+SL imports */
import gov.nasa.jpf.symbc.heap.seplogic.PathCondition;

public class Helper {

    public static Expression initializeInstanceField(FieldInfo field, ElementInfo eiRef,
						     String refChain, String suffix) {
	Expression sym_v = null;
	String name ="";

	name = field.getName();
	String fullName = refChain + "." + name + suffix;
	if (field instanceof IntegerFieldInfo
	    || field instanceof LongFieldInfo) {
	    sym_v = new SymbolicInteger(fullName);
	}
	else if (field instanceof FloatFieldInfo
		 || field instanceof DoubleFieldInfo) {
	    sym_v = new SymbolicReal(fullName);
	}
	else if (field instanceof ReferenceFieldInfo) {
	    if (field.getType().equals("java.lang.String"))
		sym_v = new StringSymbolic(fullName);
	    else
		sym_v = new SymbolicInteger(fullName);
	}
	else if (field instanceof BooleanFieldInfo) {
	    //	treat boolean as an integer with range [0,1]
	    sym_v = new SymbolicInteger(fullName, 0, 1);
	}
	eiRef.setFieldAttr(field, sym_v);
	return sym_v;
    }
    
    public static Expression[] initializeInstanceFields(FieldInfo[] fields, ElementInfo eiRef,
						String refChain) {
	Expression[] sym_vs = new Expression[fields.length];
	
	for (int i=0; i<fields.length;i++)
	    sym_vs[i] = initializeInstanceField(fields[i], eiRef, refChain, "");

	return sym_vs;
    }

    public static Expression initializeStaticField(FieldInfo staticField, ClassInfo ci,
						   ThreadInfo ti, String suffix){

	Expression sym_v = null;
	String name ="";

	name = staticField.getName();
	String fullName = ci.getName() + "." + name + suffix;// + "_init";
	
	if (staticField instanceof IntegerFieldInfo
	    || staticField instanceof LongFieldInfo) {
	    sym_v = new SymbolicInteger(fullName);
	}
	else if (staticField instanceof FloatFieldInfo
		 || staticField instanceof DoubleFieldInfo) {
	    sym_v = new SymbolicReal(fullName);
	}
	else if (staticField instanceof ReferenceFieldInfo) {
	    if (staticField.getType().equals("java.lang.String"))
		sym_v = new StringSymbolic(fullName);
	    else
		sym_v = new SymbolicInteger(fullName);
	}
	else if (staticField instanceof BooleanFieldInfo) {
	    // treat boolean as an integer with range [0,1]
	    sym_v = new SymbolicInteger(fullName, 0, 1);
	}
	
	StaticElementInfo sei = ci.getModifiableStaticElementInfo();
	if (sei == null) {
	    ci.registerClass(ti);
	    sei = ci.getStaticElementInfo();
	}
	if (sei.getFieldAttr(staticField) == null) {
	    sei.setFieldAttr(staticField, sym_v);
	}
	return sym_v;
    }

    public static Expression[] initializeStaticFields(FieldInfo[] staticFields, ClassInfo ci,
						      ThreadInfo ti) {
	Expression[] sym_vs = new Expression[staticFields.length];
	
	for (int i = 0; i < staticFields.length; i++)
	    sym_vs[i] = initializeStaticField(staticFields[i], ci, ti, "");

	return sym_vs;
    }

    public static SymbolicInteger[] expressionArrayToSymbolicIntegerArray(Expression[] symbolicValues) {
	SymbolicInteger[] symbolicIntegers = new SymbolicInteger[symbolicValues.length];

	//FIXME: the fact that we assume that all expressions are
	//symbolicIntegers will probably be annoying in the future.

	for (int i = 0; i < symbolicValues.length; i++)
	    symbolicIntegers[i] = (SymbolicInteger) symbolicValues[i];

	return symbolicIntegers;
    }

    public static FieldInfo[] mergeFieldInfoArrays(FieldInfo[] a1, FieldInfo[] a2) {
	//FIXME: when connected to internet, find a way to do that
	//with the stdlib.	
	FieldInfo[] a = new FieldInfo[a1.length + a2.length];
	for (int i = 0; i < a1.length; i++) a[i] = a1[i];
	for (int j = 0; j < a2.length; j++) a[j + a1.length] = a2[j];
	return a;
    }
    public static SymbolicInteger[] mergeSymbolicIntegerArrays(SymbolicInteger[] a1, SymbolicInteger[] a2) {
	//FIXME: when connected to internet, find a way to do that
	//with the stdlib.	
	SymbolicInteger[] a = new SymbolicInteger[a1.length + a2.length];
	for (int i = 0; i < a1.length; i++) a[i] = a1[i];
	for (int j = 0; j < a2.length; j++) a[j + a1.length] = a2[j];
	return a;
    }
    
    public static int addNewHeapNode(ClassInfo typeClassInfo, ThreadInfo ti,
				     Object attr, SymbolicInputHeap symInputHeap,
				     boolean setShared, PathCondition PC) {
	int daIndex = ti.getHeap().newObject(typeClassInfo, ti).getObjectRef();
	ti.getHeap().registerPinDown(daIndex);
	String refChain = ((SymbolicInteger) attr).getName(); // + "[" + daIndex + "]"; // do we really need to add daIndex here?
	SymbolicInteger newSymRef = new SymbolicInteger(refChain);
	ElementInfo eiRef =  ti.getModifiableElementInfo(daIndex);//ti.getElementInfo(daIndex); // TODO to review!
	if(setShared) {
	    eiRef.setShared(ti,true);//??
	}
	//daIndex.getObjectRef() -> number

	/* Create all the fields necessary. */
	
	int numOfFields = eiRef.getNumberOfFields();
	FieldInfo[] fields = new FieldInfo[numOfFields];
	for(int fieldIndex = 0; fieldIndex < numOfFields; fieldIndex++) {
	    fields[fieldIndex] = eiRef.getFieldInfo(fieldIndex);
	}

	Expression[] symbolicValues = Helper.initializeInstanceFields(fields, eiRef,refChain);
	SymbolicInteger[] symbolicIntegers = expressionArrayToSymbolicIntegerArray(symbolicValues);
	
	/* Idem for static fields. */
	
	ClassInfo superClass = typeClassInfo;
	while(superClass != null) {
	    FieldInfo[] staticFields = superClass.getDeclaredStaticFields();
	    fields = mergeFieldInfoArrays(fields, staticFields);
	    
	    Expression[] symbolicStaticValues = Helper.initializeStaticFields(staticFields, superClass, ti);
	    symbolicIntegers = mergeSymbolicIntegerArrays(symbolicIntegers, expressionArrayToSymbolicIntegerArray(symbolicStaticValues));
	    
	    superClass = superClass.getSuperClass();
	}

	// create new HeapNode based on above info
	// update associated symbolic input heap
	HeapNode n = new HeapNode(daIndex, typeClassInfo, newSymRef);
	symInputHeap._add(n);

	/* Update the constraint. */

	Map<String,SymbolicInteger> fieldsMap = new HashMap<String,SymbolicInteger>();
	for(int i = 0 ; i < fields.length ; i++)
	    fieldsMap.put(fields[i].getName(), symbolicIntegers[i]);

	PC.addEq((SymbolicInteger) attr, newSymRef);
	PC.addRecord((SymbolicInteger) attr, fieldsMap);
	
	return daIndex;
    }
}
