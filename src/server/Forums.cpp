#include <server/Forums.h>

Forums::Forums() : _lastMessageId(0)
{

}

ECode Forums::Subscribe(const std::string& client_id, const std::string& topic, uint8_t sf)
{
    auto pair = _topicSubscriptions[topic].insert(TopicSubscription(client_id, sf));
    if (!pair.second) {
        return ECode::ALREADY_SUBSCRIBED;
    }

    auto it = _usersSubscribed.find(client_id);
    if (it == _usersSubscribed.end()) {
        _usersSubscribed[client_id].last_msg_id = _lastMessageId;
    }
    _usersSubscribed[client_id].topics.insert(topic);
    return ECode::OK;
}

ECode Forums::Unsubscribe(const std::string& client_id, const std::string& topic)
{
    _topicSubscriptions[topic].erase(TopicSubscription(client_id, 0));
    _usersSubscribed[client_id].topics.erase(topic);
    return ECode::OK;
}

ECode Forums::AddMessage(const std::string& topic, const std::string& message, uint8_t type, const Peer& source)
{
    size_t subscribers = 0;
    for (const auto& client :  _topicSubscriptions[topic]) {
        subscribers += (client.sf || _usersConnected.count(client.client_id));
    }

    if (subscribers) {
        Message msg;
        msg.id = ++_lastMessageId;
        msg.msg = message;
        msg.type = type;
        msg.source = source;
        msg.ref_count = subscribers;

        _messages[topic].push_back(std::move(msg));
    }

    return ECode::OK;
}

std::list<std::pair<std::string, std::list<Forums::Message>>> Forums::GetUserMessages(const std::string& client_id)
{
    std::list<std::pair<std::string, std::list<Forums::Message>>> messages;

    for (const auto& topic : _usersSubscribed[client_id].topics) {
        auto pair = std::make_pair(topic, std::list<Message>());

        auto it = _messages[topic].begin();
        while (it != _messages[topic].end()) {

            auto& message = *it;
            if (message.id > _usersSubscribed[client_id].last_msg_id) {
                pair.second.push_back(message);
                message.ref_count--;
            }

            if (message.ref_count == 0) {
                it = _messages[topic].erase(it);
            }
            else {
                it++;
            }
        }

        messages.push_back(std::move(pair));
    }

    _usersSubscribed[client_id].last_msg_id = _lastMessageId;
    return messages;
}

void Forums::SetConnectionStatus(const std::string& client_id, bool status)
{
    if (status) {
        _usersConnected.insert(client_id);
    }
    else {
        _usersConnected.erase(client_id);
    }
}
