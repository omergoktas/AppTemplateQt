add_library(pch OBJECT pch.cpp $<$<BOOL:${APPLE}>:pch.mm>)
target_precompile_headers(pch PRIVATE pch.h)
target_link_libraries(pch PUBLIC Qt::Core)
