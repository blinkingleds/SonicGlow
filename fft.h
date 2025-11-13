#pragma once
#include <complex>
#include <vector>

using cd = std::complex<double>;
inline const double PI = acos(-1);



void fft(std::vector<cd> &a);