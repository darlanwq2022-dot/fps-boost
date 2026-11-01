#include <Geode/Geode.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/CCParticleSystem.hpp>
#include <Geode/modify/EffectGameObject.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/HardStreak.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/RingObject.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cmath>

using namespace geode::prelude;

namespace {
    using Clock = std::chrono::steady_clock;
    constexpr std::size_t kMaxWindow = 600;

    // Extreme profile: remove secondary visuals by default. HardStreak is a
    // separate trail renderer and is intentionally never disabled here.
    bool g_disableAllParticles = true;
    bool g_disableShaderUpdates = true;
    bool g_disableObjectGlow = true;
    bool g_disableObjectParticles = true;
    bool g_disablePlayerParticles = true;
    bool g_disablePlayerGlow = true;
    bool g_disableObjectShine = true;
    bool g_disableTriggerFlash = true;
    bool g_disableRingPulse = true;
    bool g_disablePlayerCircleEffects = true;
    bool g_disableSpiderDashVisual = true;
    bool g_disableDeathEffect = true;
    bool g_disableNonWaveTrail = true;
    bool g_disableGhostEffect = true;
    bool g_disableSceneGlitter = true;
    bool g_disableGravityTransitionVisual = true;
    bool g_disablePlayerFire = true;
    bool g_disablePlayerImpact = true;
    bool g_disablePlayerCompletion = true;
    bool g_disablePlayerSpawn = true;
    bool g_disableHighDetailObjects = true;
    bool g_hideDecorationObjects = true;
    bool g_disableCameraShake = true;
    bool g_disablePlayerBurst = true;

    // Wave Trail remains visible. This spacing only drops very close samples.
    // Balanced defaults preserve a smooth Wave Trail. More aggressive thinning
    // remains user-configurable, but the default no longer skips renderer frames.
    float g_wavePointSpacing = 0.15f;
    int g_waveSampleStep = 2;
    int g_waveRenderStep = 1;
    float g_waveMaxGap = 6.0f;
    std::size_t g_waveMaxPoints = 128;
    std::uint64_t g_wavePointsSkipped = 0;
    std::uint64_t g_waveRenderUpdatesSkipped = 0;

    // Profiling is OFF by default so the monitor itself does not add work
    // during normal extreme-profile gameplay.
    bool g_frameMonitorEnabled = false;
    bool g_profileGameUpdate = false;
    bool g_profilePlayerUpdate = false;
    bool g_profileParticleUpdate = false;
    bool g_profileVisibilityUpdate = false;
    bool g_profileWaveTrail = false;
    bool g_profileObjectSetup = false;
    bool g_profileObjectParsing = false;
    bool g_profileObjectComposition = false;
    PlayLayer* g_compositionProfiledLayer = nullptr;


    std::size_t g_reportInterval = 120;
    std::size_t g_profileSampleEveryNFrames = 4;
    std::uint64_t g_frameSequence = 0;

    struct FrameSample {
        bool profileSampled = false;
        double intervalMs = 0.0;
        double drawSceneWallMs = 0.0;
        double gameUpdateMs = 0.0;
        double playerUpdateMs = 0.0;
        double particleUpdateMs = 0.0;
        double visibilityUpdateMs = 0.0;
        double waveUpdateMs = 0.0;
        double objectSetupMs = 0.0;
        double objectParsingMs = 0.0;
        std::uint32_t waveUpdateCalls = 0;
        std::uint32_t objectSetupCalls = 0;
        std::uint32_t objectParsingCalls = 0;
        std::uint32_t playerUpdateCalls = 0;
        std::uint32_t particleUpdateCalls = 0;
        std::uint64_t activeParticleCountSum = 0;
        std::uint32_t levelObjectCount = 0;
    };

    FrameSample g_currentFrame;

    // Zero work when disabled; measures only selected paths when the user
    // explicitly enables diagnostics.
    class ScopedFrameTimer {
    public:
        ScopedFrameTimer(bool enabled, double& totalMs, std::uint32_t& calls)
          : m_enabled(enabled), m_totalMs(totalMs), m_calls(calls),
            m_start(enabled ? Clock::now() : Clock::time_point{}) {}
        ~ScopedFrameTimer() {
            if (!m_enabled) return;
            m_totalMs += std::chrono::duration<double, std::milli>(Clock::now() - m_start).count();
            ++m_calls;
        }
        ScopedFrameTimer(ScopedFrameTimer const&) = delete;
        ScopedFrameTimer& operator=(ScopedFrameTimer const&) = delete;
    private:
        bool m_enabled;
        double& m_totalMs;
        std::uint32_t& m_calls;
        Clock::time_point m_start;
    };

    struct FrameWindow {
        std::array<FrameSample, kMaxWindow> samples{};
        std::size_t count = 0;
        bool hasPreviousFrameStart = false;
        bool skipNextIntervalAfterReport = false;
        Clock::time_point previousFrameStart{};

