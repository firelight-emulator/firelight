#include "../../../src/app/patching/util.hpp"

#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

using namespace firelight::patching;

/**
 * @brief Test fixture for patching utility functions
 * 
 * Tests utility functions used in patch format implementations, particularly
 * the Variable Length Value (VLV) encoding/decoding functionality used in
 * various patch formats for compact integer representation.
 */
class UtilTest : public testing::Test {};

/**
 * @brief Test reading single-byte VLV values
 * 
 * Verifies that Variable Length Values encoded in a single byte (with the
 * high bit set to indicate termination) are correctly decoded.
 */
TEST_F(UtilTest, readVLV_SingleByte) {
    // Test single byte values with high bit set (terminates immediately)
    std::vector<uint8_t> data = {0xC2}; // 0x42 | 0x80 = 0xC2, should give 66
    size_t index = 0;
    
    int64_t result = readVLV(data, index);
    
    EXPECT_EQ(result, 66);
    EXPECT_EQ(index, 1);
}

/**
 * @brief Test reading multi-byte VLV values
 * 
 * Verifies that Variable Length Values spanning multiple bytes are correctly
 * decoded, with continuation bytes (high bit clear) and termination byte
 * (high bit set) handled properly.
 */
TEST_F(UtilTest, readVLV_MultiByte) {
    // Test multi-byte value: 0x00 (continue), 0x81 (stop, value=1)
    // First: result += 0*1 = 0, shift = 128, result += 128 = 128
    // Second: result += 1*128 = 128, total = 256
    std::vector<uint8_t> data = {0x00, 0x81};
    size_t index = 0;
    
    int64_t result = readVLV(data, index);
    
    EXPECT_EQ(result, 256);
    EXPECT_EQ(index, 2);
}

/**
 * @brief Test reading larger multi-byte VLV values
 * 
 * Verifies that Variable Length Values spanning three bytes are correctly
 * decoded with proper accumulation and bit shifting operations.
 */
TEST_F(UtilTest, readVLV_LargerMultiByte) {
    // Test: 0x00 (continue), 0x00 (continue), 0x81 (stop, value=1)
    // First byte: 0*1 = 0, shift becomes 128, result += 128 = 128
    // Second byte: 0*128 = 0, shift becomes 16384, result += 16384 = 16512  
    // Third byte: 1*16384 = 16384, total = 128 + 16384 + 16384 = 32896
    std::vector<uint8_t> data = {0x00, 0x00, 0x81};
    size_t index = 0;
    
    int64_t result = readVLV(data, index);
    
    EXPECT_EQ(result, 32896);
    EXPECT_EQ(index, 3);
}

/**
 * @brief Test reading VLV-encoded zero value
 * 
 * Verifies that a zero value encoded in VLV format (single byte with
 * high bit set) is correctly decoded to zero.
 */
TEST_F(UtilTest, readVLV_Zero) {
    // Test zero value with high bit set to terminate
    std::vector<uint8_t> data = {0x80}; // 0x00 | 0x80 = 0x80
    size_t index = 0;
    
    int64_t result = readVLV(data, index);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(index, 1);
}

/**
 * @brief Test reading maximum single-byte VLV value
 * 
 * Verifies that the maximum value that can be encoded in a single VLV byte
 * (127, represented as 0xFF) is correctly decoded.
 */
TEST_F(UtilTest, readVLV_MaxSingleByte) {
    // Test maximum single byte value (0xFF = 127 + high bit)
    std::vector<uint8_t> data = {0xFF}; // 0x7F | 0x80 = 0xFF
    size_t index = 0;
    
    int64_t result = readVLV(data, index);
    
    EXPECT_EQ(result, 127);
    EXPECT_EQ(index, 1);
}

/**
 * @brief Test VLV reading with empty data throws exception
 * 
 * Verifies that attempting to read a VLV from empty data throws a
 * std::runtime_error exception for proper error handling.
 */
TEST_F(UtilTest, readVLV_EmptyData) {
    // Test empty data - should throw exception
    std::vector<uint8_t> data = {};
    size_t index = 0;
    
    EXPECT_THROW(readVLV(data, index), std::runtime_error);
}

/**
 * @brief Test VLV reading with out-of-bounds index throws exception
 * 
 * Verifies that attempting to read a VLV starting beyond the data size
 * throws a std::runtime_error exception.
 */
TEST_F(UtilTest, readVLV_IndexOutOfBounds) {
    // Test index beyond data size
    std::vector<uint8_t> data = {0xC2};
    size_t index = 1;
    
    EXPECT_THROW(readVLV(data, index), std::runtime_error);
}

/**
 * @brief Test VLV reading with incomplete multi-byte sequence throws exception
 * 
 * Verifies that an incomplete multi-byte VLV sequence (missing termination
 * byte with high bit set) throws a std::runtime_error exception.
 */
TEST_F(UtilTest, readVLV_IncompleteMultiByte) {
    // Test incomplete multi-byte sequence - should throw exception
    std::vector<uint8_t> data = {0x00}; // No terminating byte with high bit
    size_t index = 0;
    
    EXPECT_THROW(readVLV(data, index), std::runtime_error);
}

/**
 * @brief Test reading multiple consecutive VLV values
 * 
 * Verifies that multiple VLV values can be read sequentially from the same
 * data buffer, with the index properly advancing after each read operation.
 */
TEST_F(UtilTest, readVLV_MultipleValues) {
    // Test reading multiple values from same data
    std::vector<uint8_t> data = {0xC2, 0xFF, 0x00, 0x81}; // 66, 127, then 256
    size_t index = 0;
    
    int64_t first = readVLV(data, index);
    EXPECT_EQ(first, 66);
    EXPECT_EQ(index, 1);
    
    int64_t second = readVLV(data, index);
    EXPECT_EQ(second, 127);
    EXPECT_EQ(index, 2);
    
    int64_t third = readVLV(data, index);
    EXPECT_EQ(third, 256);
    EXPECT_EQ(index, 4);
}