#include <firelight/netplay/base64.hpp>
#include <firelight/netplay/signal_chunker.hpp>

#include <gtest/gtest.h>

namespace firelight::netplay {

TEST(Base64Test, RoundTripsIncludingPadding) {
  const std::vector<std::string> samples{
      "", "a", "ab", "abc", "abcd", "v=0\r\no=- 4611731 2 IN IP4 127.0.0.1", std::string(5000, 'x')};
  for (const auto &text : samples) {
    const auto decoded = base64DecodeText(base64EncodeText(text));
    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(*decoded, text);
  }
  EXPECT_FALSE(base64DecodeText("not!valid?").has_value());
}

TEST(SignalChunkerTest, SmallPayloadIsOneChunk) {
  const auto chunks = chunkSignal("hello", 1000, "sig1");
  ASSERT_EQ(chunks.size(), 1u);
  EXPECT_EQ(chunks[0].index, 0);
  EXPECT_EQ(chunks[0].total, 1);

  SignalReassembler reassembler;
  const auto payload = reassembler.accept(1, chunks[0]);
  ASSERT_TRUE(payload.has_value());
  EXPECT_EQ(*payload, "hello");
}

TEST(SignalChunkerTest, LargePayloadSplitsAndReassemblesOutOfOrder) {
  const std::string payload(4000, 's'); // SDP-sized
  const auto chunks = chunkSignal(payload, 500, "sig2");
  ASSERT_GT(chunks.size(), 2u);

  SignalReassembler reassembler;
  // Deliver in reverse order; only the last-arriving chunk completes it
  for (size_t i = chunks.size(); i-- > 1;) {
    EXPECT_FALSE(reassembler.accept(7, chunks[i]).has_value());
  }
  const auto result = reassembler.accept(7, chunks[0]);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, payload);
}

TEST(SignalChunkerTest, SendersAndSignalsDoNotMix) {
  const auto first = chunkSignal("payload-one", 4, "sigA");
  const auto second = chunkSignal("payload-two", 4, "sigB");

  SignalReassembler reassembler;
  // Same signal id from two senders, plus two signals from one sender
  for (size_t i = 0; i + 1 < first.size(); ++i) {
    EXPECT_FALSE(reassembler.accept(1, first[i]).has_value());
    EXPECT_FALSE(reassembler.accept(2, first[i]).has_value());
    EXPECT_FALSE(reassembler.accept(1, second[i]).has_value());
  }
  EXPECT_EQ(reassembler.accept(1, first.back()).value(), "payload-one");
  EXPECT_EQ(reassembler.accept(2, first.back()).value(), "payload-one");
  EXPECT_EQ(reassembler.accept(1, second.back()).value(), "payload-two");
}

TEST(SignalChunkerTest, RejectsNonsenseChunks) {
  SignalReassembler reassembler;
  EXPECT_FALSE(
      reassembler.accept(1, SignalChunk{.signalId = "x", .index = 5, .total = 2, .content = "zz"}).has_value());
  EXPECT_FALSE(
      reassembler.accept(1, SignalChunk{.signalId = "x", .index = 0, .total = 0, .content = "zz"}).has_value());
}

TEST(SignalChunkerTest, SignalIdShape) {
  const auto id = generateSignalId();
  EXPECT_EQ(id.size(), 8u);
  EXPECT_NE(id, generateSignalId());
}

} // namespace firelight::netplay
