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

class Node {
    int value;
    Node next;

    public Node (int value, Node next) {
	this.value = value;
	this.next = next;
    }

    public int getValue() {
	return value;
    }
    
    public Node getNext() {
	return next;
    }
}

public class ALOAD {

    public static boolean moreThanTwo(Node n) {
	Node m = n;
	
	if (m == null) return false;
	m = m.getNext();

	if (m == null) return false;

	return true;
    }
    
    public static void main(String[] args) {
	Node n0 = null;
	Node n1 = new Node(1, n0);
	Node n2 = new Node(2, n1);
	Node n3 = new Node(3, n2);

	System.out.println(moreThanTwo(n0));
	System.out.println(moreThanTwo(n1));
	System.out.println(moreThanTwo(n2));
	System.out.println(moreThanTwo(n3));
    }
}
