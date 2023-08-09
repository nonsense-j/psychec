void compare(int len1, int len2, int a[5], int b[5]) {
    int i = 0;
    int result;
    while ((a[i] == b[i]) && (len1 > 0) && (len2 > 0)) {
        i = i + 1;
        len1 = len1 - 1;
        len2 = len2 - 1;
    }
    if ((len1 == len2) && (len1 == 0)) result = 0;
    if (((len1 == 0) && (len2 > len1)) || (a[i] < b[i])) result = 1;
    if (((len2 == 0) && (len1 > len2)) || (a[i] > b[i])) result = 2;
}
