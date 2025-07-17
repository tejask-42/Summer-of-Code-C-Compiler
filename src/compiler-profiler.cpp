#include "compiler-profiler.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <sstream>

CompilerProfiler::CompilerProfiler() 
    : profiling_enabled(false), peak_memory_usage(0), current_memory_usage(0) {}

void CompilerProfiler::enable_profiling(bool enable) {
    profiling_enabled = enable;
    if (enable) {
        total_start_time = std::chrono::high_resolution_clock::now();
    }
}

bool CompilerProfiler::is_profiling_enabled() const {
    return profiling_enabled;
}

void CompilerProfiler::start_phase(const std::string& phase_name) {
    if (!profiling_enabled) return;
    
    phase_start_times[phase_name] = std::chrono::high_resolution_clock::now();
    
    // Initialize phase profile if not exists
    if (phase_profiles.find(phase_name) == phase_profiles.end()) {
        phase_profiles[phase_name] = PhaseProfile();
        phase_profiles[phase_name].name = phase_name;
    }
}

void CompilerProfiler::end_phase(const std::string& phase_name) {
    if (!profiling_enabled) return;
    
    auto it = phase_start_times.find(phase_name);
    if (it == phase_start_times.end()) {
        std::cerr << "Warning: end_phase called for " << phase_name << " without start_phase" << std::endl;
        return;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(end_time - it->second);
    double elapsed_time = duration.count();
    
    // Update phase profile
    auto& profile = phase_profiles[phase_name];
    profile.total_time += elapsed_time;
    profile.call_count++;
    profile.average_time = profile.total_time / profile.call_count;
    profile.execution_times.push_back(elapsed_time);
    
    // Remove from start times
    phase_start_times.erase(it);
    
    // Update total end time
    total_end_time = end_time;
}

void CompilerProfiler::record_memory_usage(const std::string& phase_name, size_t memory_bytes) {
    if (!profiling_enabled) return;
    
    memory_usage[phase_name] = memory_bytes;
    current_memory_usage = memory_bytes;
    
    if (memory_bytes > peak_memory_usage) {
        peak_memory_usage = memory_bytes;
    }
    
    // Update phase profile
    if (phase_profiles.find(phase_name) != phase_profiles.end()) {
        phase_profiles[phase_name].memory_usage = memory_bytes;
    }
}

void CompilerProfiler::update_peak_memory() {
    // This would typically use system calls to get current memory usage
    // For now, we'll use a simple estimation
    current_memory_usage = 0;
    for (const auto& usage : memory_usage) {
        current_memory_usage += usage.second;
    }
    
    if (current_memory_usage > peak_memory_usage) {
        peak_memory_usage = current_memory_usage;
    }
}

size_t CompilerProfiler::get_peak_memory_usage() const {
    return peak_memory_usage;
}

size_t CompilerProfiler::get_current_memory_usage() const {
    return current_memory_usage;
}

void CompilerProfiler::increment_counter(const std::string& counter_name) {
    performance_counters[counter_name]++;
}

void CompilerProfiler::set_counter(const std::string& counter_name, size_t value) {
    performance_counters[counter_name] = value;
}

size_t CompilerProfiler::get_counter(const std::string& counter_name) const {
    auto it = performance_counters.find(counter_name);
    return (it != performance_counters.end()) ? it->second : 0;
}

void CompilerProfiler::generate_performance_report() const {
    if (!profiling_enabled) {
        std::cout << "Profiling is not enabled" << std::endl;
        return;
    }
    
    std::cout << "\n=== Compiler Performance Report ===" << std::endl;
    std::cout << "Total compilation time: " << format_time(get_total_compilation_time()) << std::endl;
    std::cout << "Peak memory usage: " << format_memory(peak_memory_usage) << std::endl;
    std::cout << std::endl;
    
    std::cout << "Phase Breakdown:" << std::endl;
    std::cout << std::setw(20) << "Phase" << std::setw(10) << "Time" << std::setw(12) << "Percentage" 
              << std::setw(8) << "Calls" << std::setw(12) << "Avg Time" << std::setw(10) << "Memory" << std::endl;
    std::cout << std::string(72, '-') << std::endl;
    
    double total_time = get_total_compilation_time();
    
    for (const auto& profile : phase_profiles) {
        const auto& p = profile.second;
        double percentage = (total_time > 0) ? (p.total_time / total_time) * 100.0 : 0.0;
        
        std::cout << std::setw(20) << p.name
                  << std::setw(10) << format_time(p.total_time)
                  << std::setw(11) << std::fixed << std::setprecision(1) << percentage << "%"
                  << std::setw(8) << p.call_count
                  << std::setw(12) << format_time(p.average_time)
                  << std::setw(10) << format_memory(p.memory_usage) << std::endl;
    }
    
    std::cout << std::string(72, '-') << std::endl;
    
    // Performance counters
    if (!performance_counters.empty()) {
        std::cout << "\nPerformance Counters:" << std::endl;
        for (const auto& counter : performance_counters) {
            std::cout << "  " << counter.first << ": " << counter.second << std::endl;
        }
    }
    
    // Optimization suggestions
    auto suggestions = get_optimization_suggestions();
    if (!suggestions.empty()) {
        std::cout << "\nOptimization Suggestions:" << std::endl;
        for (const auto& suggestion : suggestions) {
            std::cout << "  - " << suggestion << std::endl;
        }
    }
    
    std::cout << "===================================" << std::endl;
}

void CompilerProfiler::generate_detailed_report(const std::string& output_file) const {
    std::ofstream file(output_file);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open performance report file: " + output_file);
    }
    
    file << "C-- Compiler Performance Report\n";
    file << "Generated: " << std::chrono::system_clock::now().time_since_epoch().count() << "\n\n";
    
    file << "Summary:\n";
    file << "  Total compilation time: " << format_time(get_total_compilation_time()) << "\n";
    file << "  Peak memory usage: " << format_memory(peak_memory_usage) << "\n";
    file << "  Number of phases: " << phase_profiles.size() << "\n\n";
    
    file << "Phase Details:\n";
    for (const auto& profile : phase_profiles) {
        const auto& p = profile.second;
        file << "  Phase: " << p.name << "\n";
        file << "    Total time: " << format_time(p.total_time) << "\n";
        file << "    Average time: " << format_time(p.average_time) << "\n";
        file << "    Call count: " << p.call_count << "\n";
        file << "    Memory usage: " << format_memory(p.memory_usage) << "\n";
        file << "    Execution times: ";
        for (size_t i = 0; i < p.execution_times.size(); ++i) {
            if (i > 0) file << ", ";
            file << format_time(p.execution_times[i]);
        }
        file << "\n\n";
    }
    
    file << "Performance Counters:\n";
    for (const auto& counter : performance_counters) {
        file << "  " << counter.first << ": " << counter.second << "\n";
    }
    
    file.close();
}