        double markFrameStart(Clock::time_point now) {
            double intervalMs = 0.0;
            if (hasPreviousFrameStart) {
                intervalMs = std::chrono::duration<double, std::milli>(now - previousFrameStart).count();
            }
            previousFrameStart = now;
            hasPreviousFrameStart = true;
            if (skipNextIntervalAfterReport) {
                // Reporting uses synchronous logging after a frame. Discard
                // the next interval so the profiler's own log write is not
                // misreported as a game hitch.
                skipNextIntervalAfterReport = false;
                return 0.0;
            }
            return intervalMs;
        }

        void push(FrameSample const& sample) {
            if (sample.intervalMs <= 0.0) {
                return;
            }
            samples[count++] = sample;
            if (count >= g_reportInterval) {
                report();
                count = 0;
                skipNextIntervalAfterReport = true;
            }
        }

        static double percentile(std::array<double, kMaxWindow>& values, std::size_t size, double p) {
            std::sort(values.begin(), values.begin() + static_cast<std::ptrdiff_t>(size));
            auto index = static_cast<std::size_t>(p * static_cast<double>(size - 1));
            return values[std::min(index, size - 1)];
        }

        void report() const {
            if (count == 0) return;

            std::array<double, kMaxWindow> intervals{};
            std::array<double, kMaxWindow> sceneWall{};
            std::array<double, kMaxWindow> visibilityElapsed{};

            double intervalSum = 0.0;
            double sceneSum = 0.0;
            double gameSum = 0.0;
            double playerSum = 0.0;
            double particleSum = 0.0;
            double visibilitySum = 0.0;
            double waveSum = 0.0;
            double objectSetupSum = 0.0;
            double objectParsingSum = 0.0;
            std::uint64_t waveCalls = 0;
            std::uint64_t objectSetupCalls = 0;
            std::uint64_t objectParsingCalls = 0;
            std::uint64_t playerCalls = 0;
            std::uint64_t particleCalls = 0;
            std::uint64_t activeParticleSum = 0;
            std::uint32_t maxLevelObjectCount = 0;
            std::size_t profiledFrames = 0;
            std::size_t profiledIndex = 0;
            std::size_t over16ms = 0;
            std::size_t over8ms = 0;
            std::size_t over4ms = 0;

            for (std::size_t i = 0; i < count; ++i) {
                auto const& sample = samples[i];
                intervals[i] = sample.intervalMs;
                sceneWall[i] = sample.drawSceneWallMs;
                intervalSum += sample.intervalMs;
                sceneSum += sample.drawSceneWallMs;
                if (sample.intervalMs > 16.6667) ++over16ms;
                if (sample.intervalMs > 8.3333) ++over8ms;
                if (sample.intervalMs > 4.1667) ++over4ms;
                maxLevelObjectCount = std::max(maxLevelObjectCount, sample.levelObjectCount);

                if (sample.profileSampled) {
                    ++profiledFrames;
                    visibilityElapsed[profiledIndex] = sample.visibilityUpdateMs;
                    ++profiledIndex;
                    gameSum += sample.gameUpdateMs;
                    playerSum += sample.playerUpdateMs;
                    particleSum += sample.particleUpdateMs;
                    visibilitySum += sample.visibilityUpdateMs;
                    waveSum += sample.waveUpdateMs;
                    objectSetupSum += sample.objectSetupMs;
                    objectParsingSum += sample.objectParsingMs;
                    waveCalls += sample.waveUpdateCalls;
                    objectSetupCalls += sample.objectSetupCalls;
                    objectParsingCalls += sample.objectParsingCalls;
                    playerCalls += sample.playerUpdateCalls;
                    particleCalls += sample.particleUpdateCalls;
                    activeParticleSum += sample.activeParticleCountSum;
                }
            }

            auto intervalsSorted = intervals;
            auto sceneSorted = sceneWall;
            auto visibilitySorted = visibilityElapsed;
            std::sort(intervalsSorted.begin(), intervalsSorted.begin() + static_cast<std::ptrdiff_t>(count));
            // `count` is a FrameWindow member, not a local variable; copy its
            // current value into a local so the lambda can capture it safely.
            const std::size_t sampleCount = count;
            const auto percentileIndex = [sampleCount](double p) {
                return std::min(
                    static_cast<std::size_t>(p * static_cast<double>(sampleCount - 1)),
                    sampleCount - 1
                );
            };
            const double avgInterval = intervalSum / static_cast<double>(count);
            const double p95Interval = intervalsSorted[percentileIndex(0.95)];
            const double p99Interval = intervalsSorted[percentileIndex(0.99)];
            const double minInterval = intervalsSorted.front();
            const double maxInterval = intervalsSorted[count - 1];
            const double avgSceneWall = sceneSum / static_cast<double>(count);
            const double p95SceneWall = percentile(sceneSorted, count, 0.95);
            const double avgFpsEstimate = avgInterval > 0.0 ? 1000.0 / avgInterval : 0.0;

            // Correct 1% low: average the slowest ceil(1% of frame intervals),
            // then invert that duration. Reuse the sorted interval buffer.
            const std::size_t slowFrameCount = std::max<std::size_t>(1, (count + 99) / 100);
            double slowFrameSum = 0.0;
            for (std::size_t i = count - slowFrameCount; i < count; ++i) {
                slowFrameSum += intervalsSorted[i];
            }
            const double slowestAverageMs = slowFrameSum / static_cast<double>(slowFrameCount);
            const double onePercentLow = slowestAverageMs > 0.0 ? 1000.0 / slowestAverageMs : 0.0;
            const double profileDenominator = static_cast<double>(std::max<std::size_t>(1, profiledFrames));

            log::info(
                "[GD FPS Core] drawScene call cadence ({} frames): avg {:.3f} ms ({:.1f} estimated calls/s) | min {:.3f} | p95 {:.3f} | p99 {:.3f} | max {:.3f} | 1% low {:.1f} (slowest {} frames)",
                count, avgInterval, avgFpsEstimate, minInterval, p95Interval, p99Interval, maxInterval,
                onePercentLow, slowFrameCount
            );
            log::info(
                "[GD FPS Core] drawScene wall duration: avg {:.3f} ms | p95 {:.3f} ms | cadence thresholds: >16.67ms {} ({:.1f}%), >8.33ms {} ({:.1f}%), >4.17ms {} ({:.1f}%)",
                avgSceneWall, p95SceneWall,
                over16ms, 100.0 * static_cast<double>(over16ms) / static_cast<double>(count),
                over8ms, 100.0 * static_cast<double>(over8ms) / static_cast<double>(count),
                over4ms, 100.0 * static_cast<double>(over4ms) / static_cast<double>(count)
            );

            if (g_profileGameUpdate) {
                log::info(
                    "[GD FPS Core] GJBaseGameLayer::update inclusive elapsed: {:.4f} ms/profiled-frame average ({} sampled frames)",
                    gameSum / profileDenominator, profiledFrames
                );
            }
            if (g_profilePlayerUpdate) {
                log::info(
                    "[GD FPS Core] PlayerObject::update inclusive elapsed: {:.4f} ms/profiled-frame average | {} calls in sampled frames",
                    playerSum / profileDenominator, playerCalls
                );
            }
            if (g_profileParticleUpdate) {
                log::info(
                    "[GD FPS Core] CCParticleSystem::update elapsed: {:.4f} ms/profiled-frame average | {} calls | summed live-particle observations {}",
                    particleSum / profileDenominator, particleCalls, activeParticleSum
                );
            }
            if (g_profileVisibilityUpdate) {
                log::info(
                    "[GD FPS Core] PlayLayer::updateVisibility inclusive elapsed: {:.4f} ms/profiled-frame average | p95 {:.4f} ms | max loaded objects {}",
                    visibilitySum / profileDenominator,
                    profiledFrames ? percentile(visibilitySorted, profiledFrames, 0.95) : 0.0,
                    maxLevelObjectCount
                );
            }
            if (g_profileWaveTrail) {
                log::info(
                    "[GD FPS Core] HardStreak::updateStroke elapsed: {:.5f} ms/profiled-frame average | {} calls in sampled frames",
                    waveSum / profileDenominator, waveCalls
                );
            }
            if (g_profileObjectSetup) {
                log::info(
                    "[GD FPS Core] GameObject::customSetup elapsed: {:.5f} ms/profiled-frame average | {} calls in sampled frames",
                    objectSetupSum / profileDenominator, objectSetupCalls
                );
            }
            if (g_profileObjectParsing) {
                log::info(
                    "[GD FPS Core] GameObject::objectFromVector elapsed: {:.5f} ms/profiled-frame average | {} calls in sampled frames",
                    objectParsingSum / profileDenominator, objectParsingCalls
                );
            }
            log::info(
                "[GD FPS Core] Sampling: subsystem timers active on 1 of every {} frames; {} of {} frames sampled. Inclusive timers overlap and must not be added together.",
                g_profileSampleEveryNFrames, profiledFrames, count
            );
            log::info(
                "[GD FPS Core] Wave Trail: skipped input points {} | skipped renderer refreshes {}. drawScene cadence is not a GPU timer or guaranteed display-present FPS.",
                g_wavePointsSkipped, g_waveRenderUpdatesSkipped
            );
            g_wavePointsSkipped = 0;
            g_waveRenderUpdatesSkipped = 0;
        }
    };

