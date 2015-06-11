// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_REALSENSE_REALSENSE_EXTENSION_H_
#define XWALK_SYSAPPS_REALSENSE_REALSENSE_EXTENSION_H_

#include <string>

#include "base/values.h"
#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/sysapps/common/binding_object_store.h"

namespace xwalk {
namespace sysapps {
namespace experimental {

using extensions::XWalkExtension;
using extensions::XWalkExtensionFunctionHandler;
using extensions::XWalkExtensionFunctionInfo;
using extensions::XWalkExtensionInstance;

class RealSenseExtension : public XWalkExtension {
 public:
  RealSenseExtension();
  ~RealSenseExtension() override;

  // XWalkExtension implementation.
  XWalkExtensionInstance* CreateInstance() override;
};

class RealSenseInstance : public XWalkExtensionInstance {
 public:
  RealSenseInstance();

  // XWalkExtensionInstance implementation.
  void HandleMessage(scoped_ptr<base::Value> msg) override;

 private:
  void OnRealSenseConstructor(
      scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnScenePerceptionConstructor(
      scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnScan3DConstructor(
      scoped_ptr<XWalkExtensionFunctionInfo> info);

  XWalkExtensionFunctionHandler handler_;
  BindingObjectStore store_;
};

}  // namespace experimental
}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_REALSENSE_REALSENSE_EXTENSION_H_
