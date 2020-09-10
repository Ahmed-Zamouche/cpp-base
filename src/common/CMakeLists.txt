set(COMMON_HDRS
    Assert.h
    logger/Logger.hpp
    logger/ILoggable.hpp
    IPublisher.hpp
    ISubscriber.hpp
    IPersistent.hpp
    concurrent/SpinLock.h
    concurrent/Semaphore.hpp
    concurrent/Actor.hpp
)

set(COMMON_SRCS
    logger/Logger.cpp
    IPublisher.cpp
    ISubscriber.cpp
    concurrent/Actor.cpp
)

add_library(common SHARED STATIC ${COMMON_SRCS} ${COMMON_HDRS})

target_link_libraries(common PRIVATE fmt)

install(FILES ${COMMON_HDRS} DESTINATION include/common)
install(TARGETS common DESTINATION lib)

add_subdirectory(test)