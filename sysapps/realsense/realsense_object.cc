// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/realsense/realsense_object.h"

#include "xwalk/sysapps/realsense/realsense.h"

#include <string>

namespace xwalk {
namespace sysapps {

using namespace jsapi::realsense; // NOLINT

RealSenseObject::RealSenseObject() {
  handler_.Register("getVersion",
                    base::Bind(&RealSenseObject::OnGetVersion,
                               base::Unretained(this)));
}

RealSenseObject::~RealSenseObject() {
}

void RealSenseObject::StartEvent(const std::string& type) {
  NOTIMPLEMENTED();
}

void RealSenseObject::StopEvent(const std::string& type) {
  NOTIMPLEMENTED();
}

void RealSenseObject::OnGetVersion(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  scoped_ptr<base::ListValue> data(new base::ListValue());
  data->AppendString("hello realsense");
  info->PostResult(data.Pass());
}

}  // namespace sysapps
}  // namespace xwalk