    FrameWindow g_frameWindow;

    void stopParticle(cocos2d::CCParticleSystem* particle) {
        if (!particle) return;
        // Keep the Cocos scheduler intact. Unscheduling from inside setup or
        // from a particle's own update callback can invalidate scheduler state
        // on some engine builds. Stop emission and hide the visual node only.
        particle->stopSystem();
        if (particle->isVisible()) particle->setVisible(false);
    }

    void hideNode(cocos2d::CCNode* node) {
        if (node && node->isVisible()) {
            node->setVisible(false);
        }
    }

    bool shouldHideLevelObject(GameObject* object) {
        if (!object || object->m_isTrigger) return false;
        auto* manager = GameManager::get();
        // Keep editor previews and menus untouched; apply this profile only
        // while an actual PlayLayer exists.
        if (!manager || !manager->getPlayLayer()) return false;
        return (g_disableHighDetailObjects && object->m_isHighDetail) ||
            (g_hideDecorationObjects && (object->m_isDecoration || object->m_isDecoration2));
    }

    void applyObjectVisualProfile(GameObject* object) {
        if (!object) return;
        if (g_disableObjectGlow) hideNode(object->m_glowSprite);
        if (g_disableObjectParticles) stopParticle(object->m_particle);
    }

    void applyPlayerVisualProfile(PlayerObject* player) {
        if (!player) return;

        if (g_disablePlayerParticles) {
            stopParticle(player->m_playerGroundParticles);
            stopParticle(player->m_trailingParticles);
            stopParticle(player->m_shipClickParticles);
            stopParticle(player->m_vehicleGroundParticles);
            stopParticle(player->m_ufoClickParticles);
            stopParticle(player->m_robotBurstParticles);
            stopParticle(player->m_dashParticles);
            stopParticle(player->m_swingBurstParticles1);
            stopParticle(player->m_swingBurstParticles2);
            stopParticle(player->m_landParticles0);
            stopParticle(player->m_landParticles1);
        }

        if (g_disablePlayerBurst) {
            stopParticle(player->m_robotBurstParticles);
            stopParticle(player->m_swingBurstParticles1);
            stopParticle(player->m_swingBurstParticles2);
        }

        if (g_disablePlayerGlow) {
            hideNode(player->m_iconGlow);
            hideNode(player->m_vehicleGlow);
        }

        if (g_disableNonWaveTrail) {
            // The Wave Trail is m_waveTrail (HardStreak); these are separate trails.
            hideNode(player->m_regularTrail);
            hideNode(player->m_shipStreak);
        }

        if (g_disablePlayerFire) {
            hideNode(player->m_robotFire);
            hideNode(player->m_swingFireMiddle);
            hideNode(player->m_swingFireBottom);
            hideNode(player->m_swingFireTop);
            hideNode(player->m_dashSpritesContainer);
        }
    }

