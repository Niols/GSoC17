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

public class TestTreeNode {

    public static boolean isLeaf(TreeNode n) {
	return (n != null) && (n.getLeft() == null) && (n.getRight() == null);
    }
    
    public static void main(String[] args) {
	TreeNode n =
	    new TreeNode(new TreeNode(null, 2, null), 1,
			 new TreeNode(null, 3, new TreeNode(null, 4, null)));

	System.out.println("isLeaf? " + isLeaf(n));
    }
}
