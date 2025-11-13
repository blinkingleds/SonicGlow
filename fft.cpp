#include "fft.h"
#include <vector>
#include <complex>
#include <cmath>
#include <algorithm> // For std::swap

// Using declaration for complex double
using cd = std::complex<double>;


void fft(std::vector<cd> &a) {
    int n = a.size();

    // Bit-reversal 
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j)
            std::swap(a[i], a[j]);
    }

    // compute FFT
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2 * PI / len;
        cd wlen(cos(ang), sin(ang));
        for (int i = 0; i < n; i += len) {
            cd w(1);
            for (int j = 0; j < len / 2; j++) {
                cd u = a[i + j];
                cd v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}