// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_REALSENSE_SCENEPERCEPTION_OBJECT_H_
#define XWALK_SYSAPPS_REALSENSE_SCENEPERCEPTION_OBJECT_H_

#include <string>

#include "pxcsensemanager.h"  // NOLINT
#include "pxcsceneperception.h" // NOLINT

#include "xwalk/sysapps/common/event_target.h"

#include "base/threading/thread.h"

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
  virtual pxcStatus PXCAPI OnNewSample(pxcUID mid, PXCCapture::Sample* sample);
  virtual pxcStatus PXCAPI OnModuleProcessedFrame(pxcUID mid, PXCBase* module, PXCCapture::Sample* sample);
  virtual void PXCAPI OnStatus(pxcUID mid, pxcStatus status);

 private:
  void OnStart(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnStop(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnReset(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnEnableTracking(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnDisableTracking(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnEnableMeshing(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnDisableMeshing(scoped_ptr<XWalkExtensionFunctionInfo> info);
  
  void OnDoMeshingUpdate();
  void OnMeshingResult();
  
 private:
  enum State {
    IDLE,
    CHECKING,
    TRACKING,
    MESHING,
  };
  State state_;
  
  bool on_checking_;
  bool on_tracking_;
  bool on_meshing_;
  
  bool doing_meshing_updating_;
  base::Thread meshing_thread_;
  base::MessageLoop* extension_message_loop_;

  scoped_ptr<ScenePerceptionController> sceneperception_controller_;
  
  int color_image_width_;
  int color_image_height_;
  float color_capture_framerate_;
  int depth_image_width_;
  int depth_image_height_;
  float depth_capture_framerate_;
  
  PXCBlockMeshingData* block_meshing_data_;
  PXCScenePerception::MeshingUpdateInfo  meshing_update_info_;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_REALSENSE_SCENEPERCEPTION_OBJECT_H_
