cmake_minimum_required (VERSION 3.5)
project(nclcomposer_core)

include_directories (src)

add_definitions(-DCOMPOSERCORE_LIBRARY)

set (CMAKE_INCLUDE_CURRENTDIR ON)
set (CMAKE_AUTOMOC ON)

find_package (Qt5Xml REQUIRED)
find_package (Qt5Widgets REQUIRED)

set(NCLCOMPOSER_CORE_HEADERS
# Modules
  src/modules/PluginControl.h
  src/modules/MessageControl.h
  src/modules/LanguageControl.h
  src/modules/ProjectControl.h
  src/modules/ProjectReader.h

# Extensions
  src/extensions/ILanguageProfile.h
  src/extensions/IDocumentParser.h
  src/extensions/IPlugin.h
  src/extensions/IPluginFactory.h

# Util
  src/util/ComposerCoreControl_global.h
  src/util/Singleton.h
  src/util/AttributeReferences.h
  src/util/Utilities.h
  src/util/ComposerSettings.h
  src/util/Commands.h

# Exceptions
  src/model/exception/EntityNotFound.h
  src/model/exception/ParentNotFound.h
  src/model/exception/CreationFailed.h

# Model
  src/model/Entity.h
  src/model/Project.h
  #include/model/Model.h
)

set(NCLCOMPOSER_CORE_SOURCES
  src/model/Entity.cpp
  src/model/Project.cpp
  src/model/Project.cpp
  src/modules/LanguageControl.cpp
  src/modules/MessageControl.cpp
  src/modules/PluginControl.cpp
  src/modules/ProjectControl.cpp
  src/modules/ProjectReader.cpp
  src/util/AttributeReferences.cpp
  src/util/Commands.cpp
  src/util/ComposerSettings.cpp
  src/util/Utilities.cpp
)

add_library(nclcomposer_core
  SHARED
  ${NCLCOMPOSER_CORE_SOURCES}
  ${NCLCOMPOSER_CORE_HEADERS}
)

target_link_libraries(
  nclcomposer_core
  Qt5::Xml
  Qt5::Widgets
)

install (TARGETS nclcomposer_core DESTINATION bin)
