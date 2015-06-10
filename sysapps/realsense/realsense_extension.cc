// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/realsense/realsense_extension.h"

#include "grit/xwalk_sysapps_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/sysapps/realsense/realsense.h"
#include "xwalk/sysapps/realsense/realsense_object.h"
#include "xwalk/sysapps/realsense/sceneperception_object.h"

namespace xwalk {
namespace sysapps {
namespace experimental {

using jsapi::realsense::RealSenseConstructor::Params;

RealSenseExtension::RealSenseExtension() {
  set_name("xwalk.experimental.realsense");
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_REALSENSE_API).as_string());
}

RealSenseExtension::~RealSenseExtension() {}

XWalkExtensionInstance* RealSenseExtension::CreateInstance() {
  return new RealSenseInstance();
}

RealSenseInstance::RealSenseInstance()
  : handler_(this),
    store_(&handler_) {
  handler_.Register("realsenseConstructor",
      base::Bind(&RealSenseInstance::OnRealSenseConstructor,
                 base::Unretained(this)));
  handler_.Register("sceneperceptionConstructor",
      base::Bind(&RealSenseInstance::OnScenePerceptionConstructor,
                 base::Unretained(this)));
}

void RealSenseInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  handler_.HandleMessage(msg.Pass());
}

void RealSenseInstance::OnRealSenseConstructor(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  scoped_ptr<Params> params(Params::Create(*info->arguments()));

  scoped_ptr<BindingObject> obj(new RealSenseObject());
  store_.AddBindingObject(params->object_id, obj.Pass());
}

void RealSenseInstance::OnScenePerceptionConstructor(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  scoped_ptr<Params> params(Params::Create(*info->arguments()));

  scoped_ptr<BindingObject> obj(new ScenePerceptionObject());
  store_.AddBindingObject(params->object_id, obj.Pass());
}

}  // namespace experimental
}  // namespace sysapps
}  // namespace xwalk
