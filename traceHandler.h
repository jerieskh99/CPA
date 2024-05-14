//
// Created by Jeries Khoury on 09/05/2024.
//

#ifndef HSEC_TRACEHANDLER_H
#define HSEC_TRACEHANDLER_H

#include <vector>
#include <iostream>

using std::vector;


class traceHandler {
private:
    vector<vector<float>> m_trace_data;
    vector<vector<float>> m_clock_data;

    vector<vector<int>> m_clock_rise_indices;
    vector<vector<int>> m_clock_fall_indices;
    vector<vector<int>> m_clock_toggle_indices;

    vector<vector<float>> m_trimmed_trace;
    vector<vector<float>> m_binary_clock;

    vector<vector<int>> m_edge_indices;

    vector<vector<float>> m_trace_aligned;

    int m_range_start;
    int m_range_end;

public:
    traceHandler(vector<vector<float>>& trace_data, vector<vector<float>>& m_clock_data,
                 int range_start = 0, int range_end = 0 );

    vector<vector<float>> getClockedTrace();

    int get_minimal_trace_len();

    vector<vector<float>> align_traces();

};


#endif //HSEC_TRACEHANDLER_H
