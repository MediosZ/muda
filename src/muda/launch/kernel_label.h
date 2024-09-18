#pragma once
#include <muda/tools/launch_info_cache.h>
#include <string_view>
namespace muda
{
class KernelLabel
{
  public:
    KernelLabel(std::string_view name, std::string_view file = "", size_t line = ~0ull)
    {
        if constexpr(muda::RUNTIME_CHECK_ON)
        {
            details::LaunchInfoCache::current_kernel_name(name);
            details::LaunchInfoCache::current_kernel_file(file);
            details::LaunchInfoCache::current_kernel_line(line);
        }
    }

    ~KernelLabel()
    {
        if constexpr(muda::RUNTIME_CHECK_ON)
        {
            details::LaunchInfoCache::current_kernel_name("");
            details::LaunchInfoCache::current_kernel_file("");
            details::LaunchInfoCache::current_kernel_line(~0ull);
        }
    }
};
}  // namespace muda
