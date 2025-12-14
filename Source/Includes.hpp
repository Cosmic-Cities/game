#pragma once

#include <fstream>
#include <sstream>
#include <string>

#include <nlohmann/json.hpp>
#include "axmol.h"
#include <AL/al.h>

#include "AppDelegate.h"

#include "layers/LoadingLayer.h"
#include "layers/MenuLayer.h"
#include "layers/ProgressionLayer.h"

#include "utils/Starfield.h"
#include "utils/InputManager.h"

#include "managers/LocalisationManager.h"

#include "extras/MenuItemExtra.h"

#include "platform/RenderViewImpl.h"

#include "audio/AudioEngine.h"