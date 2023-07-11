build_lib_example(
  NAME VM_Example
  SOURCE_FILES VM_Example.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${liblorawan}
)

build_lib_example(
  NAME VM_with_Application_Class
  SOURCE_FILES VM_with_Application_Class.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${liblorawan}
)

build_lib_example(
  NAME VM_without_Application_Class
  SOURCE_FILES VM_without_Application_Class.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${liblorawan}
)

build_lib_example(
  NAME Waypoint_Mobility_test_Example
  SOURCE_FILES Waypoint_Mobility_test_Example.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${liblorawan}
)

build_lib_example(
  NAME WM_Adding_waypoints_from_Data_Set_Example
  SOURCE_FILES WM_Adding_waypoints_from_Data_Set_Example.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${liblorawan}
)

build_lib_example(
  NAME WM_with_unix_time_example
  SOURCE_FILES WM_with_unix_time_example.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${liblorawan}
)
