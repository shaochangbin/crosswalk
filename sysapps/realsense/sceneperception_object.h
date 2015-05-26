// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_REALSENSE_SCENEPERCEPTION_OBJECT_H_
#define XWALK_SYSAPPS_REALSENSE_SCENEPERCEPTION_OBJECT_H_

#include <string>

#include "pxcsensemanager.h"  // NOLINT

#include "xwalk/sysapps/common/event_target.h"

class ScenePerceptionController;

namespace xwalk {
namespace sysapps {

class ScenePerceptionObject :
    public EventTarget,
    public PXCSenseManager::Handler {
 public:
  ScenePerceptionObject();
  ~ScenePerceptionObject() override;

  // EventTarget implementation.
  void StartEvent(const std::string& type) override;
  void StopEvent(const std::string& type) override;
  
  // PXCSenseManager::Handler implementation.
  virtual pxcStatus PXCAPI OnModuleProcessedFrame(pxcUID mid, PXCBase* module, PXCCapture::Sample* sample);
  virtual void PXCAPI OnStatus(pxcUID mid, pxcStatus status);

 private:
  void OnStart(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnStop(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnReset(scoped_ptr<XWalkExtensionFunctionInfo> info);
  
 private:
  bool started_;
  scoped_ptr<ScenePerceptionController> sceneperception_controller_;
  
  int color_image_width_;
  int color_image_height_;
  float color_capture_framerate_;
  int depth_image_width_;
  int depth_image_height_;
  float depth_capture_framerate_;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_REALSENSE_SCENEPERCEPTION_OBJECT_H_
