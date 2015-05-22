// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_REALSENSE_REALSENSE_OBJECT_H_
#define XWALK_SYSAPPS_REALSENSE_REALSENSE_OBJECT_H_

#include <string>
#include "xwalk/sysapps/common/event_target.h"

namespace xwalk {
namespace sysapps {

class RealSenseObject : public EventTarget {
 public:
  RealSenseObject();
  ~RealSenseObject() override;

  // EventTarget implementation.
  void StartEvent(const std::string& type) override;
  void StopEvent(const std::string& type) override;

 private:
  void OnGetVersion(scoped_ptr<XWalkExtensionFunctionInfo> info);
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_REALSENSE_REALSENSE_OBJECT_H_
