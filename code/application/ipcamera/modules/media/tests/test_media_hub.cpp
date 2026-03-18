/**
 * @file test_media_hub.cpp
 * @brief Unit tests for VideoRingBuffer and VideoFrameConsumer
 *
 * Tests the core ring buffer and consumer logic without HDAL dependencies.
 * MediaHub singleton is NOT tested here (requires HDAL pipeline).
 */

#include "ipcam/media_hub.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <cassert>
#include <cstring>
#include <chrono>
#include <vector>
#include <numeric>

using namespace ipcam::media;

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    static void test_##name(); \
    struct TestRegistrar_##name { \
        TestRegistrar_##name() { \
            std::cout << "  Running: " #name "..." << std::flush; \
            try { \
                test_##name(); \
                tests_passed++; \
                std::cout << " PASSED" << std::endl; \
            } catch (const std::exception& e) { \
                tests_failed++; \
                std::cout << " FAILED: " << e.what() << std::endl; \
            } catch (...) { \
                tests_failed++; \
                std::cout << " FAILED (unknown exception)" << std::endl; \
            } \
        } \
    } s_reg_##name; \
    static void test_##name()

#define ASSERT_TRUE(expr) \
    if (!(expr)) throw std::runtime_error("Assertion failed: " #expr)

#define ASSERT_FALSE(expr) \
    if (expr) throw std::runtime_error("Assertion failed (expected false): " #expr)

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        std::ostringstream oss; \
        oss << "Assertion failed: " #a " == " #b " (" << (a) << " != " << (b) << ")"; \
        throw std::runtime_error(oss.str()); \
    }

// ============================================================================
// VideoRingBuffer Tests
// ============================================================================

TEST(ring_buffer_initial_state) {
    VideoRingBuffer rb(16);
    ASSERT_EQ(rb.GetLatestSequence(), 0u);
    ASSERT_EQ(rb.GetSlotCount(), 16u);
}

TEST(ring_buffer_single_write_read) {
    VideoRingBuffer rb(16);

    // Write a frame
    uint8_t data[] = {0x00, 0x00, 0x00, 0x01, 0x65, 0xAB, 0xCD};
    uint64_t seq = rb.Write(data, sizeof(data),
                            1000, 2000, 0, true, VideoCodecType::kH264,
                            nullptr, 0, nullptr, 0, nullptr, 0);

    ASSERT_EQ(seq, 1u);
    ASSERT_EQ(rb.GetLatestSequence(), 1u);

    // Read it back
    VideoFrame frame;
    ASSERT_TRUE(rb.Read(1, frame));
    ASSERT_EQ(frame.data.size(), sizeof(data));
    ASSERT_TRUE(std::memcmp(frame.data.data(), data, sizeof(data)) == 0);
    ASSERT_EQ(frame.timestamp_us, 1000u);
    ASSERT_EQ(frame.capture_time_ms, 2000u);
    ASSERT_EQ(frame.channel_id, 0);
    ASSERT_TRUE(frame.is_keyframe);
    ASSERT_EQ(frame.sequence, 1u);
}

TEST(ring_buffer_multiple_writes) {
    VideoRingBuffer rb(8);

    for (int i = 1; i <= 5; i++) {
        uint8_t data[] = {static_cast<uint8_t>(i)};
        uint64_t seq = rb.Write(data, 1, i * 100, i * 200,
                                0, (i % 3 == 1), VideoCodecType::kH264,
                                nullptr, 0, nullptr, 0, nullptr, 0);
        ASSERT_EQ(seq, static_cast<uint64_t>(i));
    }

    ASSERT_EQ(rb.GetLatestSequence(), 5u);

    // Read specific frames
    for (int i = 1; i <= 5; i++) {
        VideoFrame frame;
        ASSERT_TRUE(rb.Read(i, frame));
        ASSERT_EQ(frame.data.size(), 1u);
        ASSERT_EQ(frame.data[0], static_cast<uint8_t>(i));
    }
}

TEST(ring_buffer_overwrite_oldest) {
    // Small buffer: 4 slots
    VideoRingBuffer rb(4);

    // Write 6 frames (slots 1-4 get written, then 5-6 overwrite 1-2)
    for (int i = 1; i <= 6; i++) {
        uint8_t data[] = {static_cast<uint8_t>(i)};
        rb.Write(data, 1, 0, 0, 0, false, VideoCodecType::kH264,
                 nullptr, 0, nullptr, 0, nullptr, 0);
    }

    ASSERT_EQ(rb.GetLatestSequence(), 6u);
    ASSERT_EQ(rb.GetOldestSequence(), 3u);

    // Frames 1 and 2 should be gone (overwritten)
    VideoFrame frame;
    ASSERT_FALSE(rb.Read(1, frame));
    ASSERT_FALSE(rb.Read(2, frame));

    // Frames 3-6 should be readable
    ASSERT_TRUE(rb.Read(3, frame));
    ASSERT_TRUE(rb.Read(4, frame));
    ASSERT_TRUE(rb.Read(5, frame));
    ASSERT_TRUE(rb.Read(6, frame));
}

TEST(ring_buffer_read_nonexistent) {
    VideoRingBuffer rb(8);
    VideoFrame frame;

    // Nothing written yet
    ASSERT_FALSE(rb.Read(1, frame));
    ASSERT_FALSE(rb.Read(0, frame));
}

TEST(ring_buffer_sps_pps_propagation) {
    VideoRingBuffer rb(8);

    uint8_t sps[] = {0x67, 0x42, 0x00, 0x1E};
    uint8_t pps[] = {0x68, 0xCE, 0x38, 0x80};
    uint8_t data[] = {0x00, 0x00, 0x00, 0x01, 0x65};

    rb.Write(data, sizeof(data), 0, 0, 0, true, VideoCodecType::kH264,
             sps, sizeof(sps), pps, sizeof(pps), nullptr, 0);

    VideoFrame frame;
    ASSERT_TRUE(rb.Read(1, frame));
    ASSERT_EQ(frame.sps_size, sizeof(sps));
    ASSERT_TRUE(std::memcmp(frame.sps, sps, sizeof(sps)) == 0);
    ASSERT_EQ(frame.pps_size, sizeof(pps));
    ASSERT_TRUE(std::memcmp(frame.pps, pps, sizeof(pps)) == 0);
    ASSERT_EQ(frame.vps_size, 0u);
}

TEST(ring_buffer_nal_packs) {
    VideoRingBuffer rb(8);

    // Simulate a frame with 3 NAL packs
    uint8_t data[300];
    std::memset(data, 0xAB, sizeof(data));

    std::vector<VideoFrame::NalPack> packs = {
        {0, 100, false},
        {100, 100, true},
        {200, 100, false}
    };

    rb.Write(data, sizeof(data), 0, 0, 0, true, VideoCodecType::kH264,
             nullptr, 0, nullptr, 0, nullptr, 0, packs);

    VideoFrame frame;
    ASSERT_TRUE(rb.Read(1, frame));
    ASSERT_EQ(frame.nal_packs.size(), 3u);
    ASSERT_EQ(frame.nal_packs[0].offset, 0u);
    ASSERT_EQ(frame.nal_packs[0].size, 100u);
    ASSERT_FALSE(frame.nal_packs[0].is_idr);
    ASSERT_TRUE(frame.nal_packs[1].is_idr);
    ASSERT_EQ(frame.nal_packs[2].offset, 200u);
}

TEST(ring_buffer_find_keyframe_before) {
    VideoRingBuffer rb(16);

    // Write frames: 1=key, 2=P, 3=P, 4=key, 5=P, 6=P, 7=P
    for (int i = 1; i <= 7; i++) {
        uint8_t data[] = {static_cast<uint8_t>(i)};
        bool key = (i == 1 || i == 4);
        rb.Write(data, 1, 0, 0, 0, key, VideoCodecType::kH264,
                 nullptr, 0, nullptr, 0, nullptr, 0);
    }

    ASSERT_EQ(rb.FindKeyframeBefore(7), 4u);
    ASSERT_EQ(rb.FindKeyframeBefore(4), 4u);
    ASSERT_EQ(rb.FindKeyframeBefore(3), 1u);
    ASSERT_EQ(rb.FindKeyframeBefore(1), 1u);
}

TEST(ring_buffer_find_keyframe_after) {
    VideoRingBuffer rb(16);

    // Write frames: 1=P, 2=P, 3=key, 4=P, 5=key
    for (int i = 1; i <= 5; i++) {
        uint8_t data[] = {static_cast<uint8_t>(i)};
        bool key = (i == 3 || i == 5);
        rb.Write(data, 1, 0, 0, 0, key, VideoCodecType::kH264,
                 nullptr, 0, nullptr, 0, nullptr, 0);
    }

    ASSERT_EQ(rb.FindKeyframeAfter(1), 3u);
    ASSERT_EQ(rb.FindKeyframeAfter(3), 3u);
    ASSERT_EQ(rb.FindKeyframeAfter(4), 5u);
    ASSERT_EQ(rb.FindKeyframeAfter(5), 5u);
}

// ============================================================================
// VideoFrameConsumer Tests
// ============================================================================

TEST(consumer_basic_consumption) {
    VideoRingBuffer rb(16);
    std::mutex mtx;
    std::condition_variable cv;

    // Write 3 frames (first is keyframe so consumer starts there)
    for (int i = 1; i <= 3; i++) {
        uint8_t data[] = {static_cast<uint8_t>(i)};
        rb.Write(data, 1, 0, 0, 0, (i == 1), VideoCodecType::kH264,
                 nullptr, 0, nullptr, 0, nullptr, 0);
    }

    VideoFrameConsumer consumer(&rb, &cv, &mtx, 1, 0, "test");

    VideoFrame frame;
    // Should get frame 1 (keyframe)
    ASSERT_TRUE(consumer.GetNextFrame(frame));
    ASSERT_EQ(frame.data[0], 1);

    // Frame 2
    ASSERT_TRUE(consumer.GetNextFrame(frame));
    ASSERT_EQ(frame.data[0], 2);

    // Frame 3
    ASSERT_TRUE(consumer.GetNextFrame(frame));
    ASSERT_EQ(frame.data[0], 3);

    // No more frames
    ASSERT_FALSE(consumer.GetNextFrame(frame));
}

TEST(consumer_waits_for_frame) {
    VideoRingBuffer rb(16);
    std::mutex mtx;
    std::condition_variable cv;

    // Start consumer on empty buffer
    VideoFrameConsumer consumer(&rb, &cv, &mtx, 1, 0, "test-wait");

    // Launch producer that writes after a delay
    std::thread producer([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        uint8_t data[] = {0x42};
        rb.Write(data, 1, 0, 0, 0, true, VideoCodecType::kH264,
                 nullptr, 0, nullptr, 0, nullptr, 0);
        cv.notify_all();
    });

    VideoFrame frame;
    bool got_frame = consumer.WaitForFrame(frame, 500);
    producer.join();

    ASSERT_TRUE(got_frame);
    ASSERT_EQ(frame.data[0], 0x42);
}

TEST(consumer_timeout) {
    VideoRingBuffer rb(16);
    std::mutex mtx;
    std::condition_variable cv;

    VideoFrameConsumer consumer(&rb, &cv, &mtx, 1, 0, "test-timeout");

    VideoFrame frame;
    auto start = std::chrono::steady_clock::now();
    bool got_frame = consumer.WaitForFrame(frame, 50);
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    ASSERT_FALSE(got_frame);
    ASSERT_TRUE(elapsed >= 40);  // Allow some tolerance
}

TEST(consumer_catches_up_on_overrun) {
    // Small buffer: 4 slots
    VideoRingBuffer rb(4);
    std::mutex mtx;
    std::condition_variable cv;

    // Write one keyframe first so consumer has a starting point
    uint8_t data0[] = {0x00};
    rb.Write(data0, 1, 0, 0, 0, true, VideoCodecType::kH264,
             nullptr, 0, nullptr, 0, nullptr, 0);

    VideoFrameConsumer consumer(&rb, &cv, &mtx, 1, 0, "test-overrun");

    // Now overwrite the buffer completely (write 8 more frames)
    for (int i = 1; i <= 8; i++) {
        uint8_t data[] = {static_cast<uint8_t>(i)};
        // Make frame 5 a keyframe (the one the consumer should catch up to)
        bool key = (i == 5);
        rb.Write(data, 1, 0, 0, 0, key, VideoCodecType::kH264,
                 nullptr, 0, nullptr, 0, nullptr, 0);
    }

    // Consumer should skip to a valid position
    VideoFrame frame;
    bool got_frame = consumer.GetNextFrame(frame);
    ASSERT_TRUE(got_frame);

    // Should have dropped frames
    ASSERT_TRUE(consumer.HasDroppedFrames());
}

TEST(consumer_reset_to_latest) {
    VideoRingBuffer rb(16);
    std::mutex mtx;
    std::condition_variable cv;

    // Write 10 frames
    for (int i = 1; i <= 10; i++) {
        uint8_t data[] = {static_cast<uint8_t>(i)};
        rb.Write(data, 1, 0, 0, 0, (i == 1), VideoCodecType::kH264,
                 nullptr, 0, nullptr, 0, nullptr, 0);
    }

    VideoFrameConsumer consumer(&rb, &cv, &mtx, 1, 0, "test-reset");

    // Read one frame
    VideoFrame frame;
    consumer.GetNextFrame(frame);

    // Reset to latest
    consumer.ResetToLatest();

    // Next GetNextFrame should return false (we're past the latest)
    ASSERT_FALSE(consumer.GetNextFrame(frame));

    // Write a new frame
    uint8_t data[] = {0xFF};
    rb.Write(data, 1, 0, 0, 0, false, VideoCodecType::kH264,
             nullptr, 0, nullptr, 0, nullptr, 0);

    // Now we should get it
    ASSERT_TRUE(consumer.GetNextFrame(frame));
    ASSERT_EQ(frame.data[0], 0xFF);
}

TEST(consumer_reset_to_keyframe) {
    VideoRingBuffer rb(16);
    std::mutex mtx;
    std::condition_variable cv;

    // Write frames: 1=key, 2=P, 3=P, 4=P, 5=key, 6=P
    for (int i = 1; i <= 6; i++) {
        uint8_t data[] = {static_cast<uint8_t>(i)};
        bool key = (i == 1 || i == 5);
        rb.Write(data, 1, 0, 0, 0, key, VideoCodecType::kH264,
                 nullptr, 0, nullptr, 0, nullptr, 0);
    }

    VideoFrameConsumer consumer(&rb, &cv, &mtx, 1, 0, "test-keyframe");

    // Consumer starts at the most recent keyframe (5)
    VideoFrame frame;
    ASSERT_TRUE(consumer.GetNextFrame(frame));
    ASSERT_EQ(frame.data[0], 5);  // Most recent keyframe

    ASSERT_TRUE(consumer.GetNextFrame(frame));
    ASSERT_EQ(frame.data[0], 6);  // Next P-frame
}

TEST(consumer_pending_count) {
    VideoRingBuffer rb(16);
    std::mutex mtx;
    std::condition_variable cv;

    for (int i = 1; i <= 5; i++) {
        uint8_t data[] = {static_cast<uint8_t>(i)};
        rb.Write(data, 1, 0, 0, 0, (i == 1), VideoCodecType::kH264,
                 nullptr, 0, nullptr, 0, nullptr, 0);
    }

    VideoFrameConsumer consumer(&rb, &cv, &mtx, 1, 0, "test-pending");

    // Consumer starts at keyframe (seq 1), pending includes 1-5
    size_t pending = consumer.GetPendingFrameCount();
    ASSERT_TRUE(pending >= 4);  // At least 4 frames pending (may be 5)

    // Consume one
    VideoFrame frame;
    consumer.GetNextFrame(frame);
    ASSERT_TRUE(consumer.GetPendingFrameCount() < pending);
}

// ============================================================================
// Concurrent Access Tests
// ============================================================================

TEST(concurrent_producer_consumer) {
    VideoRingBuffer rb(64);
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> done{false};
    constexpr int kTotalFrames = 500;

    // Producer thread
    std::thread producer([&]() {
        for (int i = 1; i <= kTotalFrames; i++) {
            uint8_t data[64];
            std::memset(data, static_cast<uint8_t>(i & 0xFF), sizeof(data));
            rb.Write(data, sizeof(data), i * 1000, i,
                     0, (i % 30 == 1), VideoCodecType::kH264,
                     nullptr, 0, nullptr, 0, nullptr, 0);
            cv.notify_all();
            // Simulate ~30fps
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        done.store(true);
        cv.notify_all();
    });

    // Consumer thread - create after first frame so ResetToKeyframe works
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    VideoFrameConsumer consumer(&rb, &cv, &mtx, 1, 0, "test-concurrent");

    int consumed = 0;
    VideoFrame frame;
    while (!done.load() || consumer.GetPendingFrameCount() > 0) {
        if (consumer.WaitForFrame(frame, 50)) {
            consumed++;
            // Verify frame integrity (should not be torn)
            ASSERT_TRUE(frame.data.size() == 64);
        }
    }

    producer.join();

    // We should have consumed a reasonable number of frames
    ASSERT_TRUE(consumed > 0);
    std::cout << " (" << consumed << "/" << kTotalFrames << " frames consumed)";
}

TEST(concurrent_multiple_consumers) {
    VideoRingBuffer rb(64);
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> done{false};
    constexpr int kTotalFrames = 200;

    // Producer
    std::thread producer([&]() {
        for (int i = 1; i <= kTotalFrames; i++) {
            uint8_t data[] = {static_cast<uint8_t>(i & 0xFF)};
            rb.Write(data, 1, 0, 0, 0, (i % 30 == 1), VideoCodecType::kH264,
                     nullptr, 0, nullptr, 0, nullptr, 0);
            cv.notify_all();
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
        done.store(true);
        cv.notify_all();
    });

    // Wait for producer to start
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    // Multiple consumers
    constexpr int kNumConsumers = 4;
    std::atomic<int> consumer_counts[kNumConsumers];
    for (auto& c : consumer_counts) c.store(0);

    std::vector<std::thread> consumer_threads;
    for (int c = 0; c < kNumConsumers; c++) {
        consumer_threads.emplace_back([&, c]() {
            VideoFrameConsumer consumer(&rb, &cv, &mtx, c + 10, 0,
                                        "consumer-" + std::to_string(c));
            VideoFrame frame;
            while (!done.load() || consumer.GetPendingFrameCount() > 0) {
                if (consumer.WaitForFrame(frame, 50)) {
                    consumer_counts[c].fetch_add(1);
                }
            }
        });
    }

    producer.join();
    // Give consumers time to drain
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    done.store(true);
    cv.notify_all();

    for (auto& t : consumer_threads) t.join();

    // Each consumer should have consumed frames independently
    for (int c = 0; c < kNumConsumers; c++) {
        ASSERT_TRUE(consumer_counts[c].load() > 0);
    }

    std::cout << " (consumers read: ";
    for (int c = 0; c < kNumConsumers; c++) {
        if (c > 0) std::cout << "/";
        std::cout << consumer_counts[c].load();
    }
    std::cout << ")";
}

TEST(ring_buffer_h265_vps) {
    VideoRingBuffer rb(8);

    uint8_t vps[] = {0x40, 0x01, 0x0C, 0x01};
    uint8_t sps[] = {0x42, 0x01, 0x01};
    uint8_t pps[] = {0x44, 0x01, 0xC0};
    uint8_t data[] = {0x00, 0x00, 0x00, 0x01, 0x26};  // H.265 IDR

    rb.Write(data, sizeof(data), 0, 0, 0, true, VideoCodecType::kH265,
             sps, sizeof(sps), pps, sizeof(pps), vps, sizeof(vps));

    VideoFrame frame;
    ASSERT_TRUE(rb.Read(1, frame));
    ASSERT_EQ(frame.codec, VideoCodecType::kH265);
    ASSERT_EQ(frame.vps_size, sizeof(vps));
    ASSERT_TRUE(std::memcmp(frame.vps, vps, sizeof(vps)) == 0);
    ASSERT_EQ(frame.sps_size, sizeof(sps));
    ASSERT_EQ(frame.pps_size, sizeof(pps));
}

TEST(ring_buffer_large_frame) {
    VideoRingBuffer rb(4);

    // Write a frame near the max size
    std::vector<uint8_t> large_data(512 * 1024);  // 512KB
    std::iota(large_data.begin(), large_data.end(), 0);

    rb.Write(large_data.data(), static_cast<uint32_t>(large_data.size()),
             0, 0, 0, true, VideoCodecType::kH264,
             nullptr, 0, nullptr, 0, nullptr, 0);

    VideoFrame frame;
    ASSERT_TRUE(rb.Read(1, frame));
    ASSERT_EQ(frame.data.size(), large_data.size());
    ASSERT_TRUE(std::memcmp(frame.data.data(), large_data.data(), large_data.size()) == 0);
}

TEST(frame_clear) {
    VideoFrame frame;
    frame.data = {1, 2, 3};
    frame.is_keyframe = true;
    frame.sps_size = 4;
    frame.nal_packs.push_back({0, 3, false});

    frame.Clear();
    ASSERT_TRUE(frame.data.empty());
    ASSERT_FALSE(frame.is_keyframe);
    ASSERT_EQ(frame.sps_size, 0u);
    ASSERT_TRUE(frame.nal_packs.empty());
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::cout << "=== MediaHub Unit Tests ===" << std::endl;
    std::cout << std::endl;

    // Tests are auto-registered and run by static constructors above

    std::cout << std::endl;
    std::cout << "Results: " << tests_passed << " passed, "
              << tests_failed << " failed" << std::endl;

    return tests_failed > 0 ? 1 : 0;
}