    void loadSettings() {
        auto* mod = Mod::get();
        g_disableAllParticles = mod->getSettingValue<bool>("disable-all-particles");
        g_disableShaderUpdates = mod->getSettingValue<bool>("disable-shader-updates");
        g_disableObjectGlow = mod->getSettingValue<bool>("disable-object-glow");
        g_disableObjectParticles = mod->getSettingValue<bool>("disable-object-particles");
        g_disablePlayerParticles = mod->getSettingValue<bool>("disable-player-particles");
        g_disablePlayerGlow = mod->getSettingValue<bool>("disable-player-glow");
        g_disableObjectShine = mod->getSettingValue<bool>("disable-object-shine");
        g_disableTriggerFlash = mod->getSettingValue<bool>("disable-trigger-flash");
        g_disableRingPulse = mod->getSettingValue<bool>("disable-ring-pulse");
        g_disablePlayerCircleEffects = mod->getSettingValue<bool>("disable-player-circle-effects");
        g_disableSpiderDashVisual = mod->getSettingValue<bool>("disable-spider-dash-visual");
        g_disableDeathEffect = mod->getSettingValue<bool>("disable-death-effect");
        g_disableNonWaveTrail = mod->getSettingValue<bool>("disable-non-wave-trail");
        g_disableGhostEffect = mod->getSettingValue<bool>("disable-ghost-effect");
        g_disableSceneGlitter = mod->getSettingValue<bool>("disable-scene-glitter");
        g_disableGravityTransitionVisual = mod->getSettingValue<bool>("disable-gravity-transition-visual");
        g_disablePlayerFire = mod->getSettingValue<bool>("disable-player-fire");
        g_disablePlayerImpact = mod->getSettingValue<bool>("disable-player-impact");
        g_disablePlayerCompletion = mod->getSettingValue<bool>("disable-player-completion");
        g_disablePlayerSpawn = mod->getSettingValue<bool>("disable-player-spawn");
        g_disableHighDetailObjects = mod->getSettingValue<bool>("disable-high-detail-objects");
        g_hideDecorationObjects = mod->getSettingValue<bool>("hide-decoration-objects");
        g_disableCameraShake = mod->getSettingValue<bool>("disable-camera-shake");
        g_disablePlayerBurst = mod->getSettingValue<bool>("disable-player-burst");
        g_wavePointSpacing = static_cast<float>(std::clamp<double>(
            mod->getSettingValue<double>("wave-point-spacing"), 0.0, 1.0
        ));
        g_waveSampleStep = static_cast<int>(std::clamp<int64_t>(
            mod->getSettingValue<int64_t>("wave-trail-sample-step"), 1, 4
        ));
        g_waveRenderStep = static_cast<int>(std::clamp<int64_t>(
            mod->getSettingValue<int64_t>("wave-trail-render-step"), 1, 3
        ));
        g_waveMaxGap = static_cast<float>(std::clamp<double>(
            mod->getSettingValue<double>("wave-trail-max-gap"), 1.0, 12.0
        ));
        g_waveMaxPoints = static_cast<std::size_t>(std::clamp<int64_t>(
            mod->getSettingValue<int64_t>("wave-trail-max-points"), 16, 256
        ));

        g_frameMonitorEnabled = mod->getSettingValue<bool>("frame-monitor");
        g_profileGameUpdate = mod->getSettingValue<bool>("profile-game-update");
        g_profilePlayerUpdate = mod->getSettingValue<bool>("profile-player-update");
        g_profileParticleUpdate = mod->getSettingValue<bool>("profile-particle-update");
        g_profileVisibilityUpdate = mod->getSettingValue<bool>("profile-visibility-update");
        g_profileWaveTrail = mod->getSettingValue<bool>("profile-wave-trail");
        g_profileObjectSetup = mod->getSettingValue<bool>("profile-object-setup");
        g_profileObjectParsing = mod->getSettingValue<bool>("profile-object-parsing");
        g_profileObjectComposition = mod->getSettingValue<bool>("profile-object-composition");
        auto interval = mod->getSettingValue<int64_t>("report-interval");
        g_reportInterval = static_cast<std::size_t>(std::clamp<int64_t>(interval, 60, static_cast<int64_t>(kMaxWindow)));
        auto sampleEvery = mod->getSettingValue<int64_t>("profile-sampling-interval");
        g_profileSampleEveryNFrames = static_cast<std::size_t>(std::clamp<int64_t>(sampleEvery, 1, 16));
    }

