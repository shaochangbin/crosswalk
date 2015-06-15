// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_REALSENSE_SCAN3D_OBJECT_H_
#define XWALK_SYSAPPS_REALSENSE_SCAN3D_OBJECT_H_

#include <string>

#include "pxcsensemanager.h"  // NOLINT
#include "pxc3dscan.h" // NOLINT

#include "xwalk/sysapps/common/event_target.h"

#include "base/time/time.h"
#include "base/threading/thread.h"

namespace xwalk {
namespace sysapps {

class Scan3DObject :
    public EventTarget {
 public:
  Scan3DObject();
  ~Scan3DObject() override;

  // EventTarget implementation.
  void StartEvent(const std::string& type) override;
  void StopEvent(const std::string& type) override;

 private:
  enum FileFormat {
    FILE_OBJ,
    FILE_PLY,
    FILE_STL
  };

  enum ScanningMode {
    SCAN_VARIABLE,
    SCAN_OBJECT_ON_PLANAR_SURFACE_DETECTION,
    SCAN_FACE,
    SCAN_HEAD,
    SCAN_BODY
  };

  enum ReconstructionOption {
    SOLIDIFICATION,
    TEXTURE
  };

  void OnStart(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnStop(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnReset(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnSetConfiguration(scoped_ptr<XWalkExtensionFunctionInfo> info); // ?
  
  // Run on scenemanager_thread_;
  void OnCreateAndStartPipeline(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnStopAndDestroyPipeline(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnResetScan3DObject(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnRunPipeline();
  
  base::Thread scenemanager_thread_;
  PXCSenseManager* sense_manager_;
  PXC3DScan* scanner_;
  FileFormat file_format_;
  ScanningMode scanning_mode_;
  ReconstructionOption reconstruct_option_;
  int scanning_frames_;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_REALSENSE_SCAN3D_OBJECT_H_
