/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 Copyright (c) 2019-present Axmol Engine contributors (see AUTHORS.md).

 https://axmol.dev/

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "Includes.hpp"

#define USE_AUDIO_ENGINE 1

#if USE_AUDIO_ENGINE
#    include "audio/AudioEngine.h"
#endif

#include "utils/ModToggleManager.h"
#include "managers/DiscordManager.h"

using namespace ax;

static ax::Size designResolutionSize = ax::Size(480, 360);

AppDelegate::AppDelegate() {}

AppDelegate::~AppDelegate() {}

void AppDelegate::initGfxContextAttrs() {
    GfxContextAttrs gfxContextAttrs = {8, 8, 8, 8, 24, 8, 0};

    RenderView::setGfxContextAttrs(gfxContextAttrs);
}

bool AppDelegate::applicationDidFinishLaunching() {
    auto director = Director::getInstance();
    auto renderView   = director->getRenderView();
    if (!renderView)
    {
#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32) || (AX_TARGET_PLATFORM == AX_PLATFORM_MAC) || \
    (AX_TARGET_PLATFORM == AX_PLATFORM_LINUX)
        renderView = RenderViewImpl::createWithRect(
            "Cosmic Cities", ax::Rect(0, 0, 960, 720), 1.0f, true);
#else
        renderView = RenderViewImpl::create("Cosmic Cities");
#endif
        director->setRenderView(renderView);
    }

    director->setStatsDisplay(false); // set to true to see fps and whatnot

    director->setAnimationInterval(1.0f / 60); // 60 fps?

    renderView->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height,
                                    ResolutionPolicy::SHOW_ALL);

    InputManager::get().initialize();

    // Discover mods and write a handshake for MinHook-driven loaders
    cosmiccities::ModToggleManager::get().initialize("mods");
    
    auto scene = cosmiccities::LoadingLayer::scene();

    director->runWithScene(scene);

    return true;
}

void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::pauseAll();
#endif

    DiscordManager::instance().setPresence("Away", "Game minimized");
}

void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::resumeAll();
#endif

    DiscordManager::instance().setPresence("In Game", "Playing");
}

void AppDelegate::applicationWillQuit() {
    DiscordManager::instance().shutdown();
}

void AppDelegate::updateDiscord(float dt) {
    DiscordManager::instance().update();
}
