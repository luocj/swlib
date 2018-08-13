#include "base_log_macro.h"

BaseLogStream core_log("core", BaseLogStreamInterface::debug);
BaseLogStream server_log("server", BaseLogStreamInterface::debug,false);