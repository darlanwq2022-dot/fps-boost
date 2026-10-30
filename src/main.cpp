#include <Geode/Geode.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>

using namespace geode::prelude;

namespace {
    constexpr std::size_t kWindowSize = 120;
    using Clock = std::chrono::steady_clock;

    // These values are loaded once. The corresponding Geode settings require a restart,
    // so no attempt is made to restore game-owned visibility state at runtime.
    bool g_disableObjectGlow = true;
    bool g_disableObjectParticles = true;
    bool g_disablePlayerParticles = true;
    bool g_disablePlayerGlow = true;
    bool g_frameMonitorEnabled = true;

    template <class T>
    void hideNode(T* node) {
        if (node && node->isVisible()) {
            node->setVisible(false);
        }
    }

    struct FrameWindow {
        std::array<double, kWindowSize> frameIntervalsMs{};
        std::array<double, kWindowSize> drawCpuMs{};
        std::size_t count = 0;
        std::size_t next = 0;
        std::uint64_t totalSamples = 0;
        bool hasPreviousFrameStart = false;
        Clock::time_point previousFrameStart{};

        double markFrameStart(Clock::time_point now) {
            double intervalMs = 0.0;
            if (hasPreviousFrameStart) {
                intervalMs = std::chrono::duration<double, std::milli>(now - previousFrameStart).count();
            }
            previousFrameStart = now;
            hasPreviousFrameStart = true;
            return intervalMs;
        }

        void push(double intervalMs, double cpuMs) {
            if (intervalMs <= 0.0) {
                return;
            }

            frameIntervalsMs[next] = intervalMs;
            drawCpuMs[next] = cpuMs;
            next = (next + 1) % kWindowSize;
            if (count < kWindowSize) {
                ++count;
            }
            ++totalSamples;

            if (count == kWindowSize && totalSamples % kWindowSize == 0) {
                report();
            }
        }

        static double percentile(const std::array<double, kWindowSize>& sorted, double percentileValue) {
            const auto index = static_cast<std::size_t>(percentileValue * static_cast<double>(kWindowSize - 1));
            return sorted[std::min(index, kWindowSize - 1)];
        }

        void report() const {
            auto sortedIntervals = frameIntervalsMs;
            auto sortedCpu = drawCpuMs;
            std::sort(sortedIntervals.begin(), sortedIntervals.end());
            std::sort(sortedCpu.begin(), sortedCpu.end());

            double intervalSum = 0.0;
            double cpuSum = 0.0;
            for (std::size_t i = 0; i < kWindowSize; ++i) {
                intervalSum += frameIntervalsMs[i];
                cpuSum += drawCpuMs[i];
            }

            const double averageInterval = intervalSum / static_cast<double>(kWindowSize);
            const double averageCpu = cpuSum / static_cast<double>(kWindowSize);
            const double p95Interval = percentile(sortedIntervals, 0.95);
            const double p99Interval = percentile(sortedIntervals, 0.99);
            const double worstInterval = sortedIntervals.back();
            const double p95Cpu = percentile(sortedCpu, 0.95);
            const double callRate = averageInterval > 0.0 ? 1000.0 / averageInterval : 0.0;
            const double lowOnePercentEstimate = p99Interval > 0.0 ? 1000.0 / p99Interval : 0.0;

            log::info(
                "[GD FPS Core] Frame-call interval ({} samples): avg {:.3f} ms | p95 {:.3f} ms | p99 {:.3f} ms | max {:.3f} ms | avg call-rate {:.1f}/s | estimated 1% low {:.1f}/s | drawScene CPU avg {:.3f} ms | CPU p95 {:.3f} ms",
                kWindowSize,
                averageInterval,
                p95Interval,
                p99Interval,
                worstInterval,
                callRate,
                lowOnePercentEstimate,
                averageCpu,
                p95Cpu
            );
        }
    };

    FrameWindow g_frameWindow;

    void applyObjectVisualProfile(GameObject* object) {
        if (!object) {
            return;
        }

        if (g_disableObjectGlow) {
            hideNode(object->m_glowSprite);
        }
        if (g_disableObjectParticles) {
            hideNode(object->m_particle);
        }
    }

    void applyPlayerVisualProfile(PlayerObject* player) {
        if (!player || !PlayLayer::get()) {
            return;
        }

        if (g_disablePlayerParticles) {
            hideNode(player->m_playerGroundParticles);
            hideNode(player->m_shipClickParticles);
            hideNode(player->m_vehicleGroundParticles);
            hideNode(player->m_ufoClickParticles);
            hideNode(player->m_robotBurstParticles);
            hideNode(player->m_dashParticles);
            hideNode(player->m_swingBurstParticles1);
            hideNode(player->m_swingBurstParticles2);
            hideNode(player->m_landParticles0);
            hideNode(player->m_landParticles1);

            // Intentionally preserved: m_trailingParticles, m_regularTrail,
            // m_shipStreak and m_waveTrail (the visible Wave Trail).
        }

        if (g_disablePlayerGlow) {
            hideNode(player->m_iconGlow);
            hideNode(player->m_vehicleGlow);
        }
    }
}

class $modify(GDFPSCoreDirector, CCDirector) {
    void drawScene() {
        if (!g_frameMonitorEnabled) {
            CCDirector::drawScene();
            return;
        }

        const auto frameStart = Clock::now();
        const double frameIntervalMs = g_frameWindow.markFrameStart(frameStart);

        CCDirector::drawScene();

        const auto frameEnd = Clock::now();
        const double drawCpuMs = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
        g_frameWindow.push(frameIntervalMs, drawCpuMs);
    }
};

class $modify(GDFPSCoreGameObject, GameObject) {
    void customSetup() {
        GameObject::customSetup();
        applyObjectVisualProfile(this);
    }

    void setGlowColor(cocos2d::ccColor3B const& color) {
        GameObject::setGlowColor(color);
        if (g_disableObjectGlow) {
            hideNode(m_glowSprite);
        }
    }

    void claimParticle() {
        GameObject::claimParticle();
        if (g_disableObjectParticles) {
            hideNode(m_particle);
        }
    }
};

class $modify(GDFPSCorePlayerObject, PlayerObject) {
    void update(float dt) {
        PlayerObject::update(dt);
        applyPlayerVisualProfile(this);
    }
};

$on_mod(Loaded) {
    auto* mod = Mod::get();
    g_disableObjectGlow = mod->getSettingValue<bool>("disable-object-glow");
    g_disableObjectParticles = mod->getSettingValue<bool>("disable-object-particles");
    g_disablePlayerParticles = mod->getSettingValue<bool>("disable-player-particles");
    g_disablePlayerGlow = mod->getSettingValue<bool>("disable-player-glow");
    g_frameMonitorEnabled = mod->getSettingValue<bool>("frame-monitor");

    log::info("GD FPS Core v0.3.0 loaded.");
    log::info(
        "Extreme profile: object glow={}, object particles={}, player particles={}, player glow={}, frame monitor={}",
        g_disableObjectGlow,
        g_disableObjectParticles,
        g_disablePlayerParticles,
        g_disablePlayerGlow,
        g_frameMonitorEnabled
    );
    log::info("Wave Trail and player streaks are preserved. Physics, collisions, object timing and inputs are not modified.");
    log::info("Frame-call rate is an estimate based on drawScene call intervals; it is not a GPU timer or guaranteed display-present FPS.");
}
