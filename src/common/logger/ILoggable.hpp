#ifndef _GUID_3b2a3b9f_3aa0_4c38_ad4f_f088e4545031_ILoggable_HPP_
#define _GUID_3b2a3b9f_3aa0_4c38_ad4f_f088e4545031_ILoggable_HPP_

#include "Logger.hpp"

namespace common {
namespace logger {
class ILoggable {

public:
  ILoggable() {}
  virtual ~ILoggable() {}
  virtual void setLevel(const Level level) = 0;
  virtual Logger getLogger() const = 0;
};
} // namespace logger
} // namespace common

#endif /* _GUID_3b2a3b9f_3aa0_4c38_ad4f_f088e4545031_ILoggable_HPP_ */
