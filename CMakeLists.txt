cmake_minimum_required(VERSION 3.0.2)
project(ccf_person_identification)

add_compile_options(-std=c++14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake")

# find_package：フォルダを検索する
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Flann REQUIRED)
find_package(catkin REQUIRED COMPONENTS
  roscpp
  roslib
  cv_bridge
  message_generation
  message_filters
  sobits_msgs
  open_face_recognition
)

#add_message_files(
#  FILES
#)

generate_messages(
  DEPENDENCIES
  std_msgs
)

# set dlib directory to DLIB_ROOT
# include($ENV{DLIB_ROOT}/dlib/cmake)
# set(DLIB_ROOT "/dlib")
include(dlib/CMakeLists.txt)

# add_subdirectory(dlib)

###################################
## catkin specific configuration ##
###################################

catkin_package(
  INCLUDE_DIRS include
#  LIBRARIES ccf_person_identification
#  CATKIN_DEPENDS dlib_project
#  DEPENDS 
)

###########
## Build ##
###########

include_directories(
  include
  ${EIGEN3_INCLUDE_DIRS}
  ${FLANN_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
)



#add_library(ccf_person_identification
#  src/ccf_person_identification.cpp
#)

#target_link_libraries(ccf_person_identification
#  ${catkin_LIBRARIES}
#  ${OpenCV_LIBRARIES}
#  ${FLANN_LIBRARIES}
#  dlib::dlib
#)

#add_dependencies(ccf_person_identification ${ccf_person_identification_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})


add_executable(ccf_person_identification_test src/ccf_person_identification_test.cpp)
target_link_libraries(ccf_person_identification_test
  ${catkin_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${FLANN_LIBRARIES}
  dlib::dlib
)