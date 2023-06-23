/* This program prints out the n-th fibonacci number. */

n 10;

main() {
    extrn printf, fib, fib_rec, n;
    printf("%d*n%d*n", fib(n), fib_rec(n));
}

fib(n) {
    auto a, b, c, i;
    b = 1;
    i = 0;

    while(i++ < n) {
        c = a + b;
        a = b;
        b = c;
    }

    return(a);
}

fib_rec(n) {
    if(n <= 1)
        return(n);
    else
        return(fib_rec(n - 1) + fib_rec(n - 2));
}