// Copyright (c) 2018 The Raven Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef RAVENCOIN_MESSAGES_H
#define RAVENCOIN_MESSAGES_H


#include <uint256.h>
#include <serialize.h>
#include <primitives/transaction.h>
#include <validation.h>
#include "assettypes.h"

class CMessage;
class COutPoint;

// Message Database caches
extern std::set<COutPoint> setDirtyMessagesRemove;
extern std::map<COutPoint, CMessage> mapDirtyMessagesAdd;
extern std::map<COutPoint, CMessage> mapDirtyMessagesOrphaned;

// Message Channel Database caches
extern std::set<std::string> setDirtyChannelsAdd;
extern std::set<std::string> setDirtyChannelsRemove;
extern std::set<std::string> setChannelsAskedForFalse;

// Lock for messaging
extern CCriticalSection cs_messaging;

bool IsChannelWatched(const std::string &name);

bool GetMessage(const COutPoint &out, CMessage &message);

void AddChannel(const std::string &name);

void RemoveChannel(const std::string &name);

void AddMessage(const CMessage &message);

void RemoveMessage(const CMessage &message);
void RemoveMessage(const COutPoint &out);

void OrphanMessage(const CMessage &message);
void OrphanMessage(const COutPoint &out);

enum class MessageStatus {
    READ = 0,
    UNREAD = 1,
    EXPIRED = 2,
    SPAM = 3,
    HIDDEN = 4,
    ORPHAN = 5,
    ERROR = 6
};

int8_t IntFromMessageStatus(MessageStatus status);
MessageStatus MessageStatusFromInt(int8_t nStatus);

std::string MessageStatusToString(MessageStatus status);

class CMessage {
public:

    COutPoint out;
    std::string strName;
    std::string ipfsHash;
    int64_t time;
    int64_t nExpiredTime;
    MessageStatus status;
    int nBlockHeight;

    CMessage();

    void SetNull() {
        nExpiredTime = 0;
        out = COutPoint();
        strName = "";
        ipfsHash = "";
        time = 0;
        status = MessageStatus::ERROR;
        nBlockHeight = 0;
    }

    std::string ToString() {
        return strprintf("CMessage(%s, Name=%s, Message=%s, Expires=%u, Time=%u, BlockHeight=%u)", out.ToString(), strName,
                         EncodeIPFS(ipfsHash), nExpiredTime, time, nBlockHeight);
    }

    CMessage(const COutPoint &out, const std::string &strName, const std::string &ipfsHash, const int64_t &nExpiredTime,
             const int64_t &time);

    bool operator<(const CMessage &rhs) const {
        return out < rhs.out;
    }

    ADD_SERIALIZE_METHODS;

    template<typename Stream, typename Operation>
    inline void SerializationOp(Stream &s, Operation ser_action) {
        READWRITE(out);
        READWRITE(strName);
        READWRITE(ipfsHash);
        READWRITE(time);
        READWRITE(nExpiredTime);
        READWRITE(nBlockHeight);

        if (ser_action.ForRead()) {
            int8_t nStatus = 6;
            ::Unserialize(s, nStatus);
            status = MessageStatusFromInt(nStatus);
        } else {
            ::Serialize(s, IntFromMessageStatus(status));
        }
    }
};

#endif //RAVENCOIN_MESSAGES_H
