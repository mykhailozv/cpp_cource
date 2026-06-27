#define ENABLE_LOG	1
#define ENABLE_DEBUG  0
#define ENABLE_ERROR 1

#if ENABLE_LOG
  #define LOG(msg) std::cout << "[LOG] " << msg << std::endl
#else
  #define LOG(msg)
#endif
 
#if ENABLE_DEBUG
  #define DEBUG(msg) std::cout << "[DEBUG] " << msg << std::endl
#else
  #define DEBUG(msg)
#endif

#if ENABLE_ERROR
  #define ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl
#else
  #define ERROR(msg)
#endif
