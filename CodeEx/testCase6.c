int shellsort() {
    int s[5];
    int len;
    int i, j, gap;
    int temp;
    for (gap = len / 2; gap > 0; gap = gap / 2)
        for (i = gap; i < len; i = i + 1) {
            for (j = i - gap; j >= 0 && s[j] > s[j + gap]; j = j - gap) {
                temp = s[j + gap];
                s[j + gap] = s[j];
                s[j] = temp;
            }
        }
    return 1;
}
