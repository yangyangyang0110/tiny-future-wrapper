/* Proj: InferenceApi
 * File: logger.hpp
 * Created Date: 2022/12/21
 * Author: yangyangyang
 * Description:
 * -----
 * Last Modified: 2022/12/21 15:54:15
 * -----
 * Copyright (c) 2022  . All rights reserved.
 */
#ifndef INFERENCEAPI_LOGGER_HPP
#define INFERENCEAPI_LOGGER_HPP

#include <spdlog/common.h>
#include <spdlog/details/circular_q.h>
#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/os.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <optional>
#include <regex>

#include "./os.hpp"
#include <chrono>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#ifndef OS_SEP
#ifdef __MSC_VER__
#define OS_SEP '\\'
#else
#define OS_SEP '/'
#endif
#endif

#define FILENAME_ (strrchr(__FILE__, OS_SEP) ? strrchr(__FILE__, OS_SEP) + 1 : __FILE__)

#define LOG_TRACE_WITH_LOGGER(logger, ...) \
    { SPDLOG_LOGGER_TRACE(logger, __VA_ARGS__); }
#define LOG_DEBUG_WITH_LOGGER(logger, ...) \
    { SPDLOG_LOGGER_DEBUG(logger, __VA_ARGS__); }
#define LOG_WARN_WITH_LOGGER(logger, ...) \
    { SPDLOG_LOGGER_WARN(logger, __VA_ARGS__); }
#define LOG_ERROR_WITH_LOGGER(logger, ...) \
    { SPDLOG_LOGGER_ERROR(logger, __VA_ARGS__); }
#define LOG_INFO_WITH_LOGGER(logger, ...) \
    { SPDLOG_LOGGER_INFO(logger, __VA_ARGS__); }
#define LOG_CRITICAL_WITH_LOGGER(logger, ...) \
    { SPDLOG_LOGGER_CRITICAL(logger, __VA_ARGS__); }

