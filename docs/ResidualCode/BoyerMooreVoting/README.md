https://www.geeksforgeeks.org/boyer-moore-majority-voting-algorithm/

The **Boyer-Moore voting** algorithm is one of the popular optimal algorithms which is used to find the majority element among the given elements that have more than N/ 2 occurrences. This works perfectly fine for finding the majority element which takes 2 traversals over the given elements, which works in O(N) time complexity and O(1) space complexity.

Let us see the algorithm and intuition behind its working, by taking an example –

```
Input :{1,1,1,1,2,3,5}
Output : 1
Explanation : 1 occurs more than 3 times.
Input : {1,2,3}
Output : -1
```

This algorithm works on the fact that if an element occurs more than N/2 times, it means that the remaining elements other than this would definitely be less than N/2. So let us check the proceeding of the algorithm.

- First, choose a candidate from the given set of elements if it is the same as the candidate element, increase the votes. Otherwise, decrease the votes if votes become 0, select another new element as the new candidate.

**Intuition Behind Working :**
When the elements are the same as the candidate element, votes are incremented whereas when some other element is found (not equal to the candidate element), we decreased the count. This actually means that we are decreasing the priority of winning ability of the selected candidate, since we know that if the candidate is in majority it occurs more than N/2 times and the remaining elements are less than N/2. We keep decreasing the votes since we found some different element(s) than the candidate element. When votes become 0, this actually means that there are the equal  number of votes for different elements, which should not be the case for the element to be the majority element. So the candidate element cannot be the majority and hence we choose the present element as the candidate and continue the same till all the elements get finished. The final candidate would be our majority element. We check using the 2nd traversal to see whether its count is greater than N/2. If it is true, we consider it as the majority element.

**Steps to implement the algorithm :**
**Step 1 –** Find a candidate with the majority –

- Initialize a variable say **i ,votes = 0, candidate =-1**
- Traverse through the array using for loop
- If **votes = 0,** choose the **candidate = arr[i]** , make **votes=1**.
- else if the current element is the same as the candidate increment votes
- else decrement votes.

**Step 2 –** Check if the candidate has more than N/2 votes –

- Initialize a variable count =0 and increment count if it is the same as the candidate.
- If the count is >N/2, return the candidate.
- else return -1.

```
Dry run for the above example:
Given :
  arr[]=        1    1    1    1    2    3    5
 votes =0       1    2    3    4    3    2    1
 candidate = -1 1    1    1    1    1    1    1
 candidate = 1  after first traversal
                1    1    1    1    2    3    5
 count =0       1    2    3    4    4    4    4
 candidate = 1
 Hence count > 7/2 =3
 So 1 is the majority element.
```