int sum(int *arr, int n){
    if(n == 0){
        return arr[0];
    }
    return arr[n] + sum(arr, n-1);
}

int main(){
    int arr[] = {1,2,3,4,5};
    int result = sum(arr, 4);
    return 0;
}

