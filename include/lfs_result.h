#pragma once

#include <utility>
#include <type_traits>
#include <string>
#include <system_error>

namespace fs {

    enum class ErrorCode {
        kSuccess = 0,
        kFileNotFound,
        kFileExists,
        kNotDirectory,
        kIsDirectory,
        kDirectoryNotEmpty,
        kBadFileDescriptor,
        kFileTooLarge,
        kInvalidParameter,
        kNoSpaceOnDevice,
        kNoMemory,
        kNoAttribute,
        kNameTooLong,
        kCorrupted,
        kIoError,
        kUnknownError
    };

    inline const char* errorToString(ErrorCode err) {
        switch (err) {
            case ErrorCode::kSuccess: return "Success";
            case ErrorCode::kFileNotFound: return "File not found";
            case ErrorCode::kFileExists: return "File exists";
            case ErrorCode::kNotDirectory: return "Not a directory";
            case ErrorCode::kIsDirectory: return "Is a directory";
            case ErrorCode::kDirectoryNotEmpty: return "Directory not empty";
            case ErrorCode::kBadFileDescriptor: return "Bad file descriptor";
            case ErrorCode::kFileTooLarge: return "File too large";
            case ErrorCode::kInvalidParameter: return "Invalid parameter";
            case ErrorCode::kNoSpaceOnDevice: return "No space on device";
            case ErrorCode::kNoMemory: return "Out of memory";
            case ErrorCode::kNoAttribute: return "No attribute";
            case ErrorCode::kNameTooLong: return "File name too long";
            case ErrorCode::kCorrupted: return "Corrupted filesystem";
            case ErrorCode::kIoError: return "I/O error";
            default: return "Unknown error";
        }
    }

    template <typename T, typename E = ErrorCode>
    class Result {
    public:
        using value_type = T;
        using error_type = E;

        Result(const T& val) : _has_value(true), _val(val) {}
        Result(T&& val) : _has_value(true), _val(std::move(val)) {}
        Result(E err) : _has_value(false), _err(err) {}

        ~Result() {
            if (_has_value) {
                _val.~T();
            }
        }

        Result(const Result& other) : _has_value(other._has_value) {
            if (_has_value) {
                new (&_val) T(other._val);
            } else {
                _err = other._err;
            }
        }

        Result(Result&& other) noexcept : _has_value(other._has_value) {
            if (_has_value) {
                new (&_val) T(std::move(other._val));
            } else {
                _err = other._err;
            }
        }

        Result& operator=(const Result& other) {
            if (this != &other) {
                if (_has_value) {
                    _val.~T();
                }
                _has_value = other._has_value;
                if (_has_value) {
                    new (&_val) T(other._val);
                } else {
                    _err = other._err;
                }
            }
            return *this;
        }

        Result& operator=(Result&& other) noexcept {
            if (this != &other) {
                if (_has_value) {
                    _val.~T();
                }
                _has_value = other._has_value;
                if (_has_value) {
                    new (&_val) T(std::move(other._val));
                } else {
                    _err = other._err;
                }
            }
            return *this;
        }

        bool has_value() const noexcept { return _has_value; }
        explicit operator bool() const noexcept { return _has_value; }

        T& value() & {
            return _val;
        }

        const T& value() const & {
            return _val;
        }

        T&& value() && {
            return std::move(_val);
        }

        E error() const noexcept {
            return _err;
        }

        T* operator->() { return &_val; }
        const T* operator->() const { return &_val; }
        T& operator*() & { return _val; }
        const T& operator*() const & { return _val; }
        T&& operator*() && { return std::move(_val); }

    private:
        bool _has_value;
        union {
            T _val;
            E _err;
        };
    };

    // Specialization for void (only error or success status)
    template <typename E>
    class Result<void, E> {
    public:
        using value_type = void;
        using error_type = E;

        Result() : _has_value(true), _err(static_cast<E>(0)) {}
        Result(E err) : _has_value(false), _err(err) {}

        bool has_value() const noexcept { return _has_value; }
        explicit operator bool() const noexcept { return _has_value; }

        E error() const noexcept { return _err; }

        static Result success() { return Result(); }

    private:
        bool _has_value;
        E _err;
    };

} // namespace fs
