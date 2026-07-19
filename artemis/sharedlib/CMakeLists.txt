# created by xmake

cmake_minimum_required(VERSION 3.21)
cmake_policy(SET CMP0091 NEW)

project(artemis)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

if(WIN32)
	list(APPEND COMPILE_OPTIONS /permissive-)
endif()


#=======================================================================================
#    libraries
#=======================================================================================

find_package(PkgConfig REQUIRED)



# lib: zlib (mandatory)
find_package(ZLIB REQUIRED)
list(APPEND DEPS ZLIB::ZLIB)
list(APPEND COMPILE_DEFINITIONS HAS_LIB_ZLIB=1)


# lib: vulkan or gl (mandatory)
set(VULKAN_OR_GL "vulkan" CACHE STRING "Use vulkan or gl?")
find_package(Vulkan COMPONENTS shaderc_combined glslang)
if(VULKAN_FOUND)
	list(APPEND VULKAN_OR_GL_FOUND "vulkan")
endif()
find_package(OpenGL)
if(OPENGL_FOUND AND NOT APPLE)
	list(APPEND VULKAN_OR_GL_FOUND "gl")
endif()
if(VULKAN_OR_GL_FOUND)
	if(NOT ${VULKAN_OR_GL} IN_LIST VULKAN_OR_GL_FOUND)
		list(GET VULKAN_OR_GL_FOUND 0 X)
		set(VULKAN_OR_GL ${X} CACHE STRING "choose" FORCE)
		message(WARNING "Choice for VULKAN_OR_GL invalid, using ${VULKAN_OR_GL} instead")
	endif()
else()
	message(SEND_ERROR "Neither vulkan nor gl found")
	set(VULKAN_OR_GL "")
endif()
if(VULKAN_OR_GL STREQUAL "vulkan")
	if(WIN32)
		list(APPEND DEPS Vulkan::Vulkan Vulkan::glslang)
	else()
		list(APPEND DEPS Vulkan::Vulkan Vulkan::shaderc_combined Vulkan::glslang)
	endif()
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_VULKAN=1)
endif()
if(VULKAN_OR_GL STREQUAL "gl")
	list(APPEND DEPS OpenGL::GL)
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_GL=1)
endif()


# lib: glew (optional)
set(LIB_GLEW_ENABLED true CACHE BOOL "Use glew library?")
find_package(GLEW)
if(GLEW_FOUND)
else()
	set(LIB_GLEW_ENABLED false CACHE BOOL "glew library not found" FORCE)
endif()
if(${LIB_GLEW_ENABLED})
	list(APPEND DEPS GLEW::GLEW)
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_GLEW=1)
else()
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_GLEW=0)
endif()


# lib: glfw (optional)
set(LIB_GLFW_ENABLED true CACHE BOOL "Use glfw library?")
find_package(glfw3)
if(glfw3_FOUND)
else()
	set(LIB_GLFW_ENABLED false CACHE BOOL "glfw library not found" FORCE)
endif()
if(${LIB_GLFW_ENABLED})
	list(APPEND DEPS glfw)
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_GLFW=1)
else()
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_GLFW=0)
endif()


# lib: spirv (optional)
set(LIB_SPIRV_ENABLED true CACHE BOOL "Use spirv library?")
pkg_check_modules(SPIRV spirv)
if(SPIRV_FOUND)
else()
	set(LIB_SPIRV_ENABLED false CACHE BOOL "spirv library not found" FORCE)
endif()
if(${LIB_SPIRV_ENABLED})
	list(APPEND INCLUDE_DIRECTORIES ${SPIRV_INCLUDE_DIRS})
	list(APPEND LINK_DIRECTORIES ${SPIRV_LIBRARY_DIRS})
	list(APPEND DEPS ${SPIRV_LIBRARIES})
	list(APPEND COMPILE_OPTIONS ${SPIRV_CFLAGS_OTHER})
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_SPIRV=1)
else()
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_SPIRV=0)
endif()


# lib: unwind (optional)
set(LIB_UNWIND_ENABLED true CACHE BOOL "Use unwind library?")
pkg_check_modules(LIBUNWIND libunwind)
if(LIBUNWIND_FOUND)
else()
	set(LIB_UNWIND_ENABLED false CACHE BOOL "unwind library not found" FORCE)
endif()
if(${LIB_UNWIND_ENABLED})
	list(APPEND INCLUDE_DIRECTORIES ${LIBUNWIND_INCLUDE_DIRS})
	list(APPEND LINK_DIRECTORIES ${LIBUNWIND_LIBRARY_DIRS})
	list(APPEND DEPS ${LIBUNWIND_LIBRARIES})
	list(APPEND COMPILE_OPTIONS ${LIBUNWIND_CFLAGS_OTHER})
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_UNWIND=1)
else()
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_UNWIND=0)
endif()


