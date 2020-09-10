#ifndef _GUID_956693f6_9951_4895_b673_e71a001b281a_IPersistent_HPP_
#define _GUID_956693f6_9951_4895_b673_e71a001b281a_IPersistent_HPP_

namespace common {

class IPersistent {

public:
  IPersistent() {}
  virtual ~IPersistent() {}

  virtual void persist() const = 0;
};

} // namespace common

#endif /* _GUID_956693f6_9951_4895_b673_e71a001b281a_IPersistent_HPP_ */
