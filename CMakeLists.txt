add_subdirectory(src)

set(install_dir ${CMAKE_INSTALL_PREFIX}/online_collab)
set(lib_files $<TARGET_FILE:kritaonlinecollab>)

install(FILES ${lib_files} DESTINATION ${install_dir}/online_collab/lib)
install(DIRECTORY python/ DESTINATION ${install_dir}/online_collab)
install(FILES online_collab.desktop DESTINATION ${install_dir})
install(FILES $<TARGET_FILE:kritaonlinecollab> DESTINATION ${install_dir}/online_collab/lib)