    bool profilingEnabled() {
        return g_frameMonitorEnabled || g_profileGameUpdate || g_profilePlayerUpdate || g_profileParticleUpdate || g_profileVisibilityUpdate || g_profileWaveTrail || g_profileObjectSetup || g_profileObjectParsing || g_profileObjectComposition;
    }
}

class $modify(GDFPSCoreDirector, cocos2d::CCDirector) {
    void drawScene() {
        if (!profilingEnabled()) {
            cocos2d::CCDirector::drawScene();
            return;
        }

        g_currentFrame = FrameSample{};
        g_currentFrame.profileSampled = (g_frameSequence++ % g_profileSampleEveryNFrames) == 0;
        const auto frameStart = Clock::now();
        g_currentFrame.intervalMs = g_frameWindow.markFrameStart(frameStart);

        cocos2d::CCDirector::drawScene();

        const auto frameEnd = Clock::now();
        g_currentFrame.drawSceneWallMs = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
        g_frameWindow.push(g_currentFrame);
    }
};

class $modify(GDFPSCoreGameLayer, GJBaseGameLayer) {
    void update(float dt) {
        if (!g_profileGameUpdate || !g_currentFrame.profileSampled) {
            GJBaseGameLayer::update(dt);
            return;
        }
        const auto start = Clock::now();
        GJBaseGameLayer::update(dt);
        g_currentFrame.gameUpdateMs += std::chrono::duration<double, std::milli>(Clock::now() - start).count();
    }

    void shakeCamera(float duration, float strength, float interval) {
        if (!g_disableCameraShake) {
            GJBaseGameLayer::shakeCamera(duration, strength, interval);
        }
    }

    // Keep the game's particle bookkeeping/update list intact. Individual
    // CCParticleSystem instances and particle-trigger visuals are suppressed
    // at their own entry points instead of bypassing the layer manager.
    void updateParticles(float dt) {
        GJBaseGameLayer::updateParticles(dt);
    }

    // Shader-layer animation is cosmetic and independent of player physics,
    // collision, trigger dispatch, and input handling.
    void updateShaderLayer(float dt) {
        if (!g_disableShaderUpdates) {
            GJBaseGameLayer::updateShaderLayer(dt);
        }
    }

    // The object-pointer overload is inline on Windows and cannot be hooked.
    // This non-inline overload is the safe visual-trigger entry point: it has
    // no return value, so suppressing it cannot leave callers with a null node.
    void spawnParticleTrigger(
        int particleID,
        cocos2d::CCPoint position,
        float rotation,
        float scale
    ) {
        if (!g_disableAllParticles) {
            GJBaseGameLayer::spawnParticleTrigger(particleID, position, rotation, scale);
        }
    }

};

class $modify(GDFPSCorePlayerObject, PlayerObject) {
    struct Fields {
        bool visualProfileApplied = false;
    };

    void update(float dt) {
        const bool measure = g_profilePlayerUpdate && g_currentFrame.profileSampled;
        const auto start = measure ? Clock::now() : Clock::time_point{};
        PlayerObject::update(dt);
        if (measure) {
            g_currentFrame.playerUpdateMs += std::chrono::duration<double, std::milli>(Clock::now() - start).count();
            ++g_currentFrame.playerUpdateCalls;
        }
        if (!m_fields->visualProfileApplied) {
            applyPlayerVisualProfile(this);
            m_fields->visualProfileApplied = true;
        }
    }

    void updatePlayerArt() {
        PlayerObject::updatePlayerArt();
        m_fields->visualProfileApplied = false;
        applyPlayerVisualProfile(this);
        m_fields->visualProfileApplied = true;
    }

    void updateGlowColor() {
        if (g_disablePlayerGlow) {
            hideNode(m_iconGlow);
            hideNode(m_vehicleGlow);
            return;
        }
        PlayerObject::updateGlowColor();
    }

    void updatePlayerGlow() {
        if (g_disablePlayerGlow) {
            hideNode(m_iconGlow);
            hideNode(m_vehicleGlow);
            return;
        }
        PlayerObject::updatePlayerGlow();
    }

    void updateSwingFire() {
        if (!g_disablePlayerFire) PlayerObject::updateSwingFire();
    }

    void createFadeOutDartStreak() {
        if (!g_disableNonWaveTrail) {
            PlayerObject::createFadeOutDartStreak();
        }
    }

    void playBumpEffect(int objectType, GameObject* object) {
        if (!g_disablePlayerImpact) PlayerObject::playBumpEffect(objectType, object);
    }

    void playCompleteEffect(bool noEffects, bool instant) {
        if (!g_disablePlayerCompletion) PlayerObject::playCompleteEffect(noEffects, instant);
    }

    void playDeathEffect() {
        if (!g_disableDeathEffect) PlayerObject::playDeathEffect();
    }

    void playSpawnEffect() {
        if (!g_disablePlayerSpawn) PlayerObject::playSpawnEffect();
    }

    void playSpiderDashEffect(cocos2d::CCPoint from, cocos2d::CCPoint to) {
        if (!g_disableSpiderDashVisual) PlayerObject::playSpiderDashEffect(from, to);
    }

    void toggleGhostEffect(GhostType type) {
        if (g_disableGhostEffect && type != GhostType::Disabled) {
            PlayerObject::toggleGhostEffect(GhostType::Disabled);
            return;
        }
        PlayerObject::toggleGhostEffect(type);
    }

    void spawnCircle() {
        if (!g_disablePlayerCircleEffects) PlayerObject::spawnCircle();
    }

    void spawnDualCircle() {
        if (!g_disablePlayerCircleEffects) PlayerObject::spawnDualCircle();
    }

    void spawnPortalCircle(cocos2d::ccColor3B color, float startRadius) {
        if (!g_disablePlayerCircleEffects) PlayerObject::spawnPortalCircle(color, startRadius);
    }

    void spawnScaleCircle() {
        if (!g_disablePlayerCircleEffects) PlayerObject::spawnScaleCircle();
    }
};

class $modify(GDFPSCoreParticleSystem, cocos2d::CCParticleSystem) {
    struct Fields {
        bool suppressionApplied = false;
    };

    void update(float dt) {
        if (g_disableAllParticles) {
            if (!m_fields->suppressionApplied) {
                stopSystem();
                if (isVisible()) setVisible(false);
                // Keep scheduler bookkeeping untouched. The hidden emitter is
                // stopped once; future callbacks return without simulation.
                m_fields->suppressionApplied = true;
            }
            return;
        }
        m_fields->suppressionApplied = false;
        if (!g_profileParticleUpdate || !g_currentFrame.profileSampled) {
            cocos2d::CCParticleSystem::update(dt);
            return;
        }
        const auto start = Clock::now();
        cocos2d::CCParticleSystem::update(dt);
        g_currentFrame.particleUpdateMs += std::chrono::duration<double, std::milli>(Clock::now() - start).count();
        ++g_currentFrame.particleUpdateCalls;
        g_currentFrame.activeParticleCountSum += getParticleCount();
    }


};

class $modify(GDFPSCoreGameObject, GameObject) {
    struct Fields {
        bool secondaryVisualsSuppressed = false;
    };

    cocos2d::CCParticleSystemQuad* createAndAddParticle(int objectType, char const* plistName, int tag, cocos2d::tCCPositionType positionType) {
        // Preserve the engine's normal factory contract. Some callers retain
        // and configure the returned emitter, so returning nullptr here can
        // break level initialization. Suppress it only after creation.
        auto* particle = GameObject::createAndAddParticle(objectType, plistName, tag, positionType);
        if (particle && (g_disableAllParticles || g_disableObjectParticles)) {
            stopParticle(particle);
        }
        return particle;
    }

    static GameObject* objectFromVector(
        gd::vector<gd::string>& propValues,
        gd::vector<void*>& propIsPresent,
        GJBaseGameLayer* gameLayer,
        bool lowDetail
    ) {
        // Only force LDM for the active PlayLayer, never for the editor or menus.
        auto* manager = GameManager::get();
        if (g_disableHighDetailObjects && gameLayer && manager &&
            static_cast<GJBaseGameLayer*>(manager->getPlayLayer()) == gameLayer) {
            // Avoid repeating writes for every parsed object. The engine's
            // native LDM parser still decides which HD objects are omitted.
            if (!gameLayer->m_lowDetailMode) gameLayer->m_lowDetailMode = true;
            if (!gameLayer->m_extraLDM) gameLayer->m_extraLDM = true;
            lowDetail = true;
        }
        ScopedFrameTimer parseTimer(
            g_profileObjectParsing && g_currentFrame.profileSampled, g_currentFrame.objectParsingMs, g_currentFrame.objectParsingCalls
        );
        return GameObject::objectFromVector(propValues, propIsPresent, gameLayer, lowDetail);
    }

