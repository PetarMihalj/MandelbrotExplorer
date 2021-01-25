#ifndef loader
#define loader

#include <vector>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

struct Obj {
    vector<int> faces;
    vector<float> vertices;
    Obj(istream &&str) {
        char c;
        while (true) {
            str >> c;
            if (str.eof())
                break;
            if (c == 'v') {
                float v1, v2, v3;
                str >> v1 >> v2 >> v3;
                vertices.push_back(v1);
                vertices.push_back(v2);
                vertices.push_back(v3);
            } else if (c == 'f') {
                int i1, i2, i3;
                str >> i1 >> i2 >> i3;
                faces.push_back(i1 - 1);
                faces.push_back(i2 - 1);
                faces.push_back(i3 - 1);
            } else {
                string d;
                getline(str, d);
            }
        }
    }
};

#include "matrix.h"

struct CubicSpl {
    vector<vector<float>> vertices;

    CubicSpl(istream &&str) {
        char c;
        while (true) {
            str >> c;
            if (str.eof())
                break;
            if (c == 'v') {
                float v1, v2, v3;
                str >> v1 >> v2 >> v3;
                vertices.push_back({v1, v2, v3});
            } else {
                string d;
                getline(str, d);
            }
        }
    }

    vector<float> eval(long long tll, long long Tll) {
        int begin = (tll / Tll) % (vertices.size() - 3);
        float t = (tll % Tll) / float(Tll);

        vector<vector<float>> m1 = vector<vector<float>>{
            {t * t * t / 6.0f, t * t / 6.0f, t / 6.0f, 1 / 6.0f}};
        vector<vector<float>> m2 = {
            {-1, 3, -3, 1}, {3, -6, 3, 0}, {-3, 0, 3, 0}, {1, 4, 1, 0}};
        vector<vector<float>> m3(vertices.begin() + begin,
                                 vertices.begin() + begin + 4);
        vector<vector<float>> res = mul(mul(m1, m2), m3);
        return {res[0][0], res[0][1], res[0][2]};
    }

    vector<float> eval_first(long long tll, long long Tll) {
        int begin = (tll / Tll) % (vertices.size() - 3);
        float t = (tll % Tll) / float(Tll);

        vector<vector<float>> m1 =
            vector<vector<float>>{{t * t / 2.0f, t / 2.0f, 1 / 2.0f}};
        vector<vector<float>> m2 = {
            {-1, 3, -3, 1}, {2, -4, 2, 0}, {-1, 0, 1, 0}};
        vector<vector<float>> m3(vertices.begin() + begin,
                                 vertices.begin() + begin + 4);
        vector<vector<float>> res = mul(mul(m1, m2), m3);
        return {res[0][0], res[0][1], res[0][2]};
    }

    vector<float> eval_second(long long tll, long long Tll) {
        int begin = (tll / Tll) % (vertices.size() - 3);
        float t = (tll % Tll) / float(Tll);

        vector<vector<float>> m1 =
            vector<vector<float>>{{2 * t / 2.0f, 1 / 2.0f}};
        vector<vector<float>> m2 = {{-1, 3, -3, 1}, {2, -4, 2, 0}};
        vector<vector<float>> m3(vertices.begin() + begin,
                                 vertices.begin() + begin + 4);
        vector<vector<float>> res = mul(mul(m1, m2), m3);
        return {res[0][0], res[0][1], res[0][2]};
    };
};
#endif