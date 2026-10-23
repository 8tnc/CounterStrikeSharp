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

#include <eiface.h>

#include "core/gameconfig.h"
#include "core/globals.h"
#include "utils/virtual.h"

// CS2 1.41.9.2 inserted a virtual into IVEngineServer2 before SetTimescale, which the pinned hl2sdk does not
// reflect yet, so every method from there on is off by one. Calling these through the SDK header lands on the
// wrong function (e.g. GetClientSteamID -> GetAppID), so resolve them by index from gamedata instead and fall
// back to the SDK call when no index is configured for the current platform.
namespace counterstrikesharp::engine_vfuncs {

inline const CSteamID* GetClientSteamID(CPlayerSlot slot)
{
    static const int index = globals::gameConfig->GetOffset("IVEngineServer2::GetClientSteamID");
    if (index < 0) return globals::engine->GetClientSteamID(slot);
    return CALL_VIRTUAL(const CSteamID*, index, globals::engine, slot.Get());
}

inline bool IsClientFullyAuthenticated(CPlayerSlot slot)
{
    static const int index = globals::gameConfig->GetOffset("IVEngineServer2::IsClientFullyAuthenticated");
    if (index < 0) return globals::engine->IsClientFullyAuthenticated(slot);
    return CALL_VIRTUAL(bool, index, globals::engine, slot.Get());
}

inline void SetFakeClientConVarValue(CPlayerSlot slot, const char* cvar, const char* value)
{
    static const int index = globals::gameConfig->GetOffset("IVEngineServer2::SetFakeClientConVarValue");
    if (index < 0) return globals::engine->SetFakeClientConVarValue(slot, cvar, value);
    CALL_VIRTUAL(void, index, globals::engine, slot.Get(), cvar, value);
}

} // namespace counterstrikesharp::engine_vfuncs
