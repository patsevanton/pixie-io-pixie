#pragma once

#include <nghttp2/nghttp2_frame.h>

#include <map>
#include <string>

#include "src/stirling/common/parse_state.h"
#include "src/stirling/common/utils.h"

namespace pl {
namespace stirling {
namespace http2 {

using u8string = std::basic_string<uint8_t>;

// Note that NVMap keys (HTTP2 header field names) are assumed to be lowercase to match spec:
//
// From https://http2.github.io/http2-spec/#HttpHeaders:
// ... header field names MUST be converted to lowercase prior to their encoding in HTTP/2.
// A request or response containing uppercase header field names MUST be treated as malformed.
class NVMap : public std::multimap<std::string, std::string> {
 public:
  using std::multimap<std::string, std::string>::multimap;

  std::string ValueByKey(const std::string& key, const std::string& default_value = "") {
    auto iter = find(key);
    if (iter != end()) {
      return iter->second;
    }
    return default_value;
  }
};

/**
 * @brief A wrapper around  nghttp2_frame. nghttp2_frame misses some fields, for example, it has no
 * data body field in nghttp2_data. The payload is a name meant to be generic enough so that it can
 * be used to store such fields for different message types.
 */
struct Frame {
  // frame{} zero initialize the member, which is needed to make sure default value is sensible.
  Frame() : frame{} {};
  ~Frame() {
    if (frame.hd.type == NGHTTP2_HEADERS) {
      // We do not use NGHTT2's storage constructs for headers.
      // This check forbids this.
      DCHECK(frame.headers.nva == nullptr);
      DCHECK_EQ(frame.headers.nvlen, 0u);
    }
  }

  // TODO(yzhao): Remove this, as it's value is included in time_span already.
  uint64_t timestamp_ns;
  // The time stamp when this frame was created by socket tracer.
  // TODO(yzhao): Consider removing this, as it's value can be replaced by time_span, although not
  // exactly the same.
  std::chrono::time_point<std::chrono::steady_clock> creation_timestamp;

  // TODO(yzhao): Consider use std::unique_ptr<nghttp2_frame> to avoid copy.
  nghttp2_frame frame;
  u8string u8payload;

  // If true, means this frame is processed and can be destroyed.
  mutable bool consumed = false;

  // Only meaningful for HEADERS frame, indicates if a frame syncing error is detected.
  ParseState frame_sync_state = ParseState::kUnknown;
  // Only meaningful for HEADERS frame, indicates if a header block is already processed.
  ParseState headers_parse_state = ParseState::kUnknown;
  NVMap headers;

  size_t ByteSize() const { return sizeof(Frame) + u8payload.size() + CountStringMapSize(headers); }
};

}  // namespace http2
}  // namespace stirling
}  // namespace pl
