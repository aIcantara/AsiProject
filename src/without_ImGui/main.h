#pragma once

#include <Windows.h>

#include <kthook/kthook.hpp>

#include "sampapi/CNetGame.h"

namespace samp = sampapi::v037r1;

using CTimer__UpdateSignature = void(__cdecl*)();
kthook::kthook_simple<CTimer__UpdateSignature> CTimerHook{};