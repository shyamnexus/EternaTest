// This script estimates the maximum errors of `rsqrt` approximation for
// ARM NEON, SSE, AVX2, and AVX-512.
//
// Compile with Clang or GCC:
//
//      $ gcc rsqrt.c -o rsqrt -std=c99 -lm -march=native -O3 && time ./rsqrt
//      $ gcc rsqrt.c -o rsqrt -std=c99 -lm -march=skylake-avx512 -O3 && time ./rsqrt
//      $ gcc rsqrt.c -o rsqrt -std=c99 -lm -march=haswell -O3 && time ./rsqrt
//      $ gcc rsqrt.c -o rsqrt -std=c99 -lm -march=westmere -O3 && time ./rsqrt
//
// It may take around 10 seconds on a CPU with ~2 GHz clock speed and also depends
// on which ISA is used. AVX-512 instructions are clearly the heaviest.
// Sample code is from https://gist.github.com/ashvardanian
//
#include <float.h>  // `FLT_MIN`
#include <math.h>   // `sqrtf`
#include <stdint.h> // `uint32_t`
#include <stdio.h>  // `printf`

#if defined(__ARM_NEON)
#include <arm_neon.h> // ARM NEON
#elif defined(__AVX512F__)
#include <immintrin.h> // AVX-512
#elif defined(__AVX2__)
#include <immintrin.h> // AVX2
#elif defined(__SSE__)
#include <xmmintrin.h> // SSE
#endif

// Macro for computing the rsqrt approximation depending on the platform
#if defined(__ARM_NEON)

// Consider performing one Newton-Raphson iteration
//
//      vy = vmul_f32(vrsqrts_f32(vmul_f32(vx, vy), vy), vy);
//
// It can have a massive impact on the accuracy of the result. Without it:
//
//      - Maximum absolute error: 2.532945e+16
//      - Maximum relative error: 3.276857e-03
//
// With one iteration:
//
//      - Maximum absolute error: 1.077521e+14
//      - Maximum relative error: 1.621436e-05
//
// With two iterations:
//
//      - Maximum absolute error: 1.649267e+12
//      - Maximum relative error: 2.051842e-07
//
float rsqrt_approx(float x) {
    float32x2_t vx = vdup_n_f32(x);
    float32x2_t vy = vrsqrte_f32(vx);
    vy = vmul_f32(vrsqrts_f32(vmul_f32(vx, vy), vy), vy); // Newton-Raphson iteration 1
    vy = vmul_f32(vrsqrts_f32(vmul_f32(vx, vy), vy), vy); // Newton-Raphson iteration 2
    return vget_lane_f32(vy, 0);
}

#elif defined(__AVX512F__)

// Expected result:
//
//      - Maximum absolute error: 5.151212e+14
//      - Maximum relative error: 6.000695e-05
//
float rsqrt_approx(float x) {
    __m512 vx = _mm512_set1_ps(x);
    __m512 vy = _mm512_rsqrt14_ps(vx);
    return _mm512_cvtss_f32(vy);
}

#elif defined(__AVX2__)

// Expected result:
//
//      - Maximum absolute error: 2.738334e+15
//      - Maximum relative error: 3.261936e-04
//
float rsqrt_approx(float x) {
    __m256 vx = _mm256_set1_ps(x);
    __m256 vy = _mm256_rsqrt_ps(vx);
    return _mm256_cvtss_f32(vy);
}

#elif defined(__SSE__)

float rsqrt_approx(float x) {
    __m128 vx = _mm_set_ps1(x);
    __m128 vy = _mm_rsqrt_ps(vx);
    return _mm_cvtss_f32(vy);
}

#else

float rsqrt_approx(float x) {
    // Fallback: use the standard library `sqrtf`
    return 1.0f / sqrtf(x);
}

#endif

int main(int argc, char **argv) {
    double max_abs_err = 0.0, max_rel_err = 0.0;
    union {
        uint32_t u;
        float f;
    } u;
    printf("Starting `rsqrt` error estimate...\n");

    for (uint32_t i = 0;; i++) {
        u.u = i;
        float x = u.f;

        if (i == UINT32_MAX)
            break;
        // Only process positive, finite, non-zero values
        if (x <= FLT_MIN || !isfinite(x))
            continue;

        // Compute reciprocal square root using platform-specific intrinsic
        float y = rsqrt_approx(x);

        // Accurate computation using single-precision sqrtf
        float accurate_y = 1.0f / sqrtf(x);

        // Compute errors in double-precision
        double abs_err = fabs((double)y - (double)accurate_y);
        double rel_err = fabs(abs_err / (double)accurate_y);

        if (abs_err > max_abs_err)
            max_abs_err = abs_err;
        if (rel_err > max_rel_err)
            max_rel_err = rel_err;
    }

    printf("Maximum absolute error: %e\n", max_abs_err);
    printf("Maximum relative error: %e\n", max_rel_err);
    return 0;
}
