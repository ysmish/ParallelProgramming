// 330829847 Ido Maimon
// 216764803 Yuli Smishkis

long decode_c_version(long x, long y, long z) {
    y = y - z;
    x = x * y;
    long w = y;
    w = w << 63;
    w = w >> 63;
    return (w ^ x);
}