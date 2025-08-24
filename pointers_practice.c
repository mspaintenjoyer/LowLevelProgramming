#include <stdio.h>
#include <stdlib.h>

int main() {
    // Write C code here
    int *arr_ptr[50];
    int *p1,*p2,*p3;
    
    int a, b, c;
    printf("Enter three numbers \n");
    scanf("%d %d %d", &a,&b,&c);
    
    p1 = (int*)malloc(sizeof(int)); //malloc returns a void* pointer that is typecast to integer using (int*). p1 now holds the base address of a free allocated block of size 4 bytes.
    printf("Check this, %p is the address to which p1 points and %d is the value at the address  \n", p1 , *p1); 
    *p1 = a;

    p2 = &b;
    printf("The address at which b is stored %p. The value at this address %d", p2, *p2);

    p3 = &c;

    printf("\n I would like to display all the input numbers as an array");
    arr_ptr[0] = p1;//the value of p1 gets copied into arr_ptr[0]. Dereferencing arr_ptr now makes sense.
    arr_ptr[1] = p2;
    arr_ptr[2] = p3;


    for(int i = 0; i < 3; i++){  
        printf("\n %2d %d", arr_ptr[i], *arr_ptr[i]);
    }
    free(p1);
    

    return 0;
}