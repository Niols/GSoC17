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

package seplogic;

public class TestExtends2 {

    public static void swap2(SetableNode n1, SetableNode n2) {
	/* This useless function makes sure that a SetableNode is not
	 * the only node in the list. */

	Node n12 = n1.getNext();
	n1.setNext(n2.getNext());
	n2.setNext(n12);
    }
    
    public static void main(String[] args) {
	SetableNode m = new SetableNode(null);
	SetableNode n = new SetableNode(new SetableNode(null));
	swap2(m, n);
	System.out.println("The End!");
    }
}
