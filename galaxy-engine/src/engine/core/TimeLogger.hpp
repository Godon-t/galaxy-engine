#include "pch.hpp"

namespace Galaxy {
class GlobalTimer {
public:
    static GlobalTimer& getInstance();
    void logTime(const std::string& name, long long duration);
    void printAverages() const;

private:
    GlobalTimer()                              = default;
    ~GlobalTimer()                             = default;
    GlobalTimer(const GlobalTimer&)            = delete;
    GlobalTimer& operator=(const GlobalTimer&) = delete;

    mutable std::mutex mutex;
    std::unordered_map<std::string, std::pair<long long, long long>> times;
};

class ScopedTimer {
public:
    ScopedTimer(const std::string& name);
    ~ScopedTimer();

private:
    std::string name;
    std::chrono::high_resolution_clock::time_point start;
};
} // namespace Galaxy
