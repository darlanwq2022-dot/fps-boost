#include <Geode/Geode.hpp>
#include <Geode/modify/CCDirector.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>

using namespace geode::prelude;

namespace {
    // Measures CPU-side time spent inside CCDirector::drawScene.
    // This is NOT a GPU timer: asynchronous GPU work may continue after return.
    constexpr std::size_t kWindowSize = 120;
    using Clock = std::chrono::steady_clock;

    struct FrameWindow {
        std::array<double, kWindowSize> samples{};
        std::size_t count = 0;
        std::size_t next = 0;
        std::uint64_t totalFrames = 0;

        void push(double milliseconds) {
            samples[next] = milliseconds;
            next = (next + 1) % kWindowSize;
            if (count < kWindowSize) {
                ++count;
            }
            ++totalFrames;

            if (count == kWindowSize && totalFrames % kWindowSize == 0) {
                report();
            }
        }

        void report() const {
            auto sorted = samples;
            std::sort(sorted.begin(), sorted.end());

            double sum = 0.0;
            for (double value : samples) {
                sum += value;
            }

            const double average = sum / static_cast<double>(kWindowSize);
            const double p95 = sorted[static_cast<std::size_t>(0.95 * (kWindowSize - 1))];
            const double worst = sorted.back();
            const double estimatedFps = average > 0.0 ? 1000.0 / average : 0.0;

            log::info(
                "[GD FPS Core] CPU frame time (last {} frames): avg {:.3f} ms | p95 {:.3f} ms | max {:.3f} ms | reciprocal avg {:.1f} FPS",
                kWindowSize, average, p95, worst, estimatedFps
            );
        }
    };

    FrameWindow g_frameWindow;
}

class $modify(GDFPSCoreDirector, CCDirector) {
    void drawScene() {
        const auto start = Clock::now();
        CCDirector::drawScene();
        const auto end = Clock::now();

        const double elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();
        g_frameWindow.push(elapsedMs);
    }
};

$on_mod(Loaded) {
    log::info("GD FPS Core v0.2.0 loaded.");
    log::info("Frame profiler active: CPU-side CCDirector::drawScene duration, reported every 120 frames.");
    log::info("No gameplay, physics, collision, input, particle, or Wave Trail behavior is modified.");
}
