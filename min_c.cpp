#include <iostream>
#include <list>
using namespace std;
#define MAX 100000
 
// dp array to memoize the results
int dp[MAX + 1];
 
// List to store the result
list<int> denomination;
 
// Function to find the minimum number of
// coins to make the sum equals to X
int countMinCoins(int n, int C[], int m)
{
    // Base case
    if (n == 0) {
        dp[0] = 0;
        return 0;
    }
 
    // If previously computed
    // subproblem occurred
    if (dp[n] != -1)
        return dp[n];
 
    // Initialize result
    int ret = INT_MAX;
 
    // Try every coin that has smaller
    // value than n
    for (int i = 0; i < m; i++) {
 
        if (C[i] <= n) {
 
            int x
                = countMinCoins(n - C[i],
                                C, m);
 
            // Check for INT_MAX to avoid
            // overflow and see if result
            // can be minimized
            if (x != INT_MAX)
                ret = min(ret, 1 + x);
        }
    }
 
    // Memoizing value of current state
    dp[n] = ret;
    return ret;
}
 
// Function to find the possible
// combination of coins to make
// the sum equal to X
void findSolution(int n, int C[], int m)
{
    // Base Case
    if (n == 0) {
 
        // Print Solutions
        for (auto it : denomination) {
            cout << it << ' ';
        }
 
        return;
    }
 
    for (int i = 0; i < m; i++) {
 
        // Try every coin that has
        // value smaller than n
        if (n - C[i] >= 0
            and dp[n - C[i]] + 1
                    == dp[n]) {
 
            // Add current denominations
            denomination.push_back(C[i]);
 
            // Backtrack
            findSolution(n - C[i], C, m);
            break;
        }
    }
}
 
// Function to find the minimum
// combinations of coins for value X
void countMinCoinsUtil(int X, int C[],
                       int N)
{
 
    // Initialize dp with -1
    memset(dp, -1, sizeof(dp));
 
    // Min coins
    int isPossible
        = countMinCoins(X, C,
                        N);
 
    // If no solution exists
    if (isPossible == INT_MAX) {
        cout << "-1";
    }
 
    // Backtrack to find the solution
    else {
        findSolution(X, C, N);
    }
}
 
// Driver code
int main()
{
    int X = 8000;
 
    // Set of possible denominations
    int arr[] = { 2, 3, 4, 5,8,9,7,6,12,65,84};
 
    int N = sizeof(arr) / sizeof(arr[0]);
 
    // Function Call
    countMinCoinsUtil(X, arr, N);
 
    return 0;
}