void CompilerProfiler::generate_json_report(const std::string& output_file) const {
    std::ofstream file(output_file);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open JSON performance report file: " + output_file);
    }
    
    file << "{\n";
    file << "  \"performance_report\": {\n";
    file << "    \"total_compilation_time\": " << get_total_compilation_time() << ",\n";
    file << "    \"peak_memory_usage\": " << peak_memory_usage << ",\n";
    file << "    \"phases\": [\n";
    
    bool first = true;
    for (const auto& profile : phase_profiles) {
        if (!first) file << ",\n";
        first = false;
        
        const auto& p = profile.second;
        file << "      {\n";
        file << "        \"name\": \"" << p.name << "\",\n";
        file << "        \"total_time\": " << p.total_time << ",\n";
        file << "        \"average_time\": " << p.average_time << ",\n";
        file << "        \"call_count\": " << p.call_count << ",\n";
        file << "        \"memory_usage\": " << p.memory_usage << ",\n";
        file << "        \"execution_times\": [";
        for (size_t i = 0; i < p.execution_times.size(); ++i) {
            if (i > 0) file << ", ";
            file << p.execution_times[i];
        }
        file << "]\n";
        file << "      }";
    }
    
    file << "\n    ],\n";
    file << "    \"performance_counters\": {\n";
    
    first = true;
    for (const auto& counter : performance_counters) {
        if (!first) file << ",\n";
        first = false;
        file << "      \"" << counter.first << "\": " << counter.second;
    }
    
    file << "\n    }\n";
    file << "  }\n";
    file << "}\n";
    
    file.close();
}

