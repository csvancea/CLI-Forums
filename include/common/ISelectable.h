#pragma once

class ISelectable
{
public:
    virtual ~ISelectable() = default;

    virtual int GetFileDescriptor() const = 0;
    virtual void Select() = 0;

protected:
    ISelectable() = default;
    ISelectable(const ISelectable&) = default;
    ISelectable& operator= (const ISelectable&) = default;
};
