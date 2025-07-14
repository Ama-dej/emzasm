file(REMOVE_RECURSE
  "libemzasmlib.a"
  "libemzasmlib.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/emzasmlib.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
