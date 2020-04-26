#include <common/Logger.h>

Logger::Logger() :	_outputToFile(false), _outputToFileRules(0), _outputToStdout(false), _outputToStdoutRules(0),
					_outputToDebugger(false), _outputToDebuggerRules(0)
{
}

Logger::~Logger()
{
	_outputFile.close();
}

Logger& Logger::GetInstance()
{
	static Logger instance;
	return instance;
}

const std::string& Logger::GetRulePrefix(int rule)
{
	static std::string STRING_DEBUG{ "[DEBUG] " };
	static std::string STRING_WARNING{ "[WARNING] " };
	static std::string STRING_ERROR{ "[ERROR] " };
	static std::string STRING_UNKNOWN{ "" };

	switch (rule) {
	case Logger::RULE_DEBUG: return STRING_DEBUG;
	case Logger::RULE_WARNING: return STRING_WARNING;
	case Logger::RULE_ERROR: return STRING_ERROR;
	}
	return STRING_UNKNOWN;
}

bool Logger::SetOutputToFile(bool enable, int rules, const std::string& filename)
{
	if (enable) {
		if (_outputFile.is_open()) {
			LOG_WARNING("Log file is already open.");
			return false;
		}

		_outputFile.open(filename, std::ofstream::out/* | std::ofstream::app*/);
		_outputToFile = _outputFile.is_open();

		if (!_outputToFile) {
			LOG_ERROR("Can't open file \"{}\" for logging.", filename);
		}
	}
	else {
		_outputFile.close();
		_outputToFile = false;
	}
	_outputToFileRules = rules;
	return _outputToFile;
}

bool Logger::SetOutputToStdout(bool enable, int rules)
{
	_outputToStdout = enable;
	_outputToStdoutRules = rules;
	return enable;
}

bool Logger::SetOutputToDebugger(bool enable, int rules)
{
	_outputToDebugger = enable;
	_outputToDebuggerRules = rules;
	return enable;
}
