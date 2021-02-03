project (fdlibm)

#need to change optimization level to O1 in visual studio
#so intrinsics aren't included
if(MSVC)
    add_compile_options("/W4" "$<$<CONFIG:RELEASE>:/O1>")
else()
    add_compile_options("-Wall" "-Wextra" "-Werror" "$<$<CONFIG:RELEASE>:-O3>" "-fno-strict-aliasing")
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options("-stdlib=libc++")
    else()
        # nothing special for gcc at the moment
    endif()
endif()


#
#extlib - javascript
#
SET(FDLIBM_SOURCE_FILES
	src/k_standard.c
	src/k_rem_pio2.c
	src/k_cos.c
	src/k_sin.c
	src/k_tan.c
	src/e_acos.c
	src/e_acosh.c
	src/e_asin.c
	src/e_atan2.c
	src/e_atanh.c
	src/e_cosh.c
	src/e_exp.c
	src/e_fmod.c
	src/e_gamma.c
	src/e_gamma_r.c
	src/e_hypot.c
	src/e_j0.c
	src/e_j1.c
	src/e_jn.c
	src/e_lgamma.c
	src/e_lgamma_r.c
	src/e_log.c
	src/e_log10.c
	src/e_pow.c
	src/e_rem_pio2.c
	src/e_remainder.c
	src/e_scalb.c
	src/e_sinh.c
	src/e_sqrt.c
	src/w_acos.c
	src/w_acosh.c
	src/w_asin.c
	src/w_atan2.c
	src/w_atanh.c
	src/w_cosh.c
	src/w_exp.c
	src/w_fmod.c
	src/w_gamma.c
	src/w_gamma_r.c
	src/w_hypot.c
	src/w_j0.c
	src/w_j1.c
	src/w_jn.c
	src/w_lgamma.c
	src/w_lgamma_r.c
	src/w_log.c
	src/w_log10.c
	src/w_pow.c
	src/w_remainder.c
	src/w_scalb.c
	src/w_sinh.c
	src/w_sqrt.c
	src/s_asinh.c
	src/s_atan.c
	src/s_cbrt.c
	src/s_ceil.c
	src/s_copysign.c
	src/s_cos.c
	src/s_erf.c
	src/s_expm1.c
	src/s_fabs.c
	src/s_finite.c
	src/s_floor.c
	src/s_frexp.c
	src/s_ilogb.c
	src/s_isnan.c
	src/s_ldexp.c
	src/s_lib_version.c
	src/s_log1p.c
	src/s_logb.c
	src/s_matherr.c
	src/s_modf.c
	src/s_nextafter.c
	src/s_rint.c
	src/s_scalbn.c
	src/s_signgam.c
	src/s_significand.c
	src/s_sin.c
	src/s_tan.c
	src/s_tanh.c
)

SET(FDLIBM_INCL_FILES
	src/fdlibm.h
)

add_library (fdlibm ${FDLIBM_SOURCE_FILES} ${FDLIBM_INCLUDE_FILES})
target_compile_definitions(fdlibm PUBLIC "_IEEE_LIBM")

# Make sure the compiler can find include files for our library
# when other libraries or executables link to fdlibm
target_include_directories (fdlibm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


INSTALL(FILES ${FDLIBM_INCL_FILES} DESTINATION include)
INSTALL(FILES ${FDLIBM_SOURCE_FILES} DESTINATION src)
INSTALL(TARGETS fdlibm
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
