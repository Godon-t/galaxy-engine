#include "TimeLogger.hpp"

#include "Log.hpp"

namespace Galaxy {
GlobalTimer& GlobalTimer::getInstance()
{
    static GlobalTimer instance;
    return instance;
}
void GlobalTimer::logTime(const std::string& name, long long duration)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto& data = times[name];
    data.first += duration;
    data.second += 1;
}
void GlobalTimer::printAverages() const
{
    std::lock_guard<std::mutex> lock(mutex);
    auto orderingFunction = [](const std::pair<std::string, std::pair<long long, long long>>& a, const std::pair<std::string, std::pair<long long, long long>>& b) {
        return a.second.first > b.second.first;
    };
    auto timesVector = std::vector<std::pair<std::string, std::pair<long long, long long>>>(times.begin(), times.end());
    sort(timesVector.begin(), timesVector.end(), orderingFunction);
    GLX_CORE_INFO("Ordered from the highest time consuming to the lowest:")
    for (const auto& entry : timesVector) {
        const auto& name        = entry.first;
        const auto& data        = entry.second;
        long long totalDuration = data.first;
        long long count         = data.second;

        GLX_CORE_INFO("{0} - average time: {1} ms \n{0} - total call : {2}\n{0} - total time : {3} ms", name, ((long double)totalDuration / (long double)count), count, totalDuration);
    }
}
ScopedTimer::ScopedTimer(const std::string& name)
    : name(name)
    , start(std::chrono::high_resolution_clock::now())
{
}
ScopedTimer::~ScopedTimer()
{
    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    GlobalTimer::getInstance().logTime(name, duration);
}
} // namespace Galaxy
