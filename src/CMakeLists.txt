set(kritaonlinecollab_SOURCES
    OnlineCollab.cpp
    OnlineCollabDock.cpp
)

kis_add_library(kritaonlinecollab MODULE ${kritaonlinecollab_SOURCES})

generate_export_header(kritaonlinecollab BASE_NAME kritaonlinecollab)

target_compile_features(kritaonlinecollab PRIVATE cxx_std_20)
target_link_libraries(kritaonlinecollab PRIVATE kritaui kritabasicflakes kritaimage)

set_target_properties(kritaonlinecollab PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN"
)
