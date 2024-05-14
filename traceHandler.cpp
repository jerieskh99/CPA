//
// Created by Jeries Khoury on 09/05/2024.
//

#include "traceHandler.h"

traceHandler::traceHandler(vector<vector<float>> &trace_data, vector<vector<float>> &clock_data, int range_start, int range_end)
                                            :m_trace_data(trace_data), m_clock_data(clock_data),
                                            m_range_start(range_start), m_range_end(range_end){


    int line_num = (int)m_clock_data.size();

    for(int i=0; i<line_num; i++){
        vector<int> tmp;
        m_clock_toggle_indices.push_back(tmp);
    }

    int line_index = 0;
    int line_number = (int) m_clock_data.size();

    while(line_index < line_number) {
        int line_size = (int) m_clock_data[line_index].size();
        vector<float> &clk_line = m_clock_data[line_index];

        for (int sample_index = m_range_start; sample_index < m_range_end; sample_index++) {
            float curr_sample = clk_line[sample_index];
            if (curr_sample < 0.9 && curr_sample > 0.1) {
                m_clock_toggle_indices[line_index].push_back(sample_index);
            }
        }
        line_index ++;
    }

    // MAKE BINARY //

    for(int i=0; i<line_num; i++){
        vector<float> tmp;
        m_binary_clock.push_back(tmp);
    }

    for(int clk_line = 0; clk_line < line_num; clk_line ++){
        vector<float>& curr_clk_line = m_clock_data[clk_line];
        int clk_line_size = (int)curr_clk_line.size();
        for(int sample_index = 0; sample_index < clk_line_size; sample_index ++){
            float curr_sample = m_clock_data[clk_line][sample_index];
            if(curr_sample > 0.7){
                m_binary_clock[clk_line].push_back(1.0);
            }else if(curr_sample < 0.3){
                m_binary_clock[clk_line].push_back(0.0);
            }else{
                m_binary_clock[clk_line].push_back(m_binary_clock[clk_line][sample_index-1]); // keep the previous val.
            }
        }
    }

    // FILTER EDGES //

    for(int i=0; i<line_num; i++){
        vector<int> tmp;
        m_edge_indices.push_back(tmp);
    }

    for(int clk_line = 0; clk_line < line_num; clk_line ++){
        vector<float>& curr_clk_line = m_binary_clock[clk_line];
        int clk_line_size = (int)curr_clk_line.size();
        for(int sample_index = 1; sample_index < clk_line_size; sample_index ++){
            if(m_binary_clock[clk_line][sample_index] != m_binary_clock[clk_line][sample_index - 1]){
                m_edge_indices[clk_line].push_back(sample_index);
            }
        }
    }

//        int sample_index = 0;
//        while(sample_index < line_size){
//            float curr_sample = clk_line[sample_index];
//            if(curr_sample < 0.1) {
//                m_clock_toggle_indices[line_index].push_back(sample_index);
//                while((curr_sample < 0.1) && (sample_index < line_size)){
//                    sample_index++;
//                }
//                m_clock_toggle_indices[line_index].push_back(sample_index-1);
//            }else{
//                sample_index++;
//            }
//
//        }
//        line_index ++;
}


vector<vector<float>> traceHandler::getClockedTrace() {

    vector<vector<float>> trimmed_trace;
    int line_num = (int)m_trace_data.size();

    for(int i=0; i<line_num; i++){
        vector<float> tmp;
        trimmed_trace.push_back(tmp);
    }

    int line_index = 0;
    for(auto& clk_line: m_clock_toggle_indices){
        int line_size = (int) clk_line.size();
        for(int sample_index = 0; sample_index < line_size; sample_index ++){
            auto& tmp = trimmed_trace[line_index];
            tmp.push_back(m_trace_data[line_index][sample_index]);
        }
        line_index ++;
    }
    m_trimmed_trace = trimmed_trace;
    return m_trimmed_trace;
}

int traceHandler::get_minimal_trace_len() {
    if(m_trimmed_trace.empty()){
        std::cout << "Trim the trace first" << std::endl;
    }
    int min_len = (int) m_trimmed_trace[0].size();
    int min_index = 0;
    int num_lines = (int) m_trimmed_trace.size();

    for(int line = 0; line < num_lines; line++){
        int curr = (int) m_trimmed_trace[line].size();

        if(curr < min_len){
            min_len = curr;
            min_index = line;
        }
    }

    return min_len;
}



