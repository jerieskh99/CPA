//
// Created by Jeries Khoury on 08/05/2024.
//

#ifndef HSEC_CPA_H
#define HSEC_CPA_H
#include <bitset>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <stdexcept> // For std::out_of_range
#include <tuple>




#include <vector>

using std::vector;
using std::tuple;


class CPA {
private:
    vector<vector<float>> m_trace;
    // assuming that a single column is extracted when constructing
    vector<uint8_t> m_plaintext; //each int represents one of the numbers ine each line (0-256)- size is 16
    int m_start;
    int m_end;
    vector<uint32_t> aes_sbox;
    vector<uint32_t> hamming_weights;


public:
    CPA(vector<vector<float>>& trace, vector<uint8_t>& plaintext, int start = 0, int end = 50000);

    tuple<int, float> execute();
    tuple<int, float> execute2();

    static double correlate(const std::vector<float>& x, const std::vector<uint32_t>& y);

};


#endif //HSEC_CPA_H
