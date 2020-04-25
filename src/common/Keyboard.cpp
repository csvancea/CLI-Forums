#include <common/Keyboard.h>
#include <common/Logger.h>

#include <unistd.h>

#define BUFFER_SIZE 2048

Keyboard::Keyboard() { }

ECode Keyboard::Init()
{
    return ECode::OK;
}

int Keyboard::GetFileDescriptor() const
{
    return STDIN_FILENO;
}

void Keyboard::Select()
{
    ssize_t bytes_read;
    std::string buffer;
 
    buffer.resize(BUFFER_SIZE);
    bytes_read = read(STDIN_FILENO, &buffer[0], BUFFER_SIZE);
    if (bytes_read < 0) {
        LOG_ERROR("read failed with error code: {}", bytes_read);
        return;
    }
    buffer.resize(bytes_read);

    if (buffer.length() > 0 && buffer.back() == '\n') {
        buffer.pop_back();
    }

    LOG_MESSAGE("Read {} bytes from keyboard: {}", bytes_read, buffer);

    /**
     * In cazul in care se citesc mai multe comenzi, le separ dupa newline
     */

    auto delim = std::string("\n");
    auto start = 0U;
    auto end = buffer.find(delim);
    while (end != std::string::npos)
    {
        auto token = buffer.substr(start, end - start);
        _commands.push(std::move(token));

        start = end + delim.length();
        end = buffer.find(delim, start);
    }
    auto last_token = buffer.substr(start, end);
    _commands.push(std::move(last_token));
}

ECode Keyboard::GetCommand(std::string& command)
{
    if (_commands.empty()) {
        return ECode::NONEXISTENT;
    }

    command = std::move(_commands.front());
    _commands.pop();
    
    return ECode::OK;
}
