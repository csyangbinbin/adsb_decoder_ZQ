INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
AUX_SOURCE_DIRECTORY(tinyxml TINYXML_SRC)
SET(SRCFILE ../adsb_decoder_api.h HF_ADSB_Decoder.cpp)
ADD_EXECUTABLE(demo ${TINYXML_SRC} ${SRCFILE} )
TARGET_LINK_LIBRARIES(demo hfdecoder )
