cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(CEX2REX)

include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "CEX 2 REX 2.0")
set(VITA_TITLEID  "CEX2REX20")

set(VITA_VERSION  "02.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -s CATEGORY=gdb")
set(VITA_MAKE_FSELF_FLAGS "${VITA_MAKE_FSELF_FLAGS}")

include_directories(
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME}
  src/main.c
  src/ctrl.c
  src/pup.c
)

target_link_libraries(${PROJECT_NAME}
	c
	SceLibKernel_stub
	SceIofilemgr_stub
	SceAppMgr_stub
	SceCtrl_stub
	SceDisplay_stub
	SceVshBridge_stub
	SceShellSvc_stub
	ScePower_stub
	SceRegistryMgr_stub
	Cex2Rex_User_stub_weak
	taihen_stub
)

vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE)

vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}

  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
  FILE user.suprx user.suprx  
  FILE kernel.skprx kernel.skprx
  FILE kmspico.skprx kmspico.skprx
  FILE testkit.skprx testkit.skprx
  FILE vitaConfig.txt vitaConfig.txt
  FILE pstvConfig.txt pstvConfig.txt
  FILE app.info app.info
)