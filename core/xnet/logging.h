#ifdef __cplusplus

#define GOOGLE_GLOG_DLL_DECL           
#define GLOG_NO_ABBREVIATED_SEVERITIES

#include <glog/logging.h>

#define LOG_TRACE LOG(INFO)
#define LOG_DEBUG LOG(INFO)
#define LOG_INFO  LOG(INFO)
#define DLOG_TRACE LOG(INFO) << __PRETTY_FUNCTION__ << " this=" << this << " "
#define LOG_WARN  LOG(WARNING)
#define LOG_ERROR LOG(INFO)
#define LOG_FATAL LOG(FATAL)
#define LOG_RELEASE LOG(ERROR)

#endif // end of define __cplusplus
