#pragma once

#include <string>
#include <map>
#include <chrono>
#include <vector>
#include <memory>

struct PhaseProfile {
    std::string name;
    double total_time;
    double average_time;
    size_t call_count;
    size_t memory_usage;
    std::vector<double> execution_times;
    
    PhaseProfile() : total_time(0.0), average_time(0.0), call_count(0), memory_usage(0) {}
};

class CompilerProfiler {
private:
    std::map<std::string, PhaseProfile> phase_profiles;
    std::map<std::string, std::chrono::high_resolution_clock::time_point> phase_start_times;
    std::map<std::string, size_t> memory_usage;
    
    bool profiling_enabled;
    std::chrono::high_resolution_clock::time_point total_start_time;
    std::chrono::high_resolution_clock::time_point total_end_time;
    
    // Memory tracking
    size_t peak_memory_usage;
    size_t current_memory_usage;
    
    // Performance counters
    std::map<std::string, size_t> performance_counters;
    
public:
    CompilerProfiler();
    ~CompilerProfiler() = default;
    
    // Profiling control
    void enable_profiling(bool enable = true);
    bool is_profiling_enabled() const;
    
    // Phase timing
    void start_phase(const std::string& phase_name);
    void end_phase(const std::string& phase_name);
    
    // Memory tracking
    void record_memory_usage(const std::string& phase_name, size_t memory_bytes);
    void update_peak_memory();
    size_t get_peak_memory_usage() const;
    size_t get_current_memory_usage() const;
    
    // Performance counters
    void increment_counter(const std::string& counter_name);
    void set_counter(const std::string& counter_name, size_t value);
    size_t get_counter(const std::string& counter_name) const;
    
    // Report generation
    void generate_performance_report() const;
    void generate_detailed_report(const std::string& output_file) const;
    void generate_json_report(const std::string& output_file) const;
    void generate_csv_report(const std::string& output_file) const;
    
    // Statistics
    double get_total_compilation_time() const;
    double get_phase_time(const std::string& phase_name) const;
    double get_phase_percentage(const std::string& phase_name) const;
    std::vector<std::string> get_slowest_phases(int count = 5) const;
    
    // Optimization suggestions
    std::vector<std::string> get_optimization_suggestions() const;
    
    // Reset and cleanup
    void reset();
    void clear_counters();
    
    // Utility methods
    std::string format_time(double seconds) const;
    std::string format_memory(size_t bytes) const;
    void print_phase_summary() const;
};
