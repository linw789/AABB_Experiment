#include <cstring>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <windows.h>

using namespace std;

float vec3_dot(float a[3], float b[3]) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

// The matrix is column-major, meaning m[1][0] is at the 
// second column and the first row.
struct Mat3 {
    float m[3][3];

    Mat3() {
        for (size_t c = 0; c < 3; ++c) {
            for (size_t r = 0; r < 3; ++r) {
                m[c][r] = 0.0f;
            }
        }
    }

    // Input array `e` is row-major for readability.
    Mat3(const float (&e)[9]) {
        for (size_t r = 0; r < 3; ++r) {
            for (size_t c = 0; c < 3; ++c) {
                m[c][r] = e[r * 3 + c];
            }
        }
    }
};

Mat3 operator*(const Mat3& a, const Mat3& b) {
    Mat3 result = {};
    for (size_t c = 0; c < 3; ++c) {
        for (size_t r = 0; r < 3; ++r) {
            for (size_t i = 0; i < 3; ++i) {
                result.m[c][r] += a.m[i][r] * b.m[c][i];
            }
        }
    }
    return result;
}

struct AABB {
    float center[3];
    float half_exts[3];
};

AABB calc_aabb1(AABB a, float m[3][3], float t[3]) {
    AABB new_aabb = {};

    for (size_t i = 0; i < 3; i++) {
        new_aabb.center[i] = t[i];
        for (size_t j = 0; j < 3; j++) {
            new_aabb.center[i] += m[i][j] * a.center[j];
            new_aabb.half_exts[i] += fabsf(m[i][j]) * a.half_exts[j];
        }
    }

    return new_aabb;
}

AABB calc_aabb2(AABB a, float m[3][3], float t[3]) {
    // Transform the world axes into the box's local space. For example, the 
    // world x axis is transformed into: dot(world_mat^-1, Vec3(1, 0, 0)). 
    // Because world_mat is ortho-normal, the transformed world x axis is just
    // the first row of world_mat. The same reasoning goes to world y and z axes.
    float inv_world_axis_x[3] = { m[0][0], m[1][0], m[2][0] };
    float inv_world_axis_y[3] = { m[0][1], m[1][1], m[2][1] };
    float inv_world_axis_z[3] = { m[0][2], m[1][2], m[2][2] };

    // NB. The following ternary operations really kill the performance. I think in 
    // `calc_aabb1` branch predication in the for loop can be heavily optimized. But
    // here they cannot.
    // Find the furthest point along a direction.
    float furthest_px[3] = {
        /*inv_world_axis_x[0] > 0 ? a.half_exts[0] :*/ -a.half_exts[0],
        /*inv_world_axis_x[1] > 0 ? a.half_exts[1] :*/ -a.half_exts[1],
        /*inv_world_axis_x[2] > 0 ? a.half_exts[2] :*/ -a.half_exts[2],
    };
    float furthest_py[3] = {
        /*inv_world_axis_y[0] > 0 ? a.half_exts[0] :*/ -a.half_exts[0],
        /*inv_world_axis_y[1] > 0 ? a.half_exts[1] :*/ -a.half_exts[1],
        /*inv_world_axis_y[2] > 0 ? a.half_exts[2] :*/ -a.half_exts[2],
    };
    float furthest_pz[3] = {
        /*inv_world_axis_z[0] > 0 ? a.half_exts[0] :*/ -a.half_exts[0],
        /*inv_world_axis_z[1] > 0 ? a.half_exts[1] :*/ -a.half_exts[1],
        /*inv_world_axis_z[2] > 0 ? a.half_exts[2] :*/ -a.half_exts[2],
    };

    AABB new_aabb = {
        { t[0], t[1], t[2] },

        // Transform furthest points into world space, simpified as below:
        {
            vec3_dot(inv_world_axis_x, furthest_px),
            vec3_dot(inv_world_axis_y, furthest_py),
            vec3_dot(inv_world_axis_z, furthest_pz)
        }
    };
    return new_aabb;
}

void print_mat3(const Mat3& m) {
    for (size_t r = 0; r < 3; ++r) {
        cout << m.m[0][r] << " " << m.m[1][r] << " " << m.m[2][r] << endl;
    }
}

int main(int argc, char** argv) {
    const float PI2 = 3.1415926f * 2;

    const size_t ROT_MATRIX_COUNT = 100000;
    vector<Mat3> rots(ROT_MATRIX_COUNT);

    srand(1771);

    for (size_t i = 0; i < ROT_MATRIX_COUNT; i++)
    {
        float rx = rand() / (float)RAND_MAX * PI2; 
        float ry = rand() / (float)RAND_MAX * PI2; 
        float rz = rand() / (float)RAND_MAX * PI2;

        Mat3 rotx({
            1.0f, 0.0f, 0.0f,
            0.0f, cosf(rx), -sinf(rx),
            0.0f, sinf(rx), cosf(rx)
        });
        Mat3 roty({
            cosf(ry),  0.0f, sinf(ry),
            0.0f,      1.0f, 0.0f,
            -sinf(ry), 0.0f, cosf(ry)
        });
        Mat3 rotz({
            cosf(rz), -sinf(rz), 0.0f,
            sinf(rz), cosf(rz),  0.0f,
            0.0f,     0.0f,      1.0f
        });

        rots[i] = rotz * roty * rotx;
    }

    float t[3] = { 11.0f, 21.0f, 31.0f };

    AABB a = {
        {1.0f, 2.0f, 3.0f},
        {3.0f, 4.0f, 5.0f}
    };

    LARGE_INTEGER st, et; 
    LARGE_INTEGER elapsed_macros1, elapsed_macros2;
    LARGE_INTEGER freq;

    QueryPerformanceFrequency(&freq);

    vector<AABB> b1(ROT_MATRIX_COUNT);

    QueryPerformanceCounter(&st);

    for (size_t i = 0; i < ROT_MATRIX_COUNT; i++) {
        b1[i] = calc_aabb1(a, rots[i].m, t);
    }

    QueryPerformanceCounter(&et);
    elapsed_macros1.QuadPart = et.QuadPart - st.QuadPart;

    volatile float dummy1 = 0;
    for (size_t i = 0; i < ROT_MATRIX_COUNT; i++) {
        dummy1 += b1[i].center[0];
    }

    vector<AABB> b2(ROT_MATRIX_COUNT);

    QueryPerformanceCounter(&st);

    for (size_t i = 0; i < ROT_MATRIX_COUNT; i++) {
        b2[i] = calc_aabb2(a, rots[i].m, t);
    }

    QueryPerformanceCounter(&et);
    elapsed_macros2.QuadPart = et.QuadPart - st.QuadPart;
    
    volatile float dummy2 = 0;
    for (size_t i = 0; i < ROT_MATRIX_COUNT; i++) {
        dummy2 += b1[i].center[0];
    }

    cout << "calc_aabb1: " << (elapsed_macros1.QuadPart * 1000'000 / freq.QuadPart) << " macros" << endl;
    cout << "calc_aabb2: " << (elapsed_macros2.QuadPart * 1000'000 / freq.QuadPart) << " macros" << endl;

    return 0;
}