# lib: dl (optional)
if(WIN32)
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_DL=0)
else()
	list(APPEND DEPS ${CMAKE_DL_LIBS})
	list(APPEND COMPILE_DEFINITIONS HAS_LIB_DL=1)
endif()



# lib: freetype2 (mandatory)
find_package(Freetype REQUIRED)
list(APPEND DEPS Freetype::Freetype)
list(APPEND COMPILE_DEFINITIONS HAS_LIB_FREETYPE2=1)


# lib: armadillo (mandatory)
find_package(Armadillo REQUIRED)
list(APPEND DEPS ${ARMADILLO_LIBRARIES})
list(APPEND COMPILE_DEFINITIONS HAS_LIB_ARMADILLO=1)


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
list(APPEND DEPS Threads::Threads)


find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
	list(APPEND DEPS ${MATH_LIBRARY})
endif()


#=======================================================================================
#    target moduleartemis
#=======================================================================================

add_library(moduleartemis SHARED
	src/action/graph/ActionGraphAddNode.cpp
	src/action/graph/ActionGraphConnect.cpp
	src/action/graph/ActionGraphRemoveNode.cpp
	src/action/graph/ActionGraphUnconnect.cpp
	src/action/graph/ActionNodeSetSetting.cpp
	src/action/graph/ActionNodesMove.cpp
	src/data/field/BasisFields.cpp
	src/data/field/Field.cpp
	src/data/field/MultiField.cpp
	src/data/field/ScalarField.cpp
	src/data/field/VectorField.cpp
	src/data/grid/Grid.cpp
	src/data/grid/RegularGrid.cpp
	src/data/grid/UnstructuredGrid.cpp
	src/data/util/ColorMap.cpp
	src/graph/field/MultiField.cpp
	src/graph/field/ScalarField.cpp
	src/graph/field/VectorField.cpp
	src/graph/AutoConnect.cpp
	src/graph/Graph.cpp
	src/graph/NodeFactory.cpp
	src/lib/any/any.cpp
	src/lib/any/conversion.cpp
	src/lib/base/array.cpp
	src/lib/base/pointer.cpp
	src/lib/base/strings.cpp
	src/lib/dataflow/Graph.cpp
	src/lib/dataflow/Node.cpp
	src/lib/dataflow/Port.cpp
	src/lib/dataflow/ResourceNode.cpp
	src/lib/dataflow/Setting.cpp
	src/lib/dataflow/Type.cpp
	src/lib/doc/chunked.cpp
	src/lib/doc/xml.cpp
	src/lib/history/Action.cpp
	src/lib/history/ActionGroup.cpp
	src/lib/history/ActionManager.cpp
	src/lib/history/Data.cpp
	src/lib/history/MergableAction.cpp
	src/lib/image/color.cpp
	src/lib/image/image.cpp
	src/lib/image/image_bmp.cpp
	src/lib/image/image_jpg.cpp
	src/lib/image/image_png.cpp
	src/lib/image/image_tga.cpp
	src/lib/image/ImagePainter.cpp
	src/lib/kaba/asm/asm.cpp
	src/lib/kaba/asm/asm_arm.cpp
	src/lib/kaba/asm/asm_x86.cpp
	src/lib/kaba/compiler/Backend.cpp
	src/lib/kaba/compiler/BackendAmd64.cpp
	src/lib/kaba/compiler/BackendARM.cpp
	src/lib/kaba/compiler/BackendArm64.cpp
	src/lib/kaba/compiler/BackendX86.cpp
	src/lib/kaba/compiler/CommandList.cpp
	src/lib/kaba/compiler/Compiler.cpp
	src/lib/kaba/compiler/mapper.cpp
	src/lib/kaba/compiler/Serializer.cpp
	src/lib/kaba/compiler/SerialNode.cpp
	src/lib/kaba/dynamic/call.cpp
	src/lib/kaba/dynamic/dynamic.cpp
	src/lib/kaba/dynamic/exception.cpp
	src/lib/kaba/dynamic/sorting.cpp
	src/lib/kaba/lib/dict.cpp
	src/lib/kaba/lib/extern.cpp
	src/lib/kaba/lib/lib.cpp
	src/lib/kaba/lib/lib_async.cpp
	src/lib/kaba/lib/lib_base.cpp
	src/lib/kaba/lib/lib_image.cpp
	src/lib/kaba/lib/lib_kaba.cpp
	src/lib/kaba/lib/lib_math.cpp
	src/lib/kaba/lib/lib_os.cpp
	src/lib/kaba/lib/lib_threads.cpp
	src/lib/kaba/lib/lib_time.cpp
	src/lib/kaba/lib/list.cpp
	src/lib/kaba/parser/concretifier/Concretifier.cpp
	src/lib/kaba/parser/concretifier/pipe.cpp
	src/lib/kaba/parser/concretifier/special_function.cpp
	src/lib/kaba/parser/concretifier/statement.cpp
	src/lib/kaba/parser/concretifier/statement_for.cpp
	src/lib/kaba/parser/concretifier/type_matching.cpp
	src/lib/kaba/parser/AbstractParser.cpp
	src/lib/kaba/parser/import.cpp
	src/lib/kaba/parser/lexical.cpp
	src/lib/kaba/parser/macros.cpp
	src/lib/kaba/parser/Parser.cpp
	src/lib/kaba/parser/preprocessor.cpp
	src/lib/kaba/parser/Transformer.cpp
	src/lib/kaba/syntax/Block.cpp
	src/lib/kaba/syntax/Class.cpp
	src/lib/kaba/syntax/Constant.cpp
	src/lib/kaba/syntax/Function.cpp
	src/lib/kaba/syntax/Identifier.cpp
	src/lib/kaba/syntax/Node.cpp
	src/lib/kaba/syntax/Operator.cpp
	src/lib/kaba/syntax/Statement.cpp
	src/lib/kaba/syntax/SyntaxTree.cpp
	src/lib/kaba/syntax/Variable.cpp
	src/lib/kaba/template/implicit.cpp
	src/lib/kaba/template/implicit_array.cpp
	src/lib/kaba/template/implicit_callable.cpp
	src/lib/kaba/template/implicit_dict.cpp
	src/lib/kaba/template/implicit_enum.cpp
	src/lib/kaba/template/implicit_future.cpp
	src/lib/kaba/template/implicit_list.cpp
	src/lib/kaba/template/implicit_optional.cpp
	src/lib/kaba/template/implicit_pointer.cpp
	src/lib/kaba/template/implicit_product.cpp
	src/lib/kaba/template/implicit_regular.cpp
	src/lib/kaba/template/template.cpp
	src/lib/kaba/CompilerConfiguration.cpp
	src/lib/kaba/Context.cpp
	src/lib/kaba/Interpreter.cpp
	src/lib/kaba/kaba.cpp
	src/lib/kaba/Module.cpp
	src/lib/kapi/kapi.cpp
	src/lib/linalg/_kaba_export.cpp
	src/lib/linalg/Matrix.cpp
	src/lib/math/Box.cpp
	src/lib/math/complex.cpp
	src/lib/math/interpolation.cpp
	src/lib/math/mat3.cpp
	src/lib/math/mat4.cpp
	src/lib/math/math.cpp
	src/lib/math/plane.cpp
	src/lib/math/quaternion.cpp
	src/lib/math/random.cpp
	src/lib/math/ray.cpp
	src/lib/math/rect.cpp
	src/lib/math/vec2.cpp
	src/lib/math/vec3.cpp
	src/lib/math/vec4.cpp
	src/lib/nix/_kaba_export.cpp
	src/lib/nix/nix.cpp
	src/lib/nix/nix_buffer.cpp
	src/lib/nix/nix_draw.cpp
	src/lib/nix/nix_framebuffer.cpp
	src/lib/nix/nix_light.cpp
	src/lib/nix/nix_shader.cpp
	src/lib/nix/nix_textures.cpp
	src/lib/nix/nix_vertexbuffer.cpp
	src/lib/nix/nix_view.cpp
	src/lib/obs/_kaba_export.cpp
	src/lib/obs/Log.cpp
	src/lib/obs/Observable.cpp
	src/lib/os/app.cpp
	src/lib/os/CommandLineParser.cpp
	src/lib/os/config.cpp
	src/lib/os/date.cpp
	src/lib/os/file.cpp
	src/lib/os/filesystem.cpp
	src/lib/os/formatter.cpp
	src/lib/os/msg.cpp
	src/lib/os/path.cpp
	src/lib/os/stream.cpp
	src/lib/os/terminal.cpp
	src/lib/os/time.cpp
	src/lib/polymesh/create/Ball.cpp
	src/lib/polymesh/create/Cube.cpp
	src/lib/polymesh/create/Cylinder.cpp
	src/lib/polymesh/create/Plane.cpp
	src/lib/polymesh/create/Platonic.cpp
	src/lib/polymesh/create/Sphere.cpp
	src/lib/polymesh/create/Teapot.cpp
	src/lib/polymesh/create/Torus.cpp
	src/lib/polymesh/create/TorusKnot.cpp
	src/lib/polymesh/edit/AutoMergePolygons.cpp
	src/lib/polymesh/edit/BevelEdges.cpp
	src/lib/polymesh/edit/ExtrudePolygons.cpp
	src/lib/polymesh/edit/InvertPolygons.cpp
	src/lib/polymesh/edit/TriangulatePolygons.cpp
	src/lib/polymesh/_kaba_export.cpp
	src/lib/polymesh/GeometrySubtract.cpp
	src/lib/polymesh/Mesh.cpp
	src/lib/polymesh/MeshEdit.cpp
	src/lib/polymesh/Polygon.cpp
	src/lib/polymesh/SkinGenerator.cpp
	src/lib/profiler/_kaba_export.cpp
	src/lib/profiler/Profiler.cpp
	src/lib/threads/_kaba_export.cpp
	src/lib/threads/Mutex.cpp
	src/lib/threads/Thread.cpp
	src/lib/threads/ThreadedWork.cpp
	src/lib/threads/ThreadPool.cpp
	src/lib/vulkan/_kaba_export.cpp
	src/lib/vulkan/AccelerationStructure.cpp
	src/lib/vulkan/Buffer.cpp
	src/lib/vulkan/CommandBuffer.cpp
	src/lib/vulkan/DescriptorSet.cpp
	src/lib/vulkan/Device.cpp
	src/lib/vulkan/FrameBuffer.cpp
	src/lib/vulkan/helper.cpp
	src/lib/vulkan/Instance.cpp
	src/lib/vulkan/Pipeline.cpp
	src/lib/vulkan/Queue.cpp
	src/lib/vulkan/RenderPass.cpp
	src/lib/vulkan/Semaphore.cpp
	src/lib/vulkan/Shader.cpp
	src/lib/vulkan/SwapChain.cpp
	src/lib/vulkan/Texture.cpp
	src/lib/vulkan/VertexBuffer.cpp
	src/lib/vulkan/vulkan.cpp
	src/lib/ygraphics/Context.cpp
	src/lib/ygraphics/ContextGL.cpp
	src/lib/ygraphics/ContextVulkan.cpp
	src/lib/ygraphics/font.cpp
	src/lib/ygraphics/Painter.cpp
	src/lib/ygraphics/PainterGL.cpp
	src/lib/ygraphics/PainterVulkan.cpp
	src/lib/ygraphics/ShaderManager.cpp
	src/lib/ygraphics/TextCache.cpp
	src/lib/ygraphics/TextureManager.cpp
	src/plugins/PluginManager.cpp
	src/processing/field/Calculus.cpp
	src/processing/field/IsoSurface.cpp
	src/processing/helper/GlobalThreadPool.cpp
	src/storage/FormatArtemis.cpp
	src/Artemis.cpp
	src/LibExport.cpp
	src/Session.cpp)
target_include_directories(moduleartemis PUBLIC ${INCLUDE_DIRECTORIES})
target_link_directories(moduleartemis PUBLIC ${LINK_DIRECTORIES})
target_compile_options(moduleartemis PUBLIC ${COMPILE_OPTIONS})
target_compile_definitions(moduleartemis PUBLIC ${COMPILE_DEFINITIONS}  SOURCE_ROOT="${CMAKE_SOURCE_DIR}" INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")
target_link_libraries(moduleartemis PUBLIC ${DEPS})
if (NOT WIN32)
	target_compile_options(moduleartemis PUBLIC -fvisibility=hidden)
endif()
target_include_directories(moduleartemis PUBLIC src/)
target_include_directories(moduleartemis PUBLIC src)
target_include_directories(moduleartemis PUBLIC src/y)


#=======================================================================================
#    install
#=======================================================================================

set(RESOURCE_DESTINATION share/artemis)

# binaries

# resources
install(FILES "static/icons" DESTINATION ${RESOURCE_DESTINATION})
install(FILES "static/default.theme" DESTINATION ${RESOURCE_DESTINATION})
install(FILES "static/hui_resources.txt" DESTINATION ${RESOURCE_DESTINATION})
install(FILES "static/icon.svg" DESTINATION ${RESOURCE_DESTINATION})
install(FILES "static/keys.txt" DESTINATION ${RESOURCE_DESTINATION})
install(FILES "static/license.txt" DESTINATION ${RESOURCE_DESTINATION})
install(FILES "static/license_small.txt" DESTINATION ${RESOURCE_DESTINATION})
install(FILES "static/OpenSans-Regular.ttf" DESTINATION ${RESOURCE_DESTINATION})
install(FILES "plugins/" DESTINATION ${RESOURCE_DESTINATION})