#=========================================================================
# Copyright (C) 2018 The C++ Component Model(COMO) Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#=========================================================================

set(UTIL_DIR ${LIBCORE_COMO_SRC_DIR}/util)

if("$ENV{PLATFORM}" STREQUAL "openEuler")
    include_directories(
        ${EXTERNAL_DIR}/../prebuilt/icu4c/icu4c/source/common
    )
endif()

include_directories(
    ./
    ../
    ../../../inc
    ${INC_DIR}
    ${LIBCORE_OBJ_DIR})

add_subdirectory(calendar)
add_subdirectory(concurrent)
add_subdirectory(locale)
add_subdirectory(regex)

set(SOURCES
    AbstractCollection.cpp
    AbstractList.cpp
    AbstractMap.cpp
    AbstractQueue.cpp
    AbstractSequentialList.cpp
    AbstractSet.cpp
    ArrayList.cpp
    Arrays.cpp
    Calendar.cpp
    CalendarFactory.cpp
    Collections.cpp
    Currency.cpp
    Date.cpp
    EmptyArray.cpp
    Formatter.cpp
    GregorianCalendar.cpp
    Hashtable.cpp
    HashMap.cpp
    HashSet.cpp
    JapaneseImperialCalendar.cpp
    LinkedHashMap.cpp
    LinkedHashSet.cpp
    LinkedList.cpp
    Locale.cpp
    LocaleFactory.cpp
    Properties.cpp
    Random.cpp
    SimpleTimeZone.cpp
    StringTokenizer.cpp
    TimeZone.cpp
    TimeZoneFactory.cpp
    TimeZoneGetter.cpp
    TreeMap.cpp
    TreeNode.cpp
    TreeSet.cpp
    Vector.cpp
    XMLUtils.cpp
    MemUtils.cpp
    util.cpp)

add_library(como_util STATIC
    ${SOURCES})
add_dependencies(como_util build_libcore)