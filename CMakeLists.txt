add_falcor_executable(Raytracing)

target_sources(Raytracing PRIVATE
    Raytracing.cpp
    Raytracing.h
    Raytracing.3d.slang
    Raytracing.rt.slang
    triangle.slang
    SDF_Functions.slang
)

target_copy_shaders(Raytracing Samples/Raytracing)

target_source_group(Raytracing "Samples")
