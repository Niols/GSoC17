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

public class ValuedNode<T> {
    ValuedNode<T> next;
    T value;

    public ValuedNode(ValuedNode<T> next, T value) {
	this.next = next;
	this.value = value;
    }
    
    public ValuedNode<T> getNext() {
	return next;
    }

    public void setNext(ValuedNode<T> next) {
	this.next = next;
    }
    
    public T getValue() {
	return value;
    }

    public void setValue(T value) {
	this.value = value;
    }
}
