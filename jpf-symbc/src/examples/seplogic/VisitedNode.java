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

/* THIS IS NOT A TEST but a necessary file for other tests. */

package seplogic;

public class VisitedNode {
    VisitedNode next;
    boolean visited;

    public VisitedNode(VisitedNode next, boolean visited) {
	this.next = next;
	this.visited = visited;
    }

    public VisitedNode(VisitedNode next) {
	this(next, false);
    }

    public VisitedNode() {
	this(null);
    }
    
    public VisitedNode getNext() {
	return next;
    }

    public void setNext(VisitedNode next) {
	this.next = next;
    }

    public void setVisited(boolean visited) {
	this.visited = visited;
    }

    public void setVisited() {
	setVisited(true);
    }

    public void resetVisited() {
	setVisited(false);
    }
    
    public boolean visited() {
	return visited;
    }
}