double CompilerProfiler::get_total_compilation_time() const {
    if (total_start_time.time_since_epoch().count() == 0) return 0.0;
    return std::chrono::duration<double>(total_end_time - total_start_time).count();
}

double CompilerProfiler::get_phase_time(const std::string& phase_name) const {
    auto it = phase_profiles.find(phase_name);
    return (it != phase_profiles.end()) ? it->second.total_time : 0.0;
}

double CompilerProfiler::get_phase_percentage(const std::string& phase_name) const {
    double total_time = get_total_compilation_time();
    if (total_time == 0.0) return 0.0;
    
    double phase_time = get_phase_time(phase_name);
    return (phase_time / total_time) * 100.0;
}

std::vector<std::string> CompilerProfiler::get_slowest_phases(int count) const {
    std::vector<std::pair<std::string, double>> phase_times;
    
    for (const auto& profile : phase_profiles) {
        phase_times.push_back({profile.first, profile.second.total_time});
    }
    
    std::sort(phase_times.begin(), phase_times.end(),
              [](const std::pair<std::string, double>& a, const std::pair<std::string, double>& b) {
                  return a.second > b.second;
              });
    
    std::vector<std::string> slowest_phases;
    for (int i = 0; i < count && i < static_cast<int>(phase_times.size()); ++i) {
        slowest_phases.push_back(phase_times[i].first);
    }
    
    return slowest_phases;
}

std::vector<std::string> CompilerProfiler::get_optimization_suggestions() const {
    std::vector<std::string> suggestions;
    
    // Analyze performance and suggest optimizations
    auto slowest_phases = get_slowest_phases(3);
    
    for (const auto& phase : slowest_phases) {
        double percentage = get_phase_percentage(phase);
        
        if (percentage > 30.0) {
            suggestions.push_back("Consider optimizing " + phase + " phase (takes " + 
                                std::to_string(static_cast<int>(percentage)) + "% of compilation time)");
        }
    }
    
    if (peak_memory_usage > 100 * 1024 * 1024) { // 100MB
        suggestions.push_back("High memory usage detected. Consider implementing memory pooling or reducing intermediate data structures.");
    }
    
    // Check for frequently called phases
    for (const auto& profile : phase_profiles) {
        if (profile.second.call_count > 1000) {
            suggestions.push_back("Phase " + profile.first + " is called frequently (" + 
                                std::to_string(profile.second.call_count) + " times). Consider caching or batching.");
        }
    }
    
    return suggestions;
}

void CompilerProfiler::reset() {
    phase_profiles.clear();
    phase_start_times.clear();
    memory_usage.clear();
    performance_counters.clear();
    peak_memory_usage = 0;
    current_memory_usage = 0;
    total_start_time = std::chrono::high_resolution_clock::time_point();
    total_end_time = std::chrono::high_resolution_clock::time_point();
}

void CompilerProfiler::clear_counters() {
    performance_counters.clear();
}

std::string CompilerProfiler::format_time(double seconds) const {
    if (seconds < 0.001) {
        return std::to_string(static_cast<int>(seconds * 1000000)) + "μs";
    } else if (seconds < 1.0) {
        return std::to_string(static_cast<int>(seconds * 1000)) + "ms";
    } else {
        return std::to_string(seconds) + "s";
    }
}

std::string CompilerProfiler::format_memory(size_t bytes) const {
    if (bytes < 1024) {
        return std::to_string(bytes) + "B";
    } else if (bytes < 1024 * 1024) {
        return std::to_string(bytes / 1024) + "KB";
    } else {
        return std::to_string(bytes / (1024 * 1024)) + "MB";
    }
}

void CompilerProfiler::print_phase_summary() const {
    std::cout << "Phase Summary:" << std::endl;
    for (const auto& profile : phase_profiles) {
        const auto& p = profile.second;
        std::cout << "  " << p.name << ": " << format_time(p.total_time) 
                  << " (" << p.call_count << " calls)" << std::endl;
    }
}