    void applyProfileOnce() {
        if (!m_fields->secondaryVisualsSuppressed) {
            applyObjectVisualProfile(this);
            m_fields->secondaryVisualsSuppressed = true;
        }
    }

    void setVisible(bool visible) {
        if (visible && shouldHideLevelObject(this)) {
            GameObject::setVisible(false);
            applyProfileOnce();
            return;
        }
        GameObject::setVisible(visible);
        if (visible) applyProfileOnce();
    }

    void customSetup() {
        ScopedFrameTimer setupTimer(
            g_profileObjectSetup && g_currentFrame.profileSampled, g_currentFrame.objectSetupMs, g_currentFrame.objectSetupCalls
        );
        GameObject::customSetup();
        applyProfileOnce();
        if (shouldHideLevelObject(this)) {
            GameObject::setVisible(false);
        }
    }

    void addGlow(gd::string frame) {
        if (!g_disableObjectGlow) GameObject::addGlow(frame);
    }

    void createGlow(gd::string frame) {
        if (!g_disableObjectGlow) GameObject::createGlow(frame);
    }

    void setGlowColor(cocos2d::ccColor3B const& color) {
        if (g_disableObjectGlow) {
            hideNode(m_glowSprite);
            return;
        }
        GameObject::setGlowColor(color);
    }

    void claimParticle() {
        GameObject::claimParticle();
        if (g_disableObjectParticles) stopParticle(m_particle);
    }


    void playShineEffect() {
        if (!g_disableObjectShine) GameObject::playShineEffect();
    }
};

class $modify(GDFPSCoreHardStreak, HardStreak) {
    struct Fields {
        cocos2d::CCPoint lastRawPoint = { 0.f, 0.f };
        cocos2d::CCPoint lastKeptPoint = { 0.f, 0.f };
        bool hasLastRawPoint = false;
        bool hasLastKeptPoint = false;
        int samplesSinceKept = 0;
        int renderFrameCounter = 0;
    };

    void updateStroke(float dt) {
        const int step = std::clamp(g_waveRenderStep, 1, 3);
        if (step > 1) {
            ++m_fields->renderFrameCounter;
            if ((m_fields->renderFrameCounter % step) != 0) {
                ++g_waveRenderUpdatesSkipped;
                return;
            }
        }
        ScopedFrameTimer waveTimer(
            g_profileWaveTrail && g_currentFrame.profileSampled, g_currentFrame.waveUpdateMs, g_currentFrame.waveUpdateCalls
        );
        HardStreak::updateStroke(dt);
    }

    void addPoint(cocos2d::CCPoint point) {
        const int step = std::clamp(g_waveSampleStep, 1, 4);
        const float spacing = std::max(0.f, g_wavePointSpacing);
        const float maxGap = std::max(1.f, g_waveMaxGap);
        const float spacingSq = spacing * spacing;
        const float maxGapSq = maxGap * maxGap;

        auto keepPoint = [this, spacingSq](cocos2d::CCPoint const& p) {
            if (m_fields->hasLastKeptPoint) {
                const float dx = p.x - m_fields->lastKeptPoint.x;
                const float dy = p.y - m_fields->lastKeptPoint.y;
                if (dx * dx + dy * dy < spacingSq) {
                    ++g_wavePointsSkipped;
                    return false;
                }
            }
            HardStreak::addPoint(p);
            if (m_pointArray) {
                const auto maxPoints = static_cast<unsigned int>(g_waveMaxPoints);
                while (m_pointArray->count() > maxPoints) {
                    m_pointArray->removeObjectAtIndex(0);
                }
            }
            m_fields->lastKeptPoint = p;
            m_fields->hasLastKeptPoint = true;
            m_fields->samplesSinceKept = 0;
            return true;
        };

        if (!m_fields->hasLastRawPoint || step == 1) {
            keepPoint(point);
            m_fields->lastRawPoint = point;
            m_fields->hasLastRawPoint = true;
            return;
        }

        // Preserve sharper turns by inserting the previous raw sample before decimating.
        bool corner = false;
        if (m_fields->hasLastKeptPoint) {
            const float ax = m_fields->lastRawPoint.x - m_fields->lastKeptPoint.x;
            const float ay = m_fields->lastRawPoint.y - m_fields->lastKeptPoint.y;
            const float bx = point.x - m_fields->lastRawPoint.x;
            const float by = point.y - m_fields->lastRawPoint.y;
            const float aLenSq = ax * ax + ay * ay;
            const float bLenSq = bx * bx + by * by;
            if (aLenSq > 0.0001f && bLenSq > 0.0001f) {
                const float dot = ax * bx + ay * by;
                const float cross = ax * by - ay * bx;
                // Approximate a turn-angle test without sqrt/division.
                // Preserve reversals and turns above roughly 25 degrees.
                corner = dot <= 0.f || (cross * cross) > (0.18f * aLenSq * bLenSq);
            }
        }

        if (corner && m_fields->hasLastKeptPoint) {
            keepPoint(m_fields->lastRawPoint);
        }

        ++m_fields->samplesSinceKept;
        bool forceKeep = !m_fields->hasLastKeptPoint || m_fields->samplesSinceKept >= step;
        if (m_fields->hasLastKeptPoint) {
            const float dx = point.x - m_fields->lastKeptPoint.x;
            const float dy = point.y - m_fields->lastKeptPoint.y;
            if (dx * dx + dy * dy >= maxGapSq) forceKeep = true;
        }

        if (corner || forceKeep) {
            keepPoint(point);
        } else {
            ++g_wavePointsSkipped;
        }

        m_fields->lastRawPoint = point;
        m_fields->hasLastRawPoint = true;
    }

