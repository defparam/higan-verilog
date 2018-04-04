#pragma once

namespace nall {
  using uint = unsigned;

  enum class Compiler : uint { Clang, GCC, Microsoft, Unknown };
  enum class Platform : uint { Windows, MacOS, Linux, BSD, Unknown };
  enum class API : uint { Windows, Posix, Unknown };
  enum class DisplayServer : uint { Windows, Quartz, Xorg, Unknown };
  enum class Processor : uint { x86, amd64, ARM, PPC32, PPC64, Unknown };
  enum class Endian : uint { LSB, MSB, Unknown };
  enum class Build : uint { Debug, Stable, Size, Release, Performance };

  static inline constexpr auto compiler() -> Compiler;
  static inline constexpr auto platform() -> Platform;
  static inline constexpr auto api() -> API;
  static inline constexpr auto display() -> DisplayServer;
  static inline constexpr auto processor() -> Processor;
  static inline constexpr auto endian() -> Endian;
  static inline constexpr auto build() -> Build;
}

/* Compiler detection */

namespace nall {

#if defined(__clang__)
  #define COMPILER_CLANG
  constexpr auto compiler() -> Compiler { return Compiler::Clang; }

  #pragma clang diagnostic ignored "-Wunknown-pragmas"
  #pragma clang diagnostic ignored "-Wempty-body"
  #pragma clang diagnostic ignored "-Wparentheses"
  #pragma clang diagnostic ignored "-Wreturn-type"
  #pragma clang diagnostic ignored "-Wswitch"
  #pragma clang diagnostic ignored "-Wswitch-bool"
  #pragma clang diagnostic ignored "-Wtautological-compare"
  #pragma clang diagnostic ignored "-Wabsolute-value"

  //temporary
  #pragma clang diagnostic ignored "-Winconsistent-missing-override"
  #pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
  #define COMPILER_GCC
  constexpr auto compiler() -> Compiler { return Compiler::GCC; }

  #pragma GCC diagnostic ignored "-Wunknown-pragmas"
  #pragma GCC diagnostic ignored "-Wpragmas"
  #pragma GCC diagnostic ignored "-Wswitch-bool"
#elif defined(_MSC_VER)
  #define COMPILER_MICROSOFT
  constexpr auto compiler() -> Compiler { return Compiler::Microsoft; }

  #pragma warning(disable:4996)  //libc "deprecation" warnings
#else
  #warning "unable to detect compiler"
  #define COMPILER_UNKNOWN
  constexpr auto compiler() -> Compiler { return Compiler::Unknown; }
#endif

}

/* Platform detection */

namespace nall {

#if defined(_WIN32)
  #define PLATFORM_WINDOWS
  #define API_WINDOWS
  #define DISPLAY_WINDOWS
  constexpr auto platform() -> Platform { return Platform::Windows; }
  constexpr auto api() -> API { return API::Windows; }
  constexpr auto display() -> DisplayServer { return DisplayServer::Windows; }
#elif defined(__APPLE__)
  #define PLATFORM_MACOS
  #define API_POSIX
  #define DISPLAY_QUARTZ
  constexpr auto platform() -> Platform { return Platform::MacOS; }
  constexpr auto api() -> API { return API::Posix; }
  constexpr auto display() -> DisplayServer { return DisplayServer::Quartz; }
#elif defined(linux) || defined(__linux__)
  #define PLATFORM_LINUX
  #define API_POSIX
  #define DISPLAY_XORG
  constexpr auto platform() -> Platform { return Platform::Linux; }
  constexpr auto api() -> API { return API::Posix; }
  constexpr auto display() -> DisplayServer { return DisplayServer::Xorg; }
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__NetBSD__) || defined(__OpenBSD__)
  #define PLATFORM_BSD
  #define API_POSIX
  #define DISPLAY_XORG
  constexpr auto platform() -> Platform { return Platform::BSD; }
  constexpr auto api() -> API { return API::Posix; }
  constexpr auto display() -> DisplayServer { return DisplayServer::Xorg; }
#else
  #warning "unable to detect platform"
  #define PLATFORM_UNKNOWN
  #define API_UNKNOWN
  #define DISPLAY_UNKNOWN
  constexpr auto platform() -> Platform { return Platform::Unknown; }
  constexpr auto api() -> API { return API::Unknown; }
  constexpr auto display() -> DisplayServer { return DisplayServer::Unknown; }
#endif

}

#if defined(PLATFORM_MACOS)
  #include <machine/endian.h>
#elif defined(PLATFORM_LINUX)
  #include <endian.h>
#elif defined(PLATFORM_BSD)
  #include <sys/endian.h>
#endif

/* Processor detection */

namespace nall {

#if defined(__i386__) || defined(_M_IX86)
  #define PROCESSOR_X86
  constexpr auto processor() -> Processor { return Processor::x86; }
#elif defined(__amd64__) || defined(_M_AMD64)
  #define PROCESSOR_AMD64
  constexpr auto processor() -> Processor { return Processor::amd64; }
#elif defined(__arm__)
  #define PROCESSOR_ARM
  constexpr auto processor() -> Processor { return Processor::ARM; }
#elif defined(__ppc64__) || defined(_ARCH_PPC64)
  #define PROCESSOR_PPC64
  constexpr auto processor() -> Processor { return Processor::PPC64; }
#elif defined(__ppc__) || defined(_ARCH_PPC) || defined(_M_PPC)
  #define PROCESSOR_PPC32
  constexpr auto processor() -> Processor { return Processor::PPC32; }
#else
  #warning "unable to detect processor"
  #define PROCESSOR_UNKNOWN
  constexpr auto processor() -> Processor { return Processor::Unknown; }
#endif

}

/* Endian detection */

namespace nall {

#if (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && __BYTE_ORDER == __LITTLE_ENDIAN) || defined(__LITTLE_ENDIAN__) || defined(__i386__) || defined(__amd64__) || defined(_M_IX86) || defined(_M_AMD64)
  #define ENDIAN_LSB
  constexpr auto endian() -> Endian { return Endian::LSB; }
#elif (defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && __BYTE_ORDER == __BIG_ENDIAN) || defined(__BIG_ENDIAN__) || defined(__powerpc__) || defined(_M_PPC)
  #define ENDIAN_MSB
  constexpr auto endian() -> Endian { return Endian::MSB; }
#else
  #warning "unable to detect endian"
  #define ENDIAN_UNKNOWN
  constexpr auto endian() -> Endian { return Endian::Unknown; }
#endif

}

/* Build optimization level detection */

#undef DEBUG
#undef NDEBUG

namespace nall {

#if defined(BUILD_DEBUG)
  #define DEBUG
  constexpr auto build() -> Build { return Build::Debug; }
#elif defined(BUILD_STABLE)
  #define DEBUG
  constexpr auto build() -> Build { return Build::Stable; }
#elif defined(BUILD_SIZE)
  #define NDEBUG
  constexpr auto build() -> Build { return Build::Size; }
#elif defined(BUILD_RELEASE)
  #define NDEBUG
  constexpr auto build() -> Build { return Build::Release; }
#elif defined(BUILD_PERFORMANCE)
  #define NDEBUG
  constexpr auto build() -> Build { return Build::Performance; }
#else
  //default to debug mode
  #define DEBUG
  constexpr auto build() -> Build { return Build::Debug; }
#endif

}