std::vector<float> interpolate(const std::vector<float>& segment, size_t new_size) {
    std::vector<float> new_segment(new_size);
    double scale = double(segment.size() - 1) / ((int)new_size - 1);
    for (size_t i = 0; i < new_size; ++i) {
        double index = (int)i * scale;
        int lower = std::floor(index);
        int upper = std::ceil(index);
        double t = index - lower;
        if (upper >= segment.size()) upper = lower; // Prevents accessing out of bounds
        new_segment[i] = segment[lower] + t * (segment[upper] - segment[lower]);
    }
    return new_segment;
}


// Function to normalize segments for a single trace
std::vector<std::vector<float>> normalize_trace_segments(
        const std::vector<float>& trace,
        const std::vector<int>& edges,
        int standard_length) {

    std::vector<std::vector<float>> normalized_segments;

    for (size_t i = 0; i < edges.size() - 1; ++i) {
        int start = edges[i];
        int end = edges[i + 1];
        std::vector<float> segment(trace.begin() + start, trace.begin() + end);

        if (segment.size() > standard_length) {
            // Downsample by averaging
            segment = interpolate(segment, standard_length); // Using interpolation for downsizing too
        } else if (segment.size() < standard_length) {
            // Interpolate to increase length
            segment = interpolate(segment, standard_length);
        }
        normalized_segments.push_back(segment);
    }
    return normalized_segments;
}


std::vector<std::vector<float>> flatten_traces(const std::vector<std::vector<std::vector<float>>>& all_normalized_traces) {
    std::vector<std::vector<float>> flattened_traces;

    // Iterate over all traces
    for (const auto& trace_segments : all_normalized_traces) {
        std::vector<float> flattened_trace;

        // Concatenate all segments into a single trace
        for (const auto& segment : trace_segments) {
            flattened_trace.insert(flattened_trace.end(), segment.begin(), segment.end());
        }

        // Add the flattened trace to the collection of all traces
        flattened_traces.push_back(flattened_trace);
    }

    return flattened_traces;
}


float calculate_average_distance_between_edges(const std::vector<std::vector<int>>& edges_per_trace) {
    int total_sum_distances = 0;
    int total_count = 0;

    // Iterate over each trace's edges
    for (const auto& edges : edges_per_trace) {
        int sum_distances = 0;
        int count = 0;

        // Iterate through the edges, calculate distances from falling (even index) to rising (odd index)
        for (size_t i = 0; i < edges.size() - 1; i += 2) {
            if (i + 1 < edges.size()) { // Make sure there is a pair to calculate
                int distance = edges[i + 1] - edges[i];
                sum_distances += distance;
                ++count;
            }
        }

        // Update overall totals
        total_sum_distances += sum_distances;
        total_count += count;

//        // Optional: Print average for each trace
//        if (count > 0) {
//            float average_distance = static_cast<float>(sum_distances) / (float)count;
//            std::cout << "Average distance between falling and rising edges for a trace: " << average_distance << std::endl;
//        }
    }

    // Calculate and return the overall average distance
    return total_count > 0 ? static_cast<float>(total_sum_distances) / (float)total_count : 0;
}


vector<vector<float>> traceHandler::align_traces() {
    float standard_seg_size = calculate_average_distance_between_edges(m_edge_indices);
    int standard_seg_size_int = std::floor(standard_seg_size);
    vector<vector<vector<float>>> aligned_trances_chunked;

    for(int line = 0; line < m_trace_data.size(); line++){
        vector<float>& curr_tr = m_trace_data[line];
        vector<int>& curr_edge = m_edge_indices[line];
        vector<vector<float>> curr_seg = normalize_trace_segments(curr_tr, curr_edge, standard_seg_size_int);
        aligned_trances_chunked.push_back(curr_seg);
    }

    m_trace_aligned = flatten_traces(aligned_trances_chunked);
    std::cout<< "the dimensions of our new traces are: [" <<m_trace_aligned.size()
                                                          << " , " << m_trace_aligned[0].size()
                                                          <<"]" << std::endl;
    return m_trace_aligned;
}