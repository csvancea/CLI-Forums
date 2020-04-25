#pragma once

#include <queue>
#include <string>

#include <common/ISelectable.h>
#include <common/Errors.h>

class Keyboard : public ISelectable
{
public:
    Keyboard();
    Keyboard(const Keyboard&) = delete;
    Keyboard(const Keyboard&&) = delete;
    Keyboard& operator= (const Keyboard&) = delete;

    virtual int GetFileDescriptor() const override;
    virtual void Select() override;

    ECode Init();
    ECode GetCommand(std::string& command);

private:
    std::queue<std::string> _commands;
};
