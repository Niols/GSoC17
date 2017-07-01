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

public class TestExtends {

    public static void test(SetableNode n) {
	/* This useless function makes sure that a SetableNode is not
	 * the only node in the list. */
	
	SetableNode m = n;

	if (m == null) {
	    System.out.println("The node was null ><. I can't work in those conditions!");
	}
	else if (m.getNext() == null) {
	    System.out.println("The node was alone :-(. I had to give him a friend.");
	    m.setNext(new SetableNode(null));
	}
	else {
	    System.out.println("The node already had a friend! :-).");
	}
    }
    
    public static void main(String[] args) {
	SetableNode n = new SetableNode(null);
	test(n);
    }
}
