#ifndef matrix
#define matrix
#include <vector>

using namespace std;
template <class T>
vector<vector<T>> mul(vector<vector<T>> m1, vector<vector<T>> m2) {
    vector<vector<T>> res(m1.size(), vector<T>(m2[0].size()));
    for (int i = 0; i < m1.size(); i++) {
        for (int j = 0; j < m2[0].size(); j++) {
            for (int k = 0; k < m1[0].size(); k++) {
                res[i][j] += m1[i][k] * m2[k][j];
            }
        }
    }
    return res;
}

float norm3f(vector<float> v) {
    return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

float dot3f(vector<float> v1, vector<float> v2) {
    return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

vector<float> crossprod(vector<float> v1, vector<float> v2) {
    return vector<float>{v1[1] * v2[2] - v2[1] * v1[2],
                         -(v1[0] * v2[2] - v2[0] * v1[2]),
                         v1[0] * v2[1] - v2[0] * v1[1]};
}
vector<float> normalize(vector<float> v) {
    float norm = norm3f(v);
    return vector<float>{v[0] / norm, v[1] / norm, v[2] / norm};
}
#endif