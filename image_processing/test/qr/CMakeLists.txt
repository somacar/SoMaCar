cmake_minimum_required(VERSION 3.5)
project( qr_code )
find_package( OpenCV REQUIRED )
add_executable( qr_image image.cpp )
add_executable( qr_video video.cpp )
target_link_libraries( qr_image ${OpenCV_LIBS} )
target_link_libraries( qr_video ${OpenCV_LIBS} )