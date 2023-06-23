/* The following program will print out the system time
   in the format "Mmm dd hh:mm::dd" */

main() {
    extrn nwrite, lchar, buffer, ctime, time;
    auto tv;

    time(&tv);
    ctime(&tv, &buffer);
    lchar(&buffer, 16, '*n');
    nwrite(1, &buffer, 17);
}
buffer[3] 0, 0, 0;
