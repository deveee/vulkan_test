# - Find Vulkan
# Find the Vulkan includes and libraries
#
# Following variables are provided:
# VULKAN_FOUND
#     True if Vulkan has been found
# VULKAN_INCLUDE_DIR
#     The include directory of Vulkan
# VULKAN_LIBRARY
#     Vulkan library list

find_path(VULKAN_INCLUDE_DIR vulkan/vulkan.h)
find_library(VULKAN_LIBRARY NAMES vulkan)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VULKAN DEFAULT_MSG VULKAN_LIBRARY VULKAN_INCLUDE_DIR)

mark_as_advanced(VULKAN_LIBRARY VULKAN_INCLUDE_DIR)
