function(copy_presets target target_file_dir folder_name return)
  set(new_target_name ${target}_install_vst3_${folder_name})
  add_custom_target( ${new_target_name} ALL
    ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/presets/${folder_name}
    ${target_file_dir}/../Resources/${folder_name})
    set(${return} ${new_target_name} PARENT_SCOPE)
endfunction()

function(copy_all_presets target target_file_dir)
  copy_presets(${target} ${target_file_dir} presets r1)

  add_custom_target(${target}_install_all)
  add_dependencies(${target}_install_all ${r1})

  add_dependencies(${target}_install_all ${target})

endfunction()
