#pragma once

#include <sys/select.h>

#include <unordered_map>

#include <common/ISelectable.h>
#include <common/Errors.h>

class Selector
{
public:
    Selector();

    ECode Add(ISelectable *selectable);
    ECode Remove(ISelectable *selectable);
    ECode Process();

private:
    std::unordered_map<int, ISelectable *> _selectables;
    fd_set _fdset;
    int _fdmax;
};
