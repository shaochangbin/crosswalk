// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/realsense/sceneperception_object.h"

#include "base/logging.h"
#include "xwalk/sysapps/realsense/sceneperception.h"
#include "xwalk/sysapps/realsense/sp_controller.h"

#include <string>
#include <sstream>

namespace xwalk {
namespace sysapps {

using namespace jsapi::sceneperception; // NOLINT

ScenePerceptionObject::ScenePerceptionObject() :
    state_(IDLE),
    on_checking_(false),
    on_tracking_(false),
    on_meshing_(false),
    block_meshing_data_(NULL) {
  // TODO(nhu): expose these configrations to JS
  color_image_width_ = depth_image_width_ = 320;
  color_image_height_ = depth_image_height_ = 240;
  color_capture_framerate_ = depth_capture_framerate_ = 60.0;
  meshing_update_info_.blockMeshesRequired = meshing_update_info_.countOfBlockMeshesRequired  = meshing_update_info_.blockMeshesRequired = 
	meshing_update_info_.countOfVeticesRequired = meshing_update_info_.verticesRequired = meshing_update_info_.countOfFacesRequired =
	meshing_update_info_.facesRequired = meshing_update_info_.colorsRequired = 1;

  handler_.Register("start",
                    base::Bind(&ScenePerceptionObject::OnStart,
                               base::Unretained(this)));
  handler_.Register("stop",
                    base::Bind(&ScenePerceptionObject::OnStop,
                               base::Unretained(this)));
  handler_.Register("reset",
                    base::Bind(&ScenePerceptionObject::OnReset,
                               base::Unretained(this)));
  handler_.Register("enableTracking",
                    base::Bind(&ScenePerceptionObject::OnEnableTracking,
                               base::Unretained(this)));
  handler_.Register("disableTracking",
                    base::Bind(&ScenePerceptionObject::OnDisableTracking,
                               base::Unretained(this)));
  handler_.Register("enableMeshing",
                    base::Bind(&ScenePerceptionObject::OnEnableMeshing,
                               base::Unretained(this)));
  handler_.Register("disableMeshing",
                    base::Bind(&ScenePerceptionObject::OnDisableMeshing,
                               base::Unretained(this)));

}

ScenePerceptionObject::~ScenePerceptionObject() {
}

void ScenePerceptionObject::StartEvent(const std::string& type) {
  if (type == std::string("checking")) {
    on_checking_ = true;
  } else if (type == std::string("tracking")) {
    on_tracking_ = true;
  } else if (type == std::string("meshing")) {
    on_meshing_ = true;
  }
}

void ScenePerceptionObject::StopEvent(const std::string& type) {
  if (type == std::string("checking")) {
    on_checking_ = false;
  } else if (type == std::string("tracking")) {
    on_tracking_ = false;
  } else if (type == std::string("meshing")) {
    on_meshing_ = false;
  }
}

void ScenePerceptionObject::OnStart(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
      
  if (state_ != IDLE) {
    scoped_ptr<base::ListValue> error(new base::ListValue());
	  error->AppendString("state is not IDLE");
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
  
  sceneperception_controller_->PauseScenePerception(true);
  sceneperception_controller_->EnableReconstruction(false);
  
  block_meshing_data_ = sceneperception_controller_->CreatePXCBlockMeshingData();
	
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
  
  state_ = CHECKING;

  scoped_ptr<base::ListValue> error(new base::ListValue());
  error->AppendString("noerror");
  info->PostResult(error.Pass());
}

void ScenePerceptionObject::OnStop(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  if (state_ == IDLE) {
    scoped_ptr<base::ListValue> error(new base::ListValue());
    error->AppendString("state is IDLE");
    info->PostResult(error.Pass()); 
    return; 
  }
  state_ = IDLE;
  sceneperception_controller_.reset();
  if(block_meshing_data_)
		block_meshing_data_->Release();
  scoped_ptr<base::ListValue> error(new base::ListValue());
  error->AppendString("noerror");
  info->PostResult(error.Pass());
}

void ScenePerceptionObject::OnReset(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  if (state_ == IDLE) {
    scoped_ptr<base::ListValue> error(new base::ListValue());
    error->AppendString("state is IDLE");
    info->PostResult(error.Pass()); 
    return; 
  }
  sceneperception_controller_->ResetScenePerception();
  block_meshing_data_->Reset();
}

void ScenePerceptionObject::OnEnableTracking(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  if (state_ != CHECKING) {
    scoped_ptr<base::ListValue> error(new base::ListValue());
    error->AppendString("state is not CHECKING");
    info->PostResult(error.Pass()); 
    return; 
  }
  state_ = TRACKING;
  sceneperception_controller_->PauseScenePerception(false);
  scoped_ptr<base::ListValue> error(new base::ListValue());
  error->AppendString("noerror");
  info->PostResult(error.Pass());
}

void ScenePerceptionObject::OnDisableTracking(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  if (state_ != TRACKING) {
    scoped_ptr<base::ListValue> error(new base::ListValue());
    error->AppendString("state is not TRACKING");
    info->PostResult(error.Pass()); 
    return; 
  }
  state_ = CHECKING;
  sceneperception_controller_->PauseScenePerception(true);
  scoped_ptr<base::ListValue> error(new base::ListValue());
  error->AppendString("noerror");
  info->PostResult(error.Pass());
}

void ScenePerceptionObject::OnEnableMeshing(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  if (state_ != TRACKING) {
    scoped_ptr<base::ListValue> error(new base::ListValue());
    error->AppendString("state is not TRACKING");
    info->PostResult(error.Pass()); 
    return; 
  }
  state_ = MESHING;
  sceneperception_controller_->EnableReconstruction(true);
  scoped_ptr<base::ListValue> error(new base::ListValue());
  error->AppendString("noerror");
  info->PostResult(error.Pass());
}

void ScenePerceptionObject::OnDisableMeshing(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  if (state_ != MESHING) {
    scoped_ptr<base::ListValue> error(new base::ListValue());
    error->AppendString("state is not MESHING");
    info->PostResult(error.Pass()); 
    return; 
  }
  state_ = TRACKING;
  sceneperception_controller_->EnableReconstruction(false);
  scoped_ptr<base::ListValue> error(new base::ListValue());
  error->AppendString("noerror");
  info->PostResult(error.Pass());
}

pxcStatus ScenePerceptionObject::OnNewSample(
    pxcUID mid, PXCCapture::Sample* sample) {
  if (state_ == CHECKING) {
    if (on_checking_) {
      CheckingEvent event;
      float imageQuality = sceneperception_controller_->QueryScenePerception()->CheckSceneQuality(sample);
      event.quality = imageQuality;
      scoped_ptr<base::ListValue> eventData(new base::ListValue);
      eventData->Append(event.ToValue().release());
      
      DispatchEvent("checking", eventData.Pass());
    }
  }
  return PXC_STATUS_NO_ERROR;
}

pxcStatus ScenePerceptionObject::OnModuleProcessedFrame(
    pxcUID mid, PXCBase* module, PXCCapture::Sample* sample) {
  if (mid == PXCScenePerception::CUID) {
    PXCScenePerception *sp = module->QueryInstance<PXCScenePerception>();
    
    if (state_ == TRACKING || state_ == MESHING) {
      if (on_tracking_) {
        TrackingEvent event;
    
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
          event.camera_pose.push_back(pose[i]);
        }
        
        scoped_ptr<base::ListValue> eventData(new base::ListValue);
        eventData->Append(event.ToValue().release());
        
        DispatchEvent("tracking", eventData.Pass());
      }
    }
    
    if (state_ == MESHING) { 
      if (on_meshing_) {
        // Update meshes
        if(sp->IsReconstructionUpdated()) {
          pxcStatus status = sp->DoMeshingUpdate(block_meshing_data_, true, &meshing_update_info_);
          if (status == PXC_STATUS_NO_ERROR) {
            MeshingEvent event;
            linked_ptr<Mesh> mesh(new Mesh);
        
        		float *pVertices = block_meshing_data_->QueryVertices();
            int iNumVertices = block_meshing_data_->QueryNumberOfVertices();
            for (int k = 0; k < iNumVertices; ++k) {
              mesh->vertices.push_back(*pVertices);
              ++pVertices;
              mesh->vertices.push_back(*pVertices);
              ++pVertices;
              mesh->vertices.push_back(*pVertices);
              ++pVertices;
              ++pVertices; // skip confidence
            }
            
            unsigned char *pColors = block_meshing_data_->QueryVerticesColor();
            for (int k = 0; k < iNumVertices; ++k) {
              mesh->colors.push_back(*pColors);
              ++pColors;
              mesh->colors.push_back(*pColors);
              ++pColors;
              mesh->colors.push_back(*pColors);
              ++pColors;
            }
        
            int *pFaces = block_meshing_data_->QueryFaces();
            int iNumFaces = block_meshing_data_->QueryNumberOfFaces();    
            for (int l = 0; l < iNumFaces; ++l) {
              mesh->faces.push_back(*pFaces);
              ++pFaces;
              mesh->faces.push_back(*pFaces);
              ++pFaces;
              mesh->faces.push_back(*pFaces);
              ++pFaces;
            }
  
            event.meshes.push_back(mesh);
            
            scoped_ptr<base::ListValue> eventData(new base::ListValue);
            eventData->Append(event.ToValue().release());
            
            DispatchEvent("meshing", eventData.Pass());
          }
        }
      }
    }
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
    state_ = IDLE;

    return;
  }
  return;
}

}  // namespace sysapps
}  // namespace xwalk
