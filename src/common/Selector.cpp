#include <common/Selector.h>
#include <common/Logger.h>

Selector::Selector() : _fdmax(-1)
{
    FD_ZERO(&_fdset);
}

ECode Selector::Add(ISelectable *selectable)
{
    auto fd = selectable->GetFileDescriptor();
    auto ret = _selectables.insert(std::make_pair(fd, selectable));
    
    if (ret.second == false) {
        return ECode::DUPLICATE;
    }

    _fdmax = std::max(fd, _fdmax);
    FD_SET(fd, &_fdset);

    LOG_DEBUG("Inserted fd={} into _fdset, _fdmax={}", fd, _fdmax);
    return ECode::OK;
}

ECode Selector::Remove(ISelectable *selectable)
{
    auto fd = selectable->GetFileDescriptor();
    if (_selectables.erase(fd) == 0) {
        return ECode::NONEXISTENT;
    }

    FD_CLR(fd, &_fdset);
    LOG_DEBUG("Removed fd={} from _fdset, _fdmax={}", fd, _fdmax);
    return ECode::OK;
}

ECode Selector::Process()
{
    int ret;
	fd_set fd_tmp = _fdset;
		
	ret = select(_fdmax + 1, &fd_tmp, nullptr, nullptr, nullptr);
    if (ret < 0) {
        LOG_ERROR("select failed, errcode: {}", ret);
        return ECode::FD_SELECT;
    }

    for (int i = 0; i <= _fdmax; ++i) {
        if (FD_ISSET(i, &fd_tmp)) {
            LOG_DEBUG("Selected fd={}", i);
            _selectables[i]->Select();
        }
    }

    return ECode::OK;
}
