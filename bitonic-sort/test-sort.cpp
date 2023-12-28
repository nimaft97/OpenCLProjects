#include <iostream>
#include <vector>


int main()
{
    std::vector<int> arr = {3, 7, 4, 8, 6, 2, 1, 5};

    int k, i, l, j, n, tmp;

    n = arr.size();

    // given an array arr of length n, this code sorts it in place
    // all indices run from 0 to n-1
    for (k = 2; k <= n; k *= 2){ // k is doubled every iteration
        for (j = k/2; j > 0; j /= 2){ // j is halved at every iteration, with truncation of fractional parts
            for (i = 0; i < n; i++){
                l = i^j; // in C-like languages this is "i ^ j"
                if (l > i){
                    if (  ((i & k) == 0) && (arr[i] > arr[l]) ||
                          ((i & k) != 0) && (arr[i] < arr[l]) )
                    {
                        std::swap(arr[i], arr[l]);
                    }
                }
            }
        }
    }

    for (const auto elem : arr)
    {
        std::cout << elem << " ";
    }
    std::cout << std::endl;
}