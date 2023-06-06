# Complete Sum Free Sets

## Definitions
### Sum Free
A sum free set for the constant `n` is any set where the sum of any two elements mod `n` is not in the set. <br>
e.g. the set {1, 3, 5} when `n = 6` is sum free because
```
(1 + 1) % 6 = 2
(1 + 3) % 6 = 4
(1 + 5) % 6 = 0
(3 + 3) % 6 = 0
(3 + 5) % 6 = 2
(5 + 5) % 6 = 4

{1, 3, 5} does not contain 0, 2, or 4
```

### Completeness
A sum free set is complete when all values on the interval `[0, n-1]` that are not in the set are contained in the sums of the set. <br>
Functionally this means to check for completeness first we need to find the complementary set. Then we check if all the values of in the complementary set are contained by the sums set. <br>
e.g. the set {1, 3, 5} when `n = 6`
```
sums set = {0, 2, 4}
complementary set = {0, 2, 4}

sums set contains 0, 2, and 4
```


## Implementations
### Sums
To calculate all the possible sums an outer product of the set and itself can be used. <br>
e.g. the set {1, 3, 5} when `n = 6`
```
     | 1   | 3   | 5   |
| 1  | 2   | 4   | 0   |
| 3  | 4   | 0   | 2   |
| 5  | 0   | 2   | 4   |
```
Due to the nature of finding the outer product of two identical sets there are duplicate sums. <br>
They can be removed by finding a diagonal and ignoring anything below it. <br>
e.g. \'s are the diagonal
```
     | 1   | 3   | 5   |
| 1  \ 2   | 4   | 0   |
| 3  | N/A \ 0   | 2   |
| 4  | N/A | N/A \ 4   |
```
This matrix is compressed into its own set for ease of use

### Sum Free
To check if a set is sum free we simply need to loop over all the values in the sums set. <br>
For each sum we need to check if the original set contains it.

### Complete
To check for completeness we find the complementary set then loop through all the values in the sums set.
For each sum we use it as an index in a boolean array to set it as found.
After we've gone through all the values in the sums set we check if all the values in the boolean array are true.


## Optimization
### Pre Calculations
There are two ways to eliminate a large number of sets that cannot be complete.
#### Length
All complete sets must have at least as many sums as there are values in the complementary set.
Because of this we can calculate the number of sums and the number of complementary elements with the below formulas
```
|s| is the size of the set

Number of complementary elements = n - |s|

Number of sums = (|s|(|s| + 1)) / 2
```

#### Maximum First Value
For any `n` value there is a calculable maximum first value for complete sum free sets.
The equation for this is
```
(n+1) / 3
```

### Sum Free
Instead of finding all the sums then checking if they are sum free we can add a check to our sum calculations. If this check fails we stop calculating sums and mark the set and not sum free.


### Binary representation
A set can be represented in binary by having the position from left to right represent a number and the bit represent if that number is in the set. For example:
```
{1, 3, 5}: 101010
1 0 1 0 1 0
^ ^ ^ ^ ^ ^
5 4 3 2 1 0
```
