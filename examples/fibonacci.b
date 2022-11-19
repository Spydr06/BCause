/* This program prints out the n-th fibonacci number. */

n 10;

main() {
    extrn printf, fib, n;
    printf("%d*n", fib(n));
}

fib(n) {
    auto a, b 1, c, i 0;

    while(i++ < n) {
        c = a + b;
        a = b;
        b = c;
    }

    return(a);
}