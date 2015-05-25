// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/realsense/realsense_object.h"

#include "xwalk/sysapps/realsense/realsense.h"

#include "pxcsession.h" // NOLINT

#include <string>
#include <sstream>

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
  PXCSession *session = PXCSession::CreateInstance();
  PXCSession::ImplVersion ver = session->QueryVersion();
  session->Release();
  std::ostringstream major, minor;
  major << ver.major;
  minor << ver.minor;

  scoped_ptr<Version> version(new Version());
  version->major = major.str();
  version->minor = minor.str();
  info->PostResult(GetVersion::Results::Create(*version, std::string()));
}

}  // namespace sysapps
}  // namespace xwalk
