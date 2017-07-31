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

public class TestNoncyclic
{
    public static int length_noncyclic(VisitedNode node) {
	VisitedNode curr = node;
	int length = 0;
	
	while (curr != null) {

	    if (curr.visited()) {
		System.out.println("This list is cyclic! You lied to me!");
		return -1;
	    }
	    
	    length++;
	    curr.setVisited();
	    curr = curr.getNext();
	}

	return length;
    }
    
    public static void main(String[] args) {

	System.out.println("Length = " + length_noncyclic(new VisitedNode(new VisitedNode(new VisitedNode()))));
    }
}
