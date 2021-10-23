#pragma once

#define _SILENCE_ALL_CXX20_DEPRECATION_WARNINGS
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#include "sqlite3.h"

#include <assert.h>

#include <codecvt>
#include <filesystem>
#include <locale> 
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <vector>
