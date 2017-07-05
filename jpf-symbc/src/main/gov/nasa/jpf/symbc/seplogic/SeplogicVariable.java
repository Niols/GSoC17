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

package gov.nasa.jpf.symbc.seplogic;

import gov.nasa.jpf.symbc.numeric.SymbolicInteger;

public class SeplogicVariable implements SeplogicValue {
    private final SymbolicInteger n; //FIXME: emancipate!
    private final SeplogicType t;
    
    public SeplogicVariable(SymbolicInteger n, SeplogicType t) {
	this.n = n;
	this.t = t;
    }

    public String toString(boolean withTypes) {
	int code = n.hashCode();
	String repr;
	
	if (code < 26)
	    /* Try to print a letter of the alphabet. */
	    repr = String.valueOf("pqrstuvwxyzabcdefghijklmno".charAt(code));
	else
	    /* If you can't, fall back on the integer value. */
	    repr = "?" + String.valueOf(code);

	if (withTypes)
	    repr += " : " + getType().toString();

	return repr;
    }

    public String toString() {
	return toString(false);
    }
    
    public SeplogicValue copy() {
	return this; //FIXME: sure?
    }

    public SymbolicInteger getSymbolic() {
	return n;
    }

    public SeplogicType getType() {
	return t;
    }
    
    public boolean equals(SeplogicVariable v) {
	return (n.equals(v.getSymbolic()));
    }

    public boolean equals(Object o) {
	return (o instanceof SeplogicVariable) && equals((SeplogicVariable) o);
    }
}
