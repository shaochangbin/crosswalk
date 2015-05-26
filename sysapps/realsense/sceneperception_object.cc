// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/realsense/sceneperception_object.h"

#include "xwalk/sysapps/realsense/sceneperception.h"
#include "xwalk/sysapps/realsense/sp_controller.h"

#include "pxcsession.h" // NOLINT

#include <string>
#include <sstream>

namespace xwalk {
namespace sysapps {

using namespace jsapi::sceneperception; // NOLINT

ScenePerceptionObject::ScenePerceptionObject() :
    started_(false) {
  color_image_width_ = depth_image_width_ = 320;
  color_image_height_ = depth_image_height_ = 240;
  color_capture_framerate_ = depth_capture_framerate_ = 60.0;
  handler_.Register("start",
                    base::Bind(&ScenePerceptionObject::OnStart,
                               base::Unretained(this)));
  handler_.Register("stop",
                    base::Bind(&ScenePerceptionObject::OnStop,
                               base::Unretained(this)));
  handler_.Register("reset",
                    base::Bind(&ScenePerceptionObject::OnReset,
                               base::Unretained(this)));
}

ScenePerceptionObject::~ScenePerceptionObject() {
}

void ScenePerceptionObject::StartEvent(const std::string& type) {
  NOTIMPLEMENTED();
}

void ScenePerceptionObject::StopEvent(const std::string& type) {
  NOTIMPLEMENTED();
}

void ScenePerceptionObject::OnStart(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
      
  if (started_) {
    scoped_ptr<base::ListValue> error(new base::ListValue());
	  error->AppendString("already started");
  	info->PostResult(error.Pass());
	  return;
  }
  
  sceneperception_controller_.reset(
      new ScenePerceptionController(color_image_width_, color_image_height_, color_capture_framerate_, 
                                    depth_image_width_, depth_image_height_, depth_capture_framerate_));
  if(sceneperception_controller_->QueryScenePerception() == NULL) {
	  scoped_ptr<base::ListValue> error(new base::ListValue());
	  error->AppendString("failed to create scene perception");
  	info->PostResult(error.Pass());
    sceneperception_controller_.reset();
	  return;
  }
  
  sceneperception_controller_->PauseScenePerception(false);
  sceneperception_controller_->EnableReconstruction(true);
	
  if(!sceneperception_controller_->InitPipeline(this)) {
	  scoped_ptr<base::ListValue> error(new base::ListValue());
	  error->AppendString("failed to init pipeline");
  	info->PostResult(error.Pass());
    sceneperception_controller_.reset();
	  return;
  }
	
  sceneperception_controller_->QueryCaptureSize(color_image_width_, color_image_height_, depth_image_width_, depth_image_height_);

  float fx = 0.0f, fy = 0.0f, u0 = 0.0f, v0 = 0.0f; // intrinsic parameters	
  if(!sceneperception_controller_->GetCameraParameters(fx, fy, u0, v0)){
	  scoped_ptr<base::ListValue> error(new base::ListValue());
	  error->AppendString("failed to query camera");
  	info->PostResult(error.Pass());
    sceneperception_controller_.reset();
	  return;
  }
  
  if (!sceneperception_controller_->StreamFrames(false)) {
    scoped_ptr<base::ListValue> error(new base::ListValue());
	  error->AppendString("failed to stream frames");
  	info->PostResult(error.Pass());
    sceneperception_controller_.reset();
	  return;
  }
  
  started_ = true;

  scoped_ptr<base::ListValue> error(new base::ListValue());
  error->AppendString("noerror");
  info->PostResult(error.Pass());
}

void ScenePerceptionObject::OnStop(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  if (!started_) {
    scoped_ptr<base::ListValue> error(new base::ListValue());
    error->AppendString("not started");
    info->PostResult(error.Pass()); 
    return; 
  }
  started_ = false;
  sceneperception_controller_.reset();
  scoped_ptr<base::ListValue> error(new base::ListValue());
  error->AppendString("noerror");
  info->PostResult(error.Pass());
}

void ScenePerceptionObject::OnReset(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {

  sceneperception_controller_->ResetScenePerception();

}

pxcStatus ScenePerceptionObject::OnModuleProcessedFrame(
    pxcUID mid, PXCBase* module, PXCCapture::Sample* sample) {
  if (mid == PXCScenePerception::CUID) {
    if (!started_)
      return PXC_STATUS_NO_ERROR;

    PXCScenePerception *sp = module->QueryInstance<PXCScenePerception>();
    
    TrackingEvent event;
    float imageQuality = sp->CheckSceneQuality(sample);
    event.quality = imageQuality;
    
    if (imageQuality >= 0.5) {
      PXCScenePerception::TrackingAccuracy accuracy =
          sp->QueryTrackingAccuracy();
      event.accuracy = ACCURACY_NONE;
      switch(accuracy) {
        case PXCScenePerception::HIGH:
          event.accuracy = ACCURACY_HIGH;
          break;
        case PXCScenePerception::MED:
          event.accuracy = ACCURACY_MED;
          break;
        case PXCScenePerception::LOW:
          event.accuracy = ACCURACY_LOW;
          break;
        case PXCScenePerception::FAILED:
          event.accuracy = ACCURACY_FAILED;
          break;
      }
      
      float pose[12];
      sp->GetCameraPose(pose);
      for (int i = 0; i < 12; ++i) {
        event.pose.push_back(pose[i]);
      }
    }
      
    scoped_ptr<base::ListValue> eventData(new base::ListValue);
    eventData->Append(event.ToValue().release());
    
    DispatchEvent("tracking", eventData.Pass());
  }

  return PXC_STATUS_NO_ERROR;
}

void ScenePerceptionObject::OnStatus(pxcUID mid, pxcStatus status) {
  if (mid == PXCScenePerception::CUID && status < PXC_STATUS_NO_ERROR) {
    ErrorEvent event;
    std::ostringstream status_str;
    status_str << status;
    event.status = status_str.str();

    scoped_ptr<base::ListValue> eventData(new base::ListValue);
    eventData->Append(event.ToValue().release());

    DispatchEvent("error", eventData.Pass());
    
    sceneperception_controller_.reset();
    started_ = false;

    return;
  }
  return;
}

}  // namespace sysapps
}  // namespace xwalk
