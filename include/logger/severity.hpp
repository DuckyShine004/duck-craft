namespace logger {

enum class Severity : int {
    INFO = 0,
    DEBUG = 1,
    WARN = 2,
    ERROR = 3,
    FATAL = 4,
};

// clang-format off
inline static constexpr const char *_NAMES[] = {
    "INFO",
    "DEBUG",
    "WARN",
    "ERROR",
    "FATAL",
};

inline static constexpr const char *_BACKGROUND_COLOURS[] = {
    "#a6e3a1",
    "#89b4fa",
    "#f9e2af",
    "#eba0ac",
    "#f38ba8",
};

inline static constexpr const char *_TEXT_COLOURS[] = {
    "#11111b",
    "#11111b",
    "#11111b",
    "#11111b",
    "#11111b",
};
// clang-format on

} // namespace logger
