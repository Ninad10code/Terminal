#include<stdio.h>
void function(int *newsize)
{
    *newsize+=1;
}

void function1(int length)
{
    length=3;
}
int main()
{
    int size=0;
    function(&size);
    printf("%d\n",size);
    function1(size);
    printf("%d\n",size);

}