SET(VEDA_INSTALL_DEFAULT	"/opt/nec/ve")
SET(VEDA_INSTALL_PATH		"share/veoffload-veda")
SET(CPACK_PACKAGE_NAME		"veoffload-veda")
SET(AVEO_PATH			"/opt/nec/ve/veos")
SET(AVEO_INCLUDE_DIRS		"${AVEO_PATH}/include")
SET(AVEO_LIBRARY		"${AVEO_PATH}/lib64/libveo.so.1.0.0")
SET(AVEO_LIBRARIES ${AVEO_LIBRARY})
SET(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /opt/nec/ve/bin /opt/nec/ve/lib /opt/nec/ve/share)
ADD_DEFINITIONS(-DBUILD_VEOS_RELEASE=1)
