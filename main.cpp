#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include "CPA.h"
#include "traceHandler.h"
#include <chrono>

using std::vector;

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


int main() {
    const std::string base_dir = "/Users/jeries/Downloads/dataset2/";  // Ensure this path is correctly specified
    const int num_files = 16;
    const int num_clear_texts = 16;
    const int num_lines = 150;
    const int num_values_per_line = 50000;

    vector<int> key_bytes;
    vector<float> key_corr;
    int sum_key = 0;
    // Define the start and end points
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_files; ++i) {
        // ---------- //
        // OPEN FILES //
        // ---------- //

        // create file name strings:
        std::string trace_filename = base_dir + "trace" + std::to_string(i) + ".txt";
        //std::string trace_filename = "trace" + std::to_string(i) + ".txt";
        std::string clock_filename = base_dir + "clock" + std::to_string(i) + ".txt";

        std::string cleartext_filename = base_dir + "cleartext.txt";

        // open files
        std::ifstream trace_file(trace_filename);
        if (!trace_file) {
            std::cerr << "Failed to open files: " << trace_filename << std::endl;
            return 1;
        }
        std::ifstream clock_file(clock_filename);
        if (!clock_file) {
            std::cerr << "Failed to open files: " << clock_filename << std::endl;
            return 1;
        }
        std::ifstream cleartext_file(cleartext_filename);
        if (!cleartext_file) {
            std::cerr << "Failed to open files: " << cleartext_filename << std::endl;
            return 1;
        }

        // get data from files
        std::vector<std::vector<float>> trace_data(num_lines, std::vector<float>(num_values_per_line));

        std::vector<std::vector<float>> clock_data(num_lines, std::vector<float>(num_values_per_line));
        std::vector<std::vector<uint8_t>> clear_texts(num_lines, std::vector<uint8_t>(num_clear_texts));

        if (!trace_file || !clock_file ||!cleartext_file) {
            std::cerr << "Failed to open files: " << trace_filename << " or " << clock_filename << std::endl;
            return 1;
        }

        std::string line;
        int line_count = 0;
        while (std::getline(cleartext_file, line) && line_count < 150) {
            std::istringstream iss(line);
            uint32_t value;
            bool error = false;

            for (int num_count = 0; num_count < 16; ++num_count) {
                if (!(iss >> value) || value > 255) {
                    std::cerr << "Error processing data at line " << line_count + 1
                              << ", position " << num_count + 1 << std::endl;
                    error = true;
                    break;
                }
                clear_texts[line_count][num_count] = static_cast<uint8_t>(value);
            }

            // Check if there was an error or not enough data on the line
            if (error || iss >> value) { // attempt to read one more to check excess data
                std::cerr << "Incorrect number of values on line " << line_count + 1 << std::endl;
                return 1;
            }

            line_count++;
        }

        if (line_count != 150) {
            std::cerr << "Incorrect number of lines in file, expected 150 lines." << std::endl;
            return 1;
        }

        for (int line_no = 0; line_no < num_lines; ++line_no) {
            if (std::getline(trace_file, line)) {
                std::istringstream iss(line);
                for (int value_no = 0; value_no < num_values_per_line; ++value_no) {
                    if (!(iss >> trace_data[line_no][value_no])) {
                        std::cerr << "Error processing trace file data" << std::endl;
                        return 1;
                    }
                }
            }

            if (std::getline(clock_file, line)) {
                std::istringstream iss(line);
                for (int value_no = 0; value_no < num_values_per_line; ++value_no) {
                    if (!(iss >> clock_data[line_no][value_no])) {
                        std::cerr << "Error processing clock file data" << std::endl;
                        return 1;
                    }
                }
            }
        }

        // DIM CHECKS:
        std::cout << "trace dims are: [" << trace_data.size()  << " , " << trace_data[0].size()  << "]" << std::endl;
        std::cout << "clock dims are: [" << clock_data.size()  << " , " << clock_data[0].size()  << "]" << std::endl;
        std::cout << "plain dims are: [" << clear_texts.size() << " , " << clear_texts[0].size() << "]" << std::endl;

        // ------------- //
        // MANAGE TRACES //
        // ------------- //
        traceHandler tHandler(trace_data, clock_data, 0, 50000);
        //vector<vector<float>> trimmed_trace = tHandler.getClockedTrace();
        //int minimal_trace_length = tHandler.get_minimal_trace_len();
        vector<vector<float>> trimmed_trace = tHandler.align_traces();

        // ---------------//
        // EXECUTE ATTACK //
        // -------------- //
        vector<uint8_t> col = extractColumn(clear_texts, (size_t)i);

//        for(auto& elem: col){
//            std::cout << std::bitset<8>(elem) << ", ";
//        }
//        std::cout << std::endl;
        // DIM CHECKS:
        std::cout << "clear text column number " << i << " dims are: [" << col.size()  << "]" << std::endl;

        CPA cpa(trimmed_trace, col, 29250, 29500);

        tuple<int, float> key_byte_tuple = cpa.execute();
        int curr_key_byte = std::get<0>(key_byte_tuple);
        float curr_key_corr = std::get<1>(key_byte_tuple);
        sum_key += curr_key_byte;

        key_bytes.push_back(curr_key_byte);
        key_corr.push_back(curr_key_corr);
        std::cout << "Byte" << i <<": Key Guess = " << curr_key_byte << "Correlation = " << curr_key_corr << std::endl;

        trace_file.close();
        clock_file.close();
        cleartext_file.close();
    }

    // Define the start and end points
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate the duration
    std::chrono::duration<double> duration = end - start;

    // --------- //
    // CHECK KEY //
    // --------- //

    if (sum_key == 1434){
        std::cout<<"Checksum is correct - 1434 - "<<"YOU GOT THE KEY in "<< duration.count() << "seconds!"<<std::endl;
        int byte = 0;
        for (auto& b: key_bytes){
            std::cout << "Key byte "<< byte <<" is " << b << " - estimated correlation is: "
            << key_corr[byte++] <<std::endl;
        }
    }else{
        std::cout<<"Checksum is not correct"<<std::endl;
    }

    return 0;
}


