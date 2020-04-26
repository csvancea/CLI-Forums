#pragma once

#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <common/Net.h>
#include <common/Errors.h>

class Forums
{
public:
    struct Message {
        uint32_t id;
        std::string msg;
        uint8_t type;
        Peer source;

        uint32_t ref_count{};
    };

    Forums();
    Forums(const Forums&) = delete;
    Forums(const Forums&&) = delete;
    Forums& operator= (const Forums&) = delete;

    ECode Subscribe(const std::string& client_id, const std::string& topic, uint8_t sf);
    ECode Unsubscribe(const std::string& client_id, const std::string& topic);

    ECode AddMessage(const std::string& topic, const std::string& message, uint8_t type, const Peer& source);

    /**
     * Returneaza o lista de perechi intre:
     *   1. numele topicului
     *   2. o lista cu mesajele din topicul respectiv
     */
    std::list<std::pair<std::string, std::list<Message>>> GetUserMessages(const std::string& client_id);

private:
    struct UserSubscribe {
        std::unordered_set<std::string> topics;
        uint32_t last_msg_id;
    };
    struct TopicSubscription {
        TopicSubscription(const std::string& client_id, uint8_t sf) :
            client_id(client_id), sf(sf) { }

        std::string client_id;
        uint8_t sf;

        inline bool operator==(const TopicSubscription& rhs) const {
            return client_id == rhs.client_id;
        } 
    };
    struct TopicSubscriptionHash {
        std::size_t operator()(const TopicSubscription& ts) const {
            return std::hash<std::string>()(ts.client_id);
        }
    };

    uint32_t _lastMessageId;

    /**
     * Key:   topic_name
     * Value: Lista ce contine toate mesajele din acel topic cu ID-ul asociat
     */
    std::unordered_map<std::string, std::list<Message>> _messages;

    /**
     * Key:   client_id
     * Value: Structura ce contine:
     *   * setul topicurilor la care este abonat
     *   * id-ul ultimului mesaj
     */
    std::unordered_map<std::string, UserSubscribe> _usersSubscribed;

    /**
     * Key:   topic_name
     * Value: Set ce contine clientii ce sunt abonati la topic cu sf asociat
     */
    std::unordered_map<std::string, std::unordered_set<TopicSubscription, TopicSubscriptionHash>> _topicSubscriptions;
};
