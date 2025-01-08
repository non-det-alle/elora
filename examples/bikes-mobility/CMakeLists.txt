build_lib_example(
  NAME bikes-mobility-example
  SOURCE_FILES bikes-mobility-example.cc
  LIBRARIES_TO_LINK
    ${libelora}
)

build_lib_example(
  NAME netanim-bikes-example
  SOURCE_FILES netanim-bikes-example.cc
  LIBRARIES_TO_LINK
    ${libelora}
    ${libnetanim}
)

build_lib_example(
  NAME elora-bikes-example
  SOURCE_FILES elora-bikes-example.cc
  LIBRARIES_TO_LINK
    ${libelora}
    ${libtap-bridge}
    ${libcsma}
)
