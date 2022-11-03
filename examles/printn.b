/* The following function will print a non-negative number, n, to
   the base b, where 2<=b<=10.  This routine uses the fact that
   in the ASCII character set, the digits 0 to 9 have sequential
   code values.  */

printn(n, b) {
        extrn putchar;
        auto a;

        if (a = n / b)        /* assignment, not test for equality */
                printn(a, b); /* recursive */
        putchar(n % b + '0');
}
