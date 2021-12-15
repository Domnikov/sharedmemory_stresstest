/*
 * Copyright (C) 2021 - Volvo Car Corporation
 *
 * All Rights Reserved
 *
 * LEGAL NOTICE:  All information (including intellectual and technical
 * concepts) contained herein is, and remains the property of or licensed to
 * Volvo Car Corporation. This information is proprietary to Volvo Car
 * Corporation and may be covered by patents or patent applications. This
 * information is protected by trade secret or copyright law. Dissemination of
 * this information or reproduction of this material is strictly forbidden
 * unless prior written permission is obtained from Volvo Car Corporation.
 */

#include "sfw/shared_memory/shared_memory.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <climits>
#include <cstring>

using ::csp::sfw_shared_memory::SharedMemory;
using ::testing::StartsWith;

const std::string kSharedMemoryPrefix = "/testSharedMem";
constexpr size_t kMemorySize = 1;

TEST(SharedMemTest, DefaultConstructsAnInvalidSharedMemory) {
    SharedMemory shared_memory{};

    EXPECT_FALSE(shared_memory.is_valid());
    EXPECT_EQ(nullptr, shared_memory.data());
    EXPECT_EQ(0, shared_memory.size());
}

TEST(SharedMemTest, CantCreateSharedMemWithInvalidName) {
    auto shared_memory = SharedMemory::Create("testSharedMem", kMemorySize);

    EXPECT_FALSE(shared_memory.is_valid());
    EXPECT_EQ(nullptr, shared_memory.data());
    EXPECT_EQ(0, shared_memory.size());
}

TEST(SharedMemTest, CantCreateSharedMemWithTooLongName) {
    // Create a string (long_name) which is of length NAME_MAX. NAME_MAX is the max length of the zero-terminated
    // string  that is allowed by shm_open (called from SharedMemory::Create). In Create we will call
    // long_string.c_str() which will append zero terminator to the string which will result in a
    // length of NAME_MAX.
    std::string long_name = "/";
    long_name.insert(long_name.end(), NAME_MAX - 1, 'a');
    auto shared_memory = SharedMemory::Create(long_name, kMemorySize);

    EXPECT_FALSE(shared_memory.is_valid());
    EXPECT_EQ(nullptr, shared_memory.data());
    EXPECT_EQ(0, shared_memory.size());
}

TEST(SharedMemTest, CanCreateSharedMemWithVeryLongName) {
    // Create a string (long_name) which is of length NAME_MAX - 1. NAME_MAX is the max length of the string
    // that is allowed by shm_open (called from SharedMemory::Create). In Create we will call
    // long_string.c_str() which will append zero terminator to the string which will result in a
    // length of NAME_MAX.
    std::string long_name = "/";
    long_name.insert(long_name.end(), NAME_MAX - 2, 'a');
    auto shared_memory = SharedMemory::Create(long_name, kMemorySize);

    EXPECT_TRUE(shared_memory.is_valid());
    EXPECT_NE(nullptr, shared_memory.data());
    EXPECT_EQ(kMemorySize, shared_memory.size());
}

TEST(SharedMemTest, CantCreateSharedMemWithEmptyName) {
    auto shared_memory = SharedMemory::Create("", kMemorySize);

    EXPECT_FALSE(shared_memory.is_valid());
    EXPECT_EQ(nullptr, shared_memory.data());
    EXPECT_EQ(0, shared_memory.size());
}

TEST(SharedMemTest, CantCreateSharedMemWithZeroSize) {
    auto shared_memory = SharedMemory::Create(kSharedMemoryPrefix, 0);

    EXPECT_FALSE(shared_memory.is_valid());
    EXPECT_EQ(nullptr, shared_memory.data());
    EXPECT_EQ(0, shared_memory.size());
}

TEST(SharedMemTest, CanCreateValidSharedMemObject) {
    auto shared_memory = SharedMemory::CreateUnique(kSharedMemoryPrefix, kMemorySize);

    EXPECT_TRUE(shared_memory.is_valid());
    EXPECT_THAT(shared_memory.name(), StartsWith(kSharedMemoryPrefix));
    EXPECT_NE(nullptr, shared_memory.data());
    EXPECT_EQ(kMemorySize, shared_memory.size());
}

TEST(SharedMemTest, ShouldMoveConstructSharedMemory) {
    auto shared_memory = SharedMemory::CreateUnique(kSharedMemoryPrefix, kMemorySize);
    EXPECT_TRUE(shared_memory.is_valid());

    SharedMemory shared_memory2{std::move(shared_memory)};
    EXPECT_TRUE(shared_memory2.is_valid());
    EXPECT_FALSE(shared_memory.is_valid());
}

TEST(SharedMemTest, ShouldMoveAssignSharedMemory) {
    auto shared_memory = SharedMemory::CreateUnique(kSharedMemoryPrefix, kMemorySize);
    EXPECT_TRUE(shared_memory.is_valid());

    SharedMemory shared_memory2{};
    shared_memory2 = std::move(shared_memory);
    EXPECT_TRUE(shared_memory2.is_valid());
    EXPECT_FALSE(shared_memory.is_valid());
}

TEST(SharedMemTest, CanWriteToMem) {
    auto shared_memory = SharedMemory::CreateUnique(kSharedMemoryPrefix, kMemorySize);
    ASSERT_TRUE(shared_memory.is_valid());

    *static_cast<uint8_t*>(shared_memory.data()) = 42;
    EXPECT_EQ(42, *static_cast<uint8_t*>(shared_memory.data()));
}

TEST(SharedMemTest, ConsumerCanOpenAfterCreated) {
    constexpr size_t kSize = 1024;
    auto producer_shared_memory = SharedMemory::CreateUnique(kSharedMemoryPrefix, kSize);
    EXPECT_TRUE(producer_shared_memory.is_valid());

    auto consumer_shm = SharedMemory::Open(producer_shared_memory.name(), SharedMemory::AccessType::kReadOnly);
    EXPECT_TRUE(consumer_shm.is_valid());
    EXPECT_EQ(kSize, producer_shared_memory.size());
    EXPECT_EQ(kSize, consumer_shm.size());
}

TEST(SharedMemTest, ConsumerCanNotOpenBeforeCreated) {
    auto consumer_shm = SharedMemory::Open(kSharedMemoryPrefix, SharedMemory::AccessType::kReadOnly);
    EXPECT_FALSE(consumer_shm.is_valid());
}

TEST(SharedMemTest, ConsumerMemoryMatchesProducerMemoryAfterWrite) {
    constexpr size_t kSize = 1024;
    auto producer_shared_memory = SharedMemory::CreateUnique(kSharedMemoryPrefix, kSize);
    EXPECT_TRUE(producer_shared_memory.is_valid());

    auto consumer_shm = SharedMemory::Open(producer_shared_memory.name(), SharedMemory::AccessType::kReadOnly);
    EXPECT_TRUE(consumer_shm.is_valid());
    EXPECT_EQ(kSize, producer_shared_memory.size());
    EXPECT_EQ(kSize, consumer_shm.size());

    EXPECT_EQ(0, std::memcmp(producer_shared_memory.data(), consumer_shm.data(), kSize));

    auto producer_data = producer_shared_memory.data();
    uint64_t value = 42;
    std::memcpy(producer_data, &value, sizeof(uint64_t));

    uint64_t* consumer_data = (uint64_t*)consumer_shm.data();
    uint64_t read_val = *consumer_data;

    EXPECT_EQ(value, read_val);

    EXPECT_EQ(0, std::memcmp(producer_data, consumer_shm.data(), kSize));
}
