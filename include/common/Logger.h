#pragma once

#ifdef _WIN32
	#include <Windows.h>
#endif

#include <string>
#include <iostream>
#include <fstream>
#include <fmt/format.h>

#ifdef _DEBUG
	#define ENABLE_LOGGING
#endif

#ifdef ENABLE_LOGGING

#define LOG_DEBUG(format, ...) \
            do { Logger::GetInstance().Log(Logger::RULE_DEBUG, format, ## __VA_ARGS__); } while (0)

#define LOG_WARNING(format, ...) \
            do { Logger::GetInstance().Log(Logger::RULE_WARNING, format, ## __VA_ARGS__); } while (0)

#define LOG_ERROR(format, ...) \
            do { Logger::GetInstance().Log(Logger::RULE_ERROR, format, ## __VA_ARGS__); } while (0)

#else

#define LOG_DEBUG(format, ...)
#define LOG_WARNING(format, ...)
#define LOG_ERROR(format, ...)

#endif

#define LOG_MESSAGE(format, ...) \
            do { Logger::GetInstance().Log(Logger::RULE_MESSAGE, format, ## __VA_ARGS__); } while (0)

class Logger
{
private:
	Logger();
	Logger(const Logger&) = delete;
	Logger(const Logger&&) = delete;
	~Logger();

public:
	enum {
		RULE_NONE    = 0x00,
		RULE_DEBUG   = 0x01,
		RULE_WARNING = 0x02,
		RULE_ERROR   = 0x04,
		RULE_MESSAGE = 0x08,
		RULE_ALL     = 0xFF
	};

	const std::string& GetRulePrefix(int rule);

	static Logger& GetInstance();

	bool SetOutputToFile(bool enable, int rules = RULE_ALL, const std::string& filename = "chess_debug");
	bool SetOutputToStdout(bool enable, int rules = RULE_ALL);
	bool SetOutputToDebugger(bool enable, int rules = RULE_ALL);

	template <typename... Args>
	void Log(int rule, const char *format, const Args& ... args) {
		std::string message = fmt::vformat(format, fmt::make_format_args(args...));
		std::string final = fmt::format("{}{}\n", GetRulePrefix(rule), message);

		if (_outputToFile && (_outputToFileRules & rule)) {
			_outputFile << final;
			_outputFile.flush();
		}
		if (_outputToStdout && (_outputToStdoutRules & rule)) {
			std::cout << final;
		}
#ifdef _WIN32
		if (_outputToDebugger && (_outputToDebuggerRules & rule)) {
			OutputDebugStringA(final.c_str());
		}
#endif
	}

private:
	bool _outputToFile;
	int _outputToFileRules;

	bool _outputToStdout;
	int _outputToStdoutRules;

	bool _outputToDebugger;
	int _outputToDebuggerRules;

	std::ofstream _outputFile;
};

