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
  static constexpr auto domain_uuid = "{6009c534-9d4c-4037-8719-ce562551cb02}";
  static constexpr auto payload_domain_uuid = "{055ec200-1386-441d-a4b2-60e1f71c1d53}";
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
