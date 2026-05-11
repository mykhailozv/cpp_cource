#include "telemetry.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>

// Debugging exercise notes:
// this file intentionally contains four runtime defects.
// The defects are related to malformed input shape, invalid numeric values,
// unsafe time deltas, and empty logs. Exact locations are not marked on purpose.

const int EXPECTED_FIELD_COUNT = 7;
const int MAX_LINE_LENGTH = 256;
const double EPS = 0.00001;

bool split_line_into_fields(char line[], char* fields[], int& count, int max_fields) {
    count = 0;
    char* cursor = line;

    while (*cursor != '\0') {
        while (*cursor == ' ' || *cursor == '\t' || *cursor == '\n' || *cursor == '\r') {
            *cursor = '\0';
            ++cursor;
        }

        if (*cursor == '\0') {
            break;
        }

        if (count >= max_fields) {
            return false;
        }

        fields[count] = cursor;
        ++count;

        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t' && *cursor != '\n' &&
               *cursor != '\r') {
            ++cursor;
        }
    }

    return count == max_fields;
}

bool parse_long(const char* text, long& value) {
    char* end = nullptr;
    value = std::strtol(text, &end, 10);

    if (end == text || *end != '\0') {
        std::cerr << "Error: Invalid numeric value\n";
        return false;
    }

    return true;
}

bool parse_int(const char* text, int& value) {
    long parsed_value = 0;

    if(!parse_long(text, parsed_value)){
        return false;
    }
    
    value = static_cast<int>(parsed_value);

    return true;
}

bool parse_double(const char* text, double& value) {
    char* end = nullptr;
    value = std::strtod(text, &end);

    if (end == text || *end != '\0') {
        std::cerr << "Error: Invalid numeric value\n";
        return false;
    }

    return true;
}

bool parse_frame(Frame& frame, char line[], int frame_count) {
    char* fields[EXPECTED_FIELD_COUNT] = {};
    int field_count;
    const bool is_valid_split = split_line_into_fields(line, fields, field_count, EXPECTED_FIELD_COUNT);
    
    if (!is_valid_split) {
        std::cerr << "Error: Invalid frame at line " << frame_count << ": expected " << EXPECTED_FIELD_COUNT << " fields\n";
        return false;
    }

    if(!parse_long(fields[0], frame.timestamp_ms)){
        return false;
    }

    if(!parse_int(fields[1], frame.seq)){
        return false;
    }

    if(!parse_double(fields[2], frame.voltage_v)){
        return false;
    }

    if(!parse_double(fields[3], frame.current_a)){
        return false;
    }

    if(!parse_double(fields[4], frame.temperature_c)){
        return false;
    }
    
    if(!parse_int(fields[5], frame.gps_fix)){
        return false;
    }

    if(!parse_int(fields[6], frame.satellites)){
        return false;
    }

    if (frame.voltage_v < 0 + EPS) {
        std::cerr << "Error: Invalid voltage_v: " << frame.voltage_v << std::endl;
        return false;
    }

    if (frame.temperature_c < -40.0 - EPS || frame.temperature_c > 120.0 + EPS) {
        std::cerr << "Error: Invalid temperature_c: " << frame.temperature_c << std::endl;
        return false;
    }

    if (frame.gps_fix != 0 && frame.gps_fix != 1) {
        std::cerr << "Error: Invalid gps_fix: " << frame.gps_fix << std::endl;
        return false;
    }

    if (frame.satellites < 0) {
        std::cerr << "Error: Invalid satellites: " << frame.satellites << std::endl;
        return false;
    }

    return true;
}

double compute_frame_rate_hz(const Frame frames[], int frame_count) {
    const long elapsed_ms = frames[frame_count - 1].timestamp_ms - frames[0].timestamp_ms;

    return static_cast<double>((frame_count - 1) * 1000 / elapsed_ms);
}

bool read_frames(const char* path, int& frame_count, Frame frames[], int max_frames) {
    std::ifstream input{path};

    if (!input) {
        std::cerr << "error: failed to open input file: " << path << '\n';
        return false;
    }

    frame_count = 0;
    char line[MAX_LINE_LENGTH];
    bool is_valid_frame;

    while (input.getline(line, MAX_LINE_LENGTH)) {
        if (line[0] == '\0') {
            continue;
        }

        if (frame_count < max_frames) {
            is_valid_frame = parse_frame(frames[frame_count], line, frame_count);

            if (!is_valid_frame) {
                return false;
            }

            if(frame_count > 0){
                if(frames[frame_count].timestamp_ms <= frames[frame_count - 1].timestamp_ms){
                    std::cerr << "Error: Invalid timestamp_ms: must be strictly increasing" << std::endl;
                    return false;
                }
                
                if(frames[frame_count].seq - frames[frame_count - 1].seq != 1){
                    std::cerr << "Error: Invalid seq: expected increment by 1 \n";
                    return false;
                }
            }

            ++frame_count;
        }
    }

    if(frame_count < 1){
        std::cerr << "Error: empty file\n";
        return false;
    }

    return true;
}

Summary summarize(const Frame frames[], int frame_count) {
    Summary summary{};
    summary.frames_total = frame_count;
    summary.frames_valid = frame_count;
    summary.voltage_min = frames[0].voltage_v;
    summary.voltage_max = frames[0].voltage_v;
    summary.low_voltage_frames = 0;

    double temperature_sum = 0.0;

    for (int i = 0; i < frame_count; ++i) {
        if (frames[i].voltage_v < summary.voltage_min) {
            summary.voltage_min = frames[i].voltage_v;
        }

        if (frames[i].voltage_v > summary.voltage_max) {
            summary.voltage_max = frames[i].voltage_v;
        }

        temperature_sum += frames[i].temperature_c;

        if (frames[i].voltage_v < 22.0) {
            ++summary.low_voltage_frames;
        }
    }

    const int temperature_tenths = static_cast<int>(temperature_sum * 10.0) / frame_count;
    summary.temperature_avg = static_cast<double>(temperature_tenths) / 10.0;
    summary.frame_rate_hz = compute_frame_rate_hz(frames, frame_count);
    return summary;
}

void print_summary(const Summary& summary) {
    std::cout << "frames_total " << summary.frames_total << '\n';
    std::cout << "frames_valid " << summary.frames_valid << '\n';
    std::cout << "voltage_min " << summary.voltage_min << '\n';
    std::cout << "voltage_max " << summary.voltage_max << '\n';
    std::cout << "temperature_avg " << summary.temperature_avg << '\n';
    std::cout << "low_voltage_frames " << summary.low_voltage_frames << '\n';
    std::cout << "frame_rate_hz " << summary.frame_rate_hz << '\n';
}
