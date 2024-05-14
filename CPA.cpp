//
// Created by Jeries Khoury on 08/05/2024.
//

#include "CPA.h"

CPA::CPA(vector<vector<float>> &trace, vector<uint8_t> &plaintext, int start, int end): m_trace(trace),
                                                                                        m_plaintext(plaintext), m_start(start), m_end(end), aes_sbox(){

    aes_sbox = {
            0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
            0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
            0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
            0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
            0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
            0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
            0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
            0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
            0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
            0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
            0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
            0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
            0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
            0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
            0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
            0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
    };

    //std::cout << "hamming" << std::endl;
    for(int i=0; i<256; i++){
        unsigned long i_weight = std::bitset<32>(i).count();
        hamming_weights.push_back(i_weight);
        //std::cout << "["<< i << " , " << i_weight << "] ";
    }
}

template<typename T>
std::vector<T> extractColumn(const std::vector<std::vector<T>>& matrix, size_t columnIndex) {
    std::vector<T> column;
    for (const auto& row : matrix) {
        if (columnIndex >= row.size()) {
            throw std::out_of_range("Column index is out of range.");
        }
        column.push_back(row[columnIndex]);
    }
    return column;
}

tuple<int, float> CPA::execute() {
    size_t num_traces = m_trace.size();
    int best_guess = 0;
    float best_correlation = 0.0;
    vector<double> max_correlation(256,0);

    for(int key_guess = 0; key_guess < 256; key_guess ++){
        vector<uint32_t> hypothetical_power(150,0);
        int index = 0;
        for(auto& pt: m_plaintext){
            uint32_t  curr_sbox_val = aes_sbox[key_guess ^ pt];
            uint32_t  curr_hamming_val = hamming_weights[curr_sbox_val];
            hypothetical_power[index] = curr_hamming_val;
            index ++;
        }

        vector<double> correlations;
        // Addded the -1 to not get SIGSEGV
        for(int i=m_start; i< (m_end-1); i++) {
            vector<float> trace_col = extractColumn(m_trace, i);
            double corr = correlate(trace_col, hypothetical_power);
            correlations.push_back(corr);
        }

        double max = std::abs(correlations[0]);
        int max_index = 0;
        for(int i=0; i<correlations.size(); i++){
            if(std::abs(correlations[i]) > std::abs(max)){
                max = std::abs(correlations[i]);
                max_index = i;
            }
        }

        if (max > best_correlation)
        {
            best_correlation = max;
            best_guess = key_guess;
        }
    }

    auto tup = tuple(best_guess, best_correlation);
    std::cout << "got the tuple: " << "best guess: " << best_guess << " best corr:" << best_correlation << std::endl;
    return tup;
}



// make sure it's ok with the uint8
double CPA::correlate(const vector<float> &x, const vector<uint32_t> &y) {
    if (x.size() != y.size() || x.empty()) {
        if(x.empty()){
            std::cout <<"x empty " << std::endl;
        }
        std::cout <<"size not matching" << std::endl;
        return 0; // Return 0 if sizes do not match or vectors are empty
    }

    int size = static_cast<int>(x.size());
    double sum_x = std::accumulate(x.begin(), x.end(), 0.0);
    double sum_y = std::accumulate(y.begin(), y.end(), 0.0);

    double mean_x = sum_x / size;
    double mean_y = sum_y / size;

    double stddev_x = 0.0;
    double stddev_y = 0.0;
    double covariance = 0.0;

    for (int i = 0; i < size; ++i) {
        double dx = x[i] - mean_x;  // Difference from mean for x
        double dy = static_cast<double>(y[i]) - mean_y;  // Difference from mean for y
        stddev_x += dx * dx;
        stddev_y += dy * dy;
        covariance += dx * dy;
    }

    stddev_x = std::sqrt(stddev_x / size);
    stddev_y = std::sqrt(stddev_y / size);
    covariance /= size;

    if (stddev_x == 0 || stddev_y == 0) {
        return 0; // Return 0 if any standard deviation is zero
    }

    return covariance / (stddev_x * stddev_y); // Pearson correlation coefficient
}
