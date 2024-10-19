#pragma once

#include <fmt/format.h>
#include <outcome-experimental.hpp>

#include <string>
#include <string_view>

template <>
struct fmt::formatter<
    OUTCOME_V2_NAMESPACE::experimental::status_code_domain::string_ref>
    : fmt::formatter<std::string_view> {
  using Self =
      OUTCOME_V2_NAMESPACE::experimental::status_code_domain::string_ref;

  template <typename FormatContext>
  auto format(const Self &ref, FormatContext &ctx) const {
    auto s = std::string_view{ref.data(), ref.size()};
    return fmt::formatter<std::string_view>::format(s, ctx);
  }
};

SYSTEM_ERROR2_NAMESPACE_BEGIN

template <typename Enum, typename = void>
struct HasQuickFromEnum : std::false_type {};

template <typename Enum>
struct HasQuickFromEnum<Enum, std::void_t<quick_status_code_from_enum<Enum>>>
    : std::true_type {};

template <typename Enum, typename Payload>
concept EnumPayload = HasQuickFromEnum<Enum>::value &&
                      requires {
                        { quick_status_code_from_enum<Enum>::payload_uuid };
                      } && fmt::is_formattable<Payload>::value &&
                      // TODO: trivially relocatable
                      std::is_nothrow_move_constructible_v<Payload>;

template <typename Enum, typename Payload> struct EnumPayloadDomainImpl;

template <typename Enum, typename Payload = std::string>
  requires EnumPayload<Enum, Payload>
using EnumPayloadError = status_code<EnumPayloadDomainImpl<Enum, Payload>>;

template <typename Enum, typename Payload>
  requires EnumPayload<Enum, Payload>
struct EnumPayloadDomainImpl<Enum, Payload> final : public status_code_domain {
  using Base = status_code_domain;
  using Src = quick_status_code_from_enum<Enum>;
  using EnumPayloadErrorSelf = EnumPayloadError<Enum, Payload>;

  constexpr EnumPayloadDomainImpl()
      : Base(Src::payload_uuid,
             _uuid_size<detail::cstrlen(Src::payload_uuid)>()) {}
  EnumPayloadDomainImpl(const EnumPayloadDomainImpl &) = default;
  EnumPayloadDomainImpl(EnumPayloadDomainImpl &&) = default;
  EnumPayloadDomainImpl &operator=(const EnumPayloadDomainImpl &) = default;
  EnumPayloadDomainImpl &operator=(EnumPayloadDomainImpl &&) = default;
  ~EnumPayloadDomainImpl() = default;

  struct value_type {
    Enum value;
    Payload payload;
  };

  string_ref name() const noexcept final {
    return string_ref{Src::domain_name};
  }

  payload_info_t payload_info() const noexcept final {
    return {sizeof(value_type),
            sizeof(status_code_domain *) + sizeof(value_type),
            (alignof(value_type) > alignof(status_code_domain *))
                ? alignof(value_type)
                : alignof(status_code_domain *)};
  }

  static constexpr const EnumPayloadDomainImpl &get();

  static constexpr const Src::mapping *_find_mapping(Enum e) noexcept {
    for (const auto &i : Src::value_mappings()) {
      if (i.value == e) {
        return &i;
      }
    }
    return nullptr;
  }

  bool _do_failure(const status_code<void> &code) const noexcept final {
    assert(code.domain() == *this);
    const auto &c = static_cast<const EnumPayloadErrorSelf &>(code);
    const auto *mapping = _find_mapping(c.value().value);
    assert(mapping != nullptr);
    if (mapping != nullptr) {
      for (auto ec : mapping->code_mappings) {
        if (ec == errc::success) {
          return false;
        }
      }
    }
    return true;
  }

  bool _do_equivalent(const status_code<void> &code1,
                      const status_code<void> &code2) const noexcept final {
    assert(code1.domain() == *this);

    const auto &c1 = static_cast<const EnumPayloadErrorSelf &>(code1);
    if (code2.domain() == *this) {
      const auto &c2 = static_cast<const EnumPayloadErrorSelf &>(code2);
      return c1.value().value == c2.value().value;
    }

    if (code2.domain() == quick_status_code_from_enum_domain<Enum>) {
      using QuickCode = quick_status_code_from_enum_code<Enum>;
      const auto &c2 = static_cast<const QuickCode &>(code2);
      return c1.value().value == c2.value();
    }

    if (code2.domain() == generic_code_domain) {
      const auto &c2 = static_cast<const generic_code &>(code2); // NOLINT
      const auto *mapping = _find_mapping(c1.value().value);
      assert(mapping != nullptr);
      if (mapping != nullptr) {
        for (auto ec : mapping->code_mappings) {
          if (ec == c2.value()) {
            return true;
          }
        }
      }
    }
    return false;
  }

  generic_code
  _generic_code(const status_code<void> &code) const noexcept final {
    assert(code.domain() == *this);
    const auto *mapping = _find_mapping(
        static_cast<const EnumPayloadErrorSelf &>(code).value().value);
    assert(mapping != nullptr);
    if (mapping != nullptr) {
      if (mapping->code_mappings.size() > 0) {
        return *mapping->code_mappings.begin();
      }
    }
    return errc::unknown;
  }

  string_ref _do_message(const status_code<void> &code) const noexcept final {
    assert(code.domain() == *this);
    const value_type &c =
        static_cast<const EnumPayloadErrorSelf &>(code).value();
    const auto mapping = _find_mapping(c.value);
    auto s = fmt::format("{}.{}: {}", name(), mapping->message, c.payload);
    auto p = (char *)malloc(s.size()); // NOLINT
    memcpy(p, s.data(), s.size());     // NOLINT
    return atomic_refcounted_string_ref(p, s.size());
  }

  void _do_throw_exception(const status_code<void> &code) const final;
};

template <typename Enum, typename Payload>
  requires EnumPayload<Enum, Payload>
constexpr EnumPayloadDomainImpl<Enum, Payload> EnumPayloadDomain = {};

template <typename Enum, typename Payload>
  requires EnumPayload<Enum, Payload>
constexpr const EnumPayloadDomainImpl<Enum, Payload> &
EnumPayloadDomainImpl<Enum, Payload>::get() {
  return EnumPayloadDomain<Enum, Payload>;
}

template <typename Enum, typename Payload>
  requires EnumPayload<Enum, Payload>
inline system_code make_status_code(EnumPayloadError<Enum, Payload> e) {
  return make_nested_status_code(std::move(e));
}

template <typename Enum, typename Payload>
  requires EnumPayload<Enum, Payload>
void EnumPayloadDomainImpl<Enum, Payload>::_do_throw_exception(
    const status_code<void> &code) const {
  assert(code.domain() == *this);
  const auto &c = static_cast<const EnumPayloadErrorSelf &>(code);
  throw status_error<EnumPayloadDomainImpl<Enum, Payload>>(c);
}

SYSTEM_ERROR2_NAMESPACE_END