// use fmt lib, e.g. LOG_WARN("warn log, {1}, {1}, {2}", 1, 2);
#if 0
#define LOG_TRACE(msg, ...) spdlog::log({FILENAME_, __LINE__, __FUNCTION__}, spdlog::level::trace, msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) spdlog::log({FILENAME_, __LINE__, __FUNCTION__}, spdlog::level::debug, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...)  spdlog::log({FILENAME_, __LINE__, __FUNCTION__}, spdlog::level::info, msg, ##__VA_ARGS__)
#define LOG_WARN(msg, ...)  spdlog::log({FILENAME_, __LINE__, __FUNCTION__}, spdlog::level::warn, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) spdlog::log({FILENAME_, __LINE__, __FUNCTION__}, spdlog::level::err, msg, ##__VA_ARGS__)
#define LOG_CRITICAL(msg, ...) \
    spdlog::log({FILENAME_, __LINE__, __FUNCTION__}, spdlog::level::critical, msg, ##__VA_ARGS__)
#else
#define LOG_TRACE(msg, ...)    spdlog::log({FILENAME_, __LINE__, ""}, spdlog::level::trace, msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...)    spdlog::log({FILENAME_, __LINE__, ""}, spdlog::level::debug, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...)     spdlog::log({FILENAME_, __LINE__, ""}, spdlog::level::info, msg, ##__VA_ARGS__)
#define LOG_WARN(msg, ...)     spdlog::log({FILENAME_, __LINE__, ""}, spdlog::level::warn, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...)    spdlog::log({FILENAME_, __LINE__, ""}, spdlog::level::err, msg, ##__VA_ARGS__)
#define LOG_CRITICAL(msg, ...) spdlog::log({FILENAME_, __LINE__, ""}, spdlog::level::critical, msg, ##__VA_ARGS__)
#endif

namespace helper {
namespace logger {

enum class LogLevel : int8_t {
    OFF = -1,
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERR = 3,
    FATAL = 4,
    TRACE = 5,
};

struct LoggerParam {
    LogLevel logLevel;
    std::string logDir;
    std::string logName;
    size_t maxFileSize;
    size_t maxRetentionDays;
};

namespace detail {

inline bool try_to_time(std::time_t& out, const std::string& datetime,
                        const std::regex& reg = std::regex{R"((\d{4})-(\d{2})-(\d{2}))"}) noexcept {
    std::smatch pieces_match;
    if (!std::regex_match(datetime, pieces_match, reg))
        return false;

    const int year = std::stoi(pieces_match[1]);
    const int month = std::stoi(pieces_match[2]);
    const int day = std::stoi(pieces_match[3]);

    struct std::tm tm {};

    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;

    //
    if ((out = mktime(&tm)) == (time_t)-1) {
        fprintf(stderr, "Could not convert time input to time_t\n");
        return false;
    }

    return true;
}

inline void removeInvalidLogDir(const std::string& base_log_dir_path,
                                const std::chrono::time_point<std::chrono::system_clock>& invalid_time_point_boundary) {
    // const auto time = std::chrono::system_clock::to_time_t(invalid_time_point_boundary);
    // std::cout << ">>> " << std::put_time(std::localtime(&time), "%F %T") << std::endl;

    auto&& dirs = listSubDir(base_log_dir_path);
    for (auto&& dir : dirs) {
        std::time_t dir_time;
        if (!detail::try_to_time(dir_time, getLastDirName(dir)))
            continue;
        if (invalid_time_point_boundary > std::chrono::system_clock::from_time_t(dir_time)) {
            std::cout << ">>> Will Remove Dir is " << dir << std::endl;
            removeDirectory(dir);
        }
    }
}

} // namespace detail

} // namespace logger
} // namespace helper

namespace spdlog {
namespace sinks {

template <typename Mutex>
class easy_file_sink final : public base_sink<Mutex> {
public:
    easy_file_sink(filename_t base_log_dir, filename_t log_basename, size_t max_file_bytes, size_t max_retention_days)
        : base_log_dir_(std::move(base_log_dir))
        , log_basename_(std::move(log_basename))
        , max_file_bytes_(max_file_bytes)
        , max_retention_days_(max_retention_days) {
        auto now = log_clock::now();
        auto filename = gen_filename_by_daily(base_log_dir_, log_basename_, now_tm(now));

        // remove before logs.
        if (max_retention_days_ > 0) {
            // using namespace std::chrono_literals;
            ::helper::logger::detail::removeInvalidLogDir(base_log_dir_,
                                                          now - max_retention_days * std::chrono::hours(24));
        }

        file_helper_.open(filename, false);
        current_size_ = file_helper_.size();
        rotation_tp_ = next_rotation_tp_();

        if (max_retention_days_ > 0) {
            files_path_list_.push_back(std::move(std::set<filename_t>()));
            files_path_list_[files_path_list_.size() - 1].insert(filename);
        }
    }

    filename_t filename() {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        return file_helper_.filename();
    }

protected:
    void sink_it_(const details::log_msg& msg) override {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        current_size_ += formatted.size();

        auto time = msg.time;
        if (time >= rotation_tp_) {
            file_helper_.close();
            auto filename = gen_filename_by_daily(base_log_dir_, log_basename_, now_tm(time));
            file_helper_.open(filename, false);
            current_size_ = file_helper_.size();
            rotation_tp_ = next_rotation_tp_();

            {
                files_path_list_.push_back(std::move(std::set<filename_t>()));
                files_path_list_[files_path_list_.size() - 1].emplace(filename);
            }

            // Do the cleaning only at the end because it might throw on
            // failure.
            if (max_retention_days_ > 0 && files_path_list_.size() > max_retention_days_)
                delete_old_();
        }
        else if (current_size_ >= max_file_bytes_) {
            file_helper_.close();
            auto src_name = gen_filename_by_daily(base_log_dir_, log_basename_, now_tm(time));
            auto target_name = gen_filename_by_filesize(base_log_dir_, log_basename_, now_tm(time),
                                                        files_path_list_[files_path_list_.size() - 1].size());

            // rename file if failed then us `target_name` as src_name.
            if (!rename_file_(src_name, target_name)) {
                details::os::sleep_for_millis(200);
                if (!rename_file_(src_name, target_name)) {
                    fprintf(stderr, "%s:%d rename %s to %s failed\n", FILENAME_, __LINE__, src_name.c_str(),
                            target_name.c_str());
                    src_name = target_name;
                }
            }

            files_path_list_[files_path_list_.size() - 1].emplace(src_name);
            if (src_name != target_name)
                files_path_list_[files_path_list_.size() - 1].emplace(target_name);

            file_helper_.open(src_name, false);
            current_size_ = file_helper_.size();
            rotation_tp_ = next_rotation_tp_();
        }

        file_helper_.write(formatted);
    }

    void flush_() override { file_helper_.flush(); }

private:
    tm now_tm(log_clock::time_point tp) {
        time_t t_now = log_clock::to_time_t(tp);
        return spdlog::details::os::localtime(t_now);
    }

    /**
     * @brief Get next day tm.
     *
     * @return log_clock::time_point
     */
    log_clock::time_point next_rotation_tp_() {
        auto now = log_clock::now();
        tm date = now_tm(now);
        date.tm_hour = 0;
        date.tm_min = 0;
        date.tm_sec = 0;
        auto rotation_time = log_clock::from_time_t(std::mktime(&date));
        if (rotation_time > now)
            return rotation_time;
        return {rotation_time + std::chrono::hours(24)};
    }

    // Delete the file N rotations ago.
    // Throw spdlog_ex on failure to delete the old file.
    void delete_old_() {
        for (auto iter = files_path_list_.begin(); iter != files_path_list_.end();) {
            if (files_path_list_.size() <= max_retention_days_)
                break;

            for (const auto& it : *iter) {
                bool ok = details::os::remove_if_exists(it) == 0;
                if (!ok)
                    std::cerr << "Failed removing daily file " << details::os::filename_to_str(it) << " " << errno
                              << std::endl;
            }
            files_path_list_.erase(iter);
        }
    }

    /*  */
    static filename_t gen_filename_by_daily(const filename_t& base_log_dir, const filename_t& basename,
                                            const tm& now_tm) {
        auto date_dir_name =
          fmt::format("{:04d}-{:02d}-{:02d}", now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);
        spdlog::filename_t basename_wo_ext, ext;
        std::tie(basename_wo_ext, ext) = details::file_helper::split_by_extension(basename);
        return fmt::format(SPDLOG_FILENAME_T("{}{}{}"), ::addSepIfNot(::join<2>({base_log_dir, date_dir_name}, OS_SEP)),
                           basename_wo_ext, ext.empty() ? ".log" : ext);
    }

    //
    static filename_t gen_filename_by_filesize(const filename_t& base_log_dir, const filename_t& basename,
                                               const tm& now_tm, const int32_t& idx) {
        auto date_dir_name =
          fmt::format("{:04d}-{:02d}-{:02d}", now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);

        spdlog::filename_t basename_wo_ext, ext;
        std::tie(basename_wo_ext, ext) = details::file_helper::split_by_extension(basename);
        return fmt::format(SPDLOG_FILENAME_T("{}{}{}{}"),
                           ::addSepIfNot(::join<2>({base_log_dir, date_dir_name}, OS_SEP)), basename_wo_ext,
                           fmt::format("{:02d}{:02d}{:02d}", now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec),
                           ext.empty() ? ".log" : ext);
    }

    static bool rename_file_(const filename_t& src_filename, const filename_t& target_filename) {
        (void)details::os::remove(target_filename);
        return details::os::rename(src_filename, target_filename) == 0;
    }

    filename_t base_log_dir_, log_basename_;
    log_clock::time_point rotation_tp_;
    details::file_helper file_helper_;
    std::size_t max_file_bytes_, max_retention_days_, current_size_;
    std::vector<std::set<filename_t>> files_path_list_;
};

using easy_file_sink_mt = easy_file_sink<std::mutex>;
using easy_file_sink_st = easy_file_sink<details::null_mutex>;

} // namespace sinks
} // namespace spdlog

namespace spdlog {

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> easy_logger_mt(const std::string& logger_name, const filename_t& filename,
                                              size_t max_size, size_t max_keep_days = -1) {
    return Factory::template create<sinks::easy_file_sink_mt>(logger_name, filename, max_size, max_keep_days);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> easy_logger_st(const std::string& logger_name, const filename_t& filename,
                                              size_t max_size, size_t max_keep_days = -1) {
    return Factory::template create<sinks::easy_file_sink_st>(logger_name, filename, max_size, max_keep_days);
}

} // namespace spdlog

namespace helper {
namespace logger {

class LoggerGenerator {
public:
    static spdlog::level::level_enum to_spdlog_level(const LogLevel& log_level) noexcept(false) {
        static std::map<LogLevel, spdlog::level::level_enum> log_level_to_spdlog_level{
          {LogLevel::OFF,   spdlog::level::off     },
          {LogLevel::TRACE, spdlog::level::trace   },
          {LogLevel::DEBUG, spdlog::level::debug   },
          {LogLevel::INFO,  spdlog::level::info    },
          {LogLevel::WARN,  spdlog::level::warn    },
          {LogLevel::ERR,   spdlog::level::err     },
          {LogLevel::FATAL, spdlog::level::critical},
        };
        auto it = log_level_to_spdlog_level.find(log_level);
        return (it != log_level_to_spdlog_level.end()) ? it->second : spdlog::level::debug;
    }

    static std::shared_ptr<spdlog::logger> gen_logger(const spdlog::level::level_enum& level,
                                                      const std::string& logger_name, const std::string& base_log_dir,
                                                      size_t max_file_bytes, size_t max_retention_days) {
        std::vector<spdlog::sink_ptr> sinks;
        sinks.reserve(2);

        if (level >= spdlog::level::level_enum::warn) {
            sinks.push_back(std::make_shared<spdlog::sinks::stderr_color_sink_mt>());
        }
        else /*if (level >= spdlog::level::level_enum::info)*/ {
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        }

        if (max_file_bytes > 0 && max_retention_days > 0) {
            sinks.push_back(std::make_shared<spdlog::sinks::easy_file_sink_mt>(base_log_dir, logger_name,
                                                                               max_file_bytes, max_retention_days));
        }

        // set default logger format.
        auto logger = std::make_shared<spdlog::logger>(logger_name, std::begin(sinks), std::end(sinks));
        logger->set_level(level);
        // logger->set_pattern("[%L %Y-%m-%d %T.%e (P)%P (T)%t] [%!] [%s:%#] %v");
        logger->set_pattern("[%C-%m-%d %T.%e (P)%P (T)%t] [%l] [%s:%#] %v");
        return logger;
    }

    static void set_default_logger(LoggerParam&& logParam) {
        const auto spd_level = to_spdlog_level(logParam.logLevel);

        // spdlog::level::level_enum spd_level =
        //     helper::logger::to_spdlog_level<LogLevel::INFO>();

        auto logger =
          gen_logger(spd_level, logParam.logName, logParam.logDir, logParam.maxFileSize, logParam.maxRetentionDays);
        spdlog::set_default_logger(logger);

        spdlog::flush_on(spd_level);
        spdlog::flush_every(std::chrono::seconds(1));
    }
};

enum class FileSizeType : int32_t { B = 1, KB = B * 1024, MB = KB * 1024 };

template <FileSizeType fileSizeType = FileSizeType::MB>
static void InitializeGlobalLogger(LoggerParam&& logParam) {
    static LoggerGenerator loggerGenerator;
    logParam.maxFileSize *= static_cast<int32_t>(fileSizeType);
    LoggerGenerator::set_default_logger(std::move(logParam));
}

} // namespace logger
} // namespace helper

#endif // INFERENCEAPI_LOGGER_HPP
