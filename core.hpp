#pragma once

#include <iostream>
#include <fstream>
#include <sstream>

#include <thread>
#include <string>
#include <array>
#include <ctime>

#include <boost/noncopyable.hpp>
#include <boost/format.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>

namespace {
    std::array<std::string, 6> enumArray = {"trace",
                                            "debug",
                                            "info",
                                            "warning",
                                            "error",
                                            "fatal"};
}


namespace logger {
    enum logger_level
    {
        trace,
        debug,
        info,
        warning,
        error,
        fatal
    };

    class core : private boost::noncopyable
    {
        public:
        core() = default;

        ~core() {
            flush();
            _pool.join();
        }

        static std::string getPath() {
            return _path;
        }

        static void setPath(std::string const &path) {
            boost::asio::post(_pool, [=]() {
                _path = path;
            });
        }

        static std::size_t getBufferSize() {
            return _bufferSize;
        }

        static void setBufferSize(std::size_t bufferSize) {
            boost::asio::post(_pool, [=]() {
                _bufferSize = bufferSize;
            });
        }

        static void log(logger::logger_level const &log, std::string const &msg) {
            auto thread_id = std::this_thread::get_id();
            auto task = [=]() {
                try {
                    if (!_stream.is_open()) {
                        if (_path.empty()) {
                            if (log > 3) {
                                std::cerr << _format % getDate() % thread_id % enumArray[log] % msg;
                                return;
                            } else {
                                std::cout << _format % getDate() % thread_id % enumArray[log] % msg;
                                return;
                            }
                        }
                        _stream.open(_path);
                        if (!_stream.is_open()) {
                            throw std::runtime_error("Invalid path to file");
                        }
                    }
                    _buffer << _format % getDate() % thread_id % enumArray[log] % msg;
                    if (_buffer.bad() && !_buffer.eof()) {
                        throw std::runtime_error("Buffer error");
                    }
                    _buffer.seekg(0, std::ios::end);
                    if (_buffer.tellg() > _bufferSize) {
                        _stream << _buffer.str();
                        _buffer.str("");
                    }
                } catch (std::exception &) {
                    _exception = std::current_exception();
                }
            };
            boost::asio::post(_pool, task);
            if (_exception) {
                rethrow_exception(_exception);
            }
        }

        static void flush() {
            boost::asio::post(_pool, []() {
                try {
                    _stream << _buffer.str();
                    _buffer.str("");
                    _stream.flush();
                } catch (std::exception &) {
                    _exception = std::current_exception();
                }
            });
            _pool.join();
            if (_exception) {
                rethrow_exception(_exception);
            }
        }

        private:
        static std::stringstream _buffer;
        static std::ofstream _stream;
        static std::string _path;
        static std::size_t _bufferSize;
        static boost::asio::thread_pool _pool;
        static boost::format _format;
        static std::exception_ptr _exception;

        static std::string getDate() {
            std::stringstream buff;
            time_t now = time(nullptr);
            tm *ltm = localtime(&now);
            buff << 1900 + ltm->tm_year << '-' << ltm->tm_mon + 1 << '-' << ltm->tm_mday;
            buff << ' ' << ltm->tm_hour << ':' << ltm->tm_min << ':' << ltm->tm_sec;
            return buff.str();
        }
    };

    std::stringstream core::_buffer;
    std::ofstream core::_stream;
    std::string core::_path;
    std::size_t core::_bufferSize = 1024;
    boost::asio::thread_pool core::_pool(1);
    boost::format core::_format("[%1%] [%2%] [%3%] %4% \n"); //"[%TimeStamp%] [%ThreadID%] [%Severity%] %Message% \n"
    std::exception_ptr core::_exception;

    namespace detail {
        static core log;
    }
}
