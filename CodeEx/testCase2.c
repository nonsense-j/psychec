int g(int d) {
    while (d < 5) {
        if (d == 0) break;
        d = d + 1;
    }
    return d + 1;
}

int f(int p) {
    int v = 0;
    if (p == v || p > v) {
        p = 1;
        if (v > 1) p = g(p);
        p += 1;
    } else {
        p = 3;
        p += 2;
    }
    return p;
}