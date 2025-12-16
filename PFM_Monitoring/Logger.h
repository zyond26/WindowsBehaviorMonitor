#pragma once
#pragma once
#include <string>
#include <mutex>

class Logger {
public:
    static Logger& Instance();

    void Info(const std::wstring& msg);
    void Warn(const std::wstring& msg);
    void Error(const std::wstring& msg);

    void SetLogFile(const std::wstring& path);

private:
    Logger() = default;
    ~Logger();

    std::mutex mtx_;
    std::wstring logfile_;
    void Write(const std::wstring& level, const std::wstring& msg);
};
