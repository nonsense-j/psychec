int f(int p) {
    if (p > 2)
        p = p * 2;
    else
        p = p - 10;
    p *= 2;
    return p;
}
int g(int p) {
    if (p > 2) p += 1;
    p = f(p);
    return p;
}