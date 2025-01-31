#----------------------------------*-CMake-*----------------------------------#
# Copyright 2022 UT-Battelle, LLC and other Celeritas developers.
# See the top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#[=======================================================================[.rst:

FindVecGeom
-----------

Find the VecGeom library and set up library linking and flags for use with
Celeritas.

#]=======================================================================]

find_package(VecGeom QUIET CONFIG)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VecGeom CONFIG_MODE)

if(VecGeom_FOUND)
  get_target_property(_vecgeom_lib_type VecGeom::vecgeom TYPE)
  if ("x${_vecgeom_lib_type}" STREQUAL "xSTATIC_LIBRARY")
     set(_vecgeom_cuda_runtime "Static")
  else()
     set(_vecgeom_cuda_runtime "Shared")
  endif()
  set_target_properties(VecGeom::vecgeom PROPERTIES
    CELERITAS_CUDA_STATIC_LIBRARY VecGeom::vecgeomcuda_static
    CELERITAS_CUDA_MIDDLE_LIBRARY VecGeom::vecgeomcuda
    CELERITAS_CUDA_FINAL_LIBRARY VecGeom::vecgeomcuda
  )
  if(CELERITAS_USE_CUDA)
    set_target_properties(VecGeom::vecgeomcuda PROPERTIES
      CELERITAS_CUDA_LIBRARY_TYPE Shared
      #CUDA_RUNTIME_LIBRARY ${_vecgeom_cuda_runtime}
    )
    set_target_properties(VecGeom::vecgeomcuda_static PROPERTIES
      CELERITAS_CUDA_LIBRARY_TYPE Static
    )
    # Suppress warnings from virtual function calls in RDC
    foreach(_lib VecGeom::vecgeomcuda VecGeom::vecgeomcuda_static)
      target_compile_options(${_lib}
        INTERFACE "$<$<COMPILE_LANGUAGE:CUDA>:SHELL: -Xnvlink --suppress-stack-size-warning>"
      )
      target_link_options(${_lib}
        INTERFACE "$<DEVICE_LINK:SHELL: -Xnvlink --suppress-stack-size-warning>"
      )
    endforeach()

    # Inform celeritas_add_library code
    foreach(_lib VecGeom::vecgeom VecGeom::vecgeomcuda
        VecGeom::vecgeomcuda_static)
      set_target_properties(${_lib} PROPERTIES
        CELERITAS_CUDA_STATIC_LIBRARY VecGeom::vecgeomcuda_static
        CELERITAS_CUDA_MIDDLE_LIBRARY VecGeom::vecgeomcuda
        CELERITAS_CUDA_FINAL_LIBRARY VecGeom::vecgeomcuda
      )
    endforeach()
  endif()
endif()

#-----------------------------------------------------------------------------#