    void stopStroke() {
        m_fields->hasLastRawPoint = false;
        m_fields->hasLastKeptPoint = false;
        m_fields->samplesSinceKept = 0;
        m_fields->renderFrameCounter = 0;
        HardStreak::stopStroke();
    }
};

class $modify(GDFPSCorePlayLayer, PlayLayer) {
    // Deliberately do not hook PlayLayer::init or invoke layer-visibility
    // toggles during construction. The engine owns this fragile lifecycle.
    void updateVisibility(float dt) {
        if (!g_profileVisibilityUpdate || !g_currentFrame.profileSampled) {
            PlayLayer::updateVisibility(dt);
        } else {
            const auto start = Clock::now();
            PlayLayer::updateVisibility(dt);
            g_currentFrame.visibilityUpdateMs += std::chrono::duration<double, std::milli>(Clock::now() - start).count();
            if (m_objects) {
                g_currentFrame.levelObjectCount = static_cast<std::uint32_t>(m_objects->count());
            }
        }

        // Optional one-shot read-only inventory for Bismuth renderer research.
        // It uses the already-hooked PlayLayer updateVisibility entry point and
        // never alters object visibility, sorting, render order, or gameplay.
        if (g_profileObjectComposition && this != g_compositionProfiledLayer && m_objects) {
            const auto objectCount = m_objects->count();
            if (objectCount > 0) {
                const auto scanStart = Clock::now();
                std::uint32_t decorations = 0;
                std::uint32_t triggers = 0;
                std::uint32_t highDetail = 0;
                std::uint32_t visible = 0;
                for (unsigned int i = 0; i < objectCount; ++i) {
                    auto* object = static_cast<GameObject*>(m_objects->objectAtIndex(i));
                    if (!object) continue;
                    if (object->m_isDecoration || object->m_isDecoration2) ++decorations;
                    if (object->m_isTrigger) ++triggers;
                    if (object->m_isHighDetail) ++highDetail;
                    if (object->isVisible()) ++visible;
                }
                const double scanMs = std::chrono::duration<double, std::milli>(Clock::now() - scanStart).count();
                g_compositionProfiledLayer = this;
                log::info(
                    "[GD FPS Core] Bismuth preflight object inventory: total {} | visible {} | decoration-marked {} | triggers {} | high-detail {} | one-shot scan {:.3f} ms. Read-only; no objects were culled or modified.",
                    objectCount, visible, decorations, triggers, highDetail, scanMs
                );
            }
        }
    }

    void toggleGlitter(bool visible) {
        if (!g_disableSceneGlitter) PlayLayer::toggleGlitter(visible);
    }

    void playGravityEffect(bool flip) {
        if (!g_disableGravityTransitionVisual) PlayLayer::playGravityEffect(flip);
    }
};

class $modify(GDFPSCoreRingObject, RingObject) {
    void spawnCircle() {
        if (!g_disableRingPulse) RingObject::spawnCircle();
    }
};

class $modify(GDFPSCoreEffectObject, EffectGameObject) {
    void playTriggerEffect() {
        if (!g_disableTriggerFlash) EffectGameObject::playTriggerEffect();
    }
};

$on_mod(Loaded) {
    loadSettings();
    log::info("GD FPS Core v0.9.2 Bismuth Research Build loaded.");
    log::info("ULTRA profile: supported particle triggers and emitters, shader-layer animation, and player/object secondary visuals are suppressed while preserving the engine particle bookkeeping and node lifecycle.");
    log::info("Object particle factory calls preserve their original return values; optional emitters are stopped and hidden after creation, and the particle update hook avoids simulating suppressed effects.");
    log::info("Native Extra LDM filters High Detail objects before runtime-list insertion; gameplay decorations are hidden without touching trigger objects.");
    log::info("Wave Trail stays enabled. Default settings preserve every renderer refresh and use conservative point decimation.");
    log::info("Profiler reports drawScene call cadence and wall duration, plus opt-in subsystem elapsed-time samples and a one-shot object-composition inventory. It does not claim GPU time or guaranteed display-present FPS.");
    log::info("PlayLayer construction and Cocos scheduler bookkeeping are left to the game. Gameplay update, trigger execution, collision, physics and input loops are preserved. Active gameplay objects still require CPU/memory even when secondary visuals are removed.");
}
