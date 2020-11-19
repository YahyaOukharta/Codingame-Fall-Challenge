#include <iostream>
#include <queue>
#include <tuple>
#include <vector>
#include <unordered_set>

using namespace std;


vector<int> minNumbers(int x, int*arr, int n)
{
    queue < tuple<int, vector<int> > > q; //tuple contains current sum , and path

    q.push({x, {}});

    unordered_set<int> v;

    while (q.size())
    {
        //cout << "depth = "<<d<<endl;
        int s = q.size();
        //cout << "queue size = "<<s<<endl;

        while (s--)
        {   auto front = q.front();
            int current_sum = get<0>(front);
            vector<int> path = get<1>(front);

            if(!current_sum) // base case 
                return path;
            
            q.pop();
            if (v.find(current_sum) != v.end() or current_sum < 0) // already visited or negative number //
                continue;
            v.insert(current_sum);
            for(int i = 0; i < n; i++)
            {
                vector<int> v=path;
                v.push_back(arr[i]);
                q.push({current_sum - arr[i], v});
            }
        }
    }

    return {};
}

int main()
{
    int arr[] = {1,2,2,2,2,2};
    int n = sizeof(arr) / sizeof(int);
    int x = 80;

    // cout << minNumbers(x,arr,n)<<endl;
    for (auto c : minNumbers(x,arr,n))
    {
        cout<< " " << c;
    }
    cout << endl;
    return 0;
}

