```cpp
enum class XErrc : std::uint8_t {
  out_of_range,
  io_error
};

// generated from quick_status_code.py
SYSTEM_ERROR2_NAMESPACE_BEGIN
template <>
struct quick_status_code_from_enum<XErrc>
    : quick_status_code_from_enum_defaults<XErrc> {
  static constexpr auto domain_name = "XErrc";
  // 0x807d0522e7b4e8eb
  static constexpr auto domain_uuid = "35f55a10-24ca-4c8c-8b7b-116e069a9b84";
  // 0x5b88ce2002a67819
  static constexpr auto payload_uuid = "3a62e7c9-7cef-4928-abe5-8de97e03c19d";
  static const std::initializer_list<mapping>& value_mappings() {
    static const std::initializer_list<mapping> v = {
      {XErrc::out_of_range, "out_of_range", {}}, // NOLINT
      {XErrc::io_error, "io_error", {}}, // NOLINT
    };
    return v;
  }
};
SYSTEM_ERROR2_NAMESPACE_END

namespace oc = OUTCOME_V2_NAMESPACE::experimental;

// default payload type is std::string
using XError = oc::EnumPayloadError<XErrc>;

template<typename T>
using Result = oc::status_result<T>;

// return error without payload
Result<std::string> read_file() {
  return XErrc::io_error;
}

// return error with custome payload
Result<std::string> read_file2() {
  return XError({XErrc::io_error, "/path/to/file"});
}

void test() {
  auto ret = read_file();
  assert(ret.has_failure());
  assert(ret.error() == XErrc::io_error);
  // XErrc.io_error
  fmt::println("{}", ret.error().message());

  auto ret2 = read_file2();
  assert(ret2.has_failure());
  assert(ret2.error() == XErrc::io_error);
  assert(ret2.error() == ret.error());
  // XErrc.io_error: /path/to/file
  fmt::println("{}", ret2.error().message());
}
```
