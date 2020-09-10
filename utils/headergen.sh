#!/usr/bin/env bash


uuid=$(uuidgen -r)
uuid="${uuid//-/_}"
namespace=$1
name=$2

echo "#ifndef _GUID_${uuid}_${name}_HPP_
#define _GUID_${uuid}_${name}_HPP_

namespace ${namespace} {


} // namespace ${namespace}

#endif /* _GUID_${uuid}_${name}_HPP_ */"
