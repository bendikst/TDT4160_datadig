#include <stdio.h>

int main(void)
{
    // Deklarer en array med 10 heltall
    int arr[10];

    // Sett første og siste element, resten er uinitialisert
    arr[0] = 1;
    arr[9] = 10;

    // Les første element
    printf("arr[0] = %d\n", arr[0]);

    // Deklarer en peker til int, gi den adressen til første element i arrayet
    int *arrptr = arr;
    // Dette er det samme som:
    arrptr = &arr[0];

    // Dereferering av peker gir første element i arr
    *arrptr = 2;
    printf("*arrptr = %d, arr[0] = %d\n", *arrptr, arr[0]);

    // Vi kan bruke [] operator på pekere!
    arrptr[9] = 11;
    printf("arrptr[9] = %d, arr[9] = %d, \n", arrptr[9], arr[9]);

    // arrptr[9] er det samme som:
    *(arrptr + 9) = 12;
    printf("*(arrptr + 9) = %d, arr[9] = %d\n", *(arrptr + 9), arr[9]);

    // Vi kan peke til vilkårlige minneadresser
    arrptr = (int *)0xf00d;
    printf("arrptr = %p\n", arrptr);

    // Merk at pekeraritmetikk avhenger av størrelsen på det som pekes på!
    // Her legger vi til 1*4 bytes til adressen og får arrptr = 0xf011,
    // siden størrelsen til int er 4 bytes:
    arrptr += 1;
    printf("arrptr = %p\n", arrptr);

    return 0;
}
