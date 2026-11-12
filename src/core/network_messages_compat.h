/*
 *  This file is part of CounterStrikeSharp.
 *  CounterStrikeSharp is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CounterStrikeSharp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CounterStrikeSharp.  If not, see <https://www.gnu.org/licenses/>. *
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include <networksystem/inetworkmessages.h>
#include <networksystem/inetworkserializer.h>
#include "utlstring.h"

#include "core/gameconfig.h"
#include "core/globals.h"
#include "utils/virtual.h"

// CS2 1.41.9.7 (2026-11-11) rewrote network message registration in libnetworksystem:
//  - The INetworkMessageInternal* handed around by the engine (e.g. to IGameEventSystem::PostEventAbstract) is no
//    longer a serializer object with a vtable. It is the 40 byte NetMessageInfo_t record itself, so calling any
//    virtual on it through the pinned SDK header jumps through the message name as if it were a vtable and crashes
//    the server on the first posted message (the SourceTV bot joining is enough).
//  - INetworkMessages lost SerializeAbstract, AllocateAndCopyConstruct and Deallocate and moved FindOrCreateNetMessage,
//    so FindNetworkMessagePartial and FindNetworkMessageById moved to new vtable slots.
// Layout cross-checked against the 1.41.9.7 libengine2.so, which reads the buffer type straight from byte 37 of the
// record (`movzx ebx, byte ptr [rax + 0x25]; and ebx, 0x1f`) instead of going through a vtable.
namespace counterstrikesharp::netmessages {

struct NetMessageRecord_t
{
    CUtlString m_szName;
    CNetMessage* (*m_pfnAllocate)();
    CUtlString m_szGroup;
    int m_nMessageId;
    int m_nCategories;
    int m_nRequiredHandlerCategories;
    uint8_t m_nGroupId;
    uint8_t m_nBufType : 5;
    uint8_t m_bCannotBeNetworked : 1;
    uint8_t m_bUnk001 : 1;
    uint8_t m_bNoSignonGroup : 1;
};

static_assert(sizeof(void*) != 8 || (sizeof(NetMessageRecord_t) == 40 && offsetof(NetMessageRecord_t, m_pfnAllocate) == 8 &&
                                     offsetof(NetMessageRecord_t, m_szGroup) == 16 && offsetof(NetMessageRecord_t, m_nMessageId) == 24 &&
                                     offsetof(NetMessageRecord_t, m_nCategories) == 28 && offsetof(NetMessageRecord_t, m_nGroupId) == 36),
              "NetMessageRecord_t layout");

inline const NetMessageRecord_t* GetRecord(INetworkMessageInternal* pNetMessage)
{
    return reinterpret_cast<const NetMessageRecord_t*>(pNetMessage);
}

inline int GetMessageId(INetworkMessageInternal* pNetMessage) { return pNetMessage ? GetRecord(pNetMessage)->m_nMessageId : -1; }

inline const char* GetMessageName(INetworkMessageInternal* pNetMessage)
{
    if (!pNetMessage) return "";
    const char* name = GetRecord(pNetMessage)->m_szName.Get();
    return name ? name : "";
}

inline CNetMessage* AllocateMessage(INetworkMessageInternal* pNetMessage)
{
    if (!pNetMessage) return nullptr;
    auto pfnAllocate = GetRecord(pNetMessage)->m_pfnAllocate;
    return pfnAllocate ? pfnAllocate() : nullptr;
}

// INetworkMessages has no base class, so these are absolute vtable slots. They can be overridden from gamedata
// without a rebuild; the built-in values apply when the key is missing (e.g. after a gamedata auto-update).
inline int ResolveIndex(const char* gamedataKey, int fallback)
{
    int index = globals::gameConfig ? globals::gameConfig->GetOffset(gamedataKey) : -1;
    return index >= 0 ? index : fallback;
}

inline INetworkMessageInternal* FindNetworkMessagePartial(const char* szPartialName)
{
    static const int index = ResolveIndex("INetworkMessages::FindNetworkMessagePartial", 10);
    return CALL_VIRTUAL(INetworkMessageInternal*, index, globals::networkMessages, szPartialName);
}

inline INetworkMessageInternal* FindNetworkMessageById(int nMessageId)
{
    static const int index = ResolveIndex("INetworkMessages::FindNetworkMessageById", 27);
    return CALL_VIRTUAL(INetworkMessageInternal*, index, globals::networkMessages, nMessageId);
}

} // namespace counterstrikesharp::netmessages
