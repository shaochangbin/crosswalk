// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/realsense/scan3d_object.h"

#include "base/bind.h"
#include "base/logging.h"
#include "xwalk/sysapps/realsense/scan3d.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"

#include <string>
#include <sstream>

namespace xwalk {
namespace sysapps {

using namespace jsapi::scan3d; // NOLINT

namespace {

void PrintReconstructMessage(pxcStatus result) {
  if (result >= PXC_STATUS_NO_ERROR) {
    LOG(INFO) << "Mesh file is saved successfully.";
  } else if (result == PXC_STATUS_FILE_WRITE_FAILED) {
    LOG(ERROR) << "The file could not be created using the provided path. Aborting.";
  } else if (result == PXC_STATUS_ITEM_UNAVAILABLE || 
             result == PXC_STATUS_DATA_UNAVAILABLE) {
    LOG(ERROR) << "No scan data found. Aborting.";
  } else if (result < PXC_STATUS_NO_ERROR) {
    LOG(ERROR) << "Reconstruct Error:" << result;
  }
}

}  // namespace

Scan3DObject::Scan3DObject() :
    sense_manager_(PXCSenseManager::CreateInstance()),
    scanning_frames_(100),
    scenemanager_thread_("SceneManagerThread") {
  // TODO(changbin): expose these configurations to JS.
  file_format_ = FILE_OBJ;
  scanning_mode_ = SCAN_FACE;
  reconstruct_option_ = SOLIDIFICATION;

  handler_.Register("start",
                    base::Bind(&Scan3DObject::OnStart,
                               base::Unretained(this)));
  handler_.Register("stop",
                    base::Bind(&Scan3DObject::OnStop,
                               base::Unretained(this)));
  handler_.Register("reset",
                    base::Bind(&Scan3DObject::OnReset,
                               base::Unretained(this)));
  handler_.Register("setConfiguration",
                    base::Bind(&Scan3DObject::OnSetConfiguration,
                               base::Unretained(this)));
}

Scan3DObject::~Scan3DObject() {
  sense_manager_->Release();
}

void Scan3DObject::StartEvent(const std::string& type) {
}

void Scan3DObject::StopEvent(const std::string& type) {
}

void Scan3DObject::OnStart(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  LOG(INFO) << "-----" << __FUNCTION__;
  if (scenemanager_thread_.IsRunning()) {
    scoped_ptr<base::ListValue> error(new base::ListValue());
    error->AppendString("scenemanager thread is running");
    info->PostResult(error.Pass()); 
    return;  // Wrong state.
  }
  scenemanager_thread_.Start();
  
  scenemanager_thread_.message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&Scan3DObject::OnCreateAndStartPipeline,
                 base::Unretained(this),
                 base::Passed(&info)));
}

void Scan3DObject::OnCreateAndStartPipeline(scoped_ptr<XWalkExtensionFunctionInfo> info) {
  DCHECK_EQ(scenemanager_thread_.message_loop(), base::MessageLoop::current());
  pxcStatus result = PXC_STATUS_NO_ERROR;

  // Enable the 3D Scan video module.
  result = sense_manager_->Enable3DScan();
  if (result < PXC_STATUS_NO_ERROR) {
    LOG(ERROR) << "Enable 3DScan failed: " << result; 
    return;
  }
  // Initialize the streaming system.
  result = sense_manager_->Init();
  if (result < PXC_STATUS_NO_ERROR) {
    LOG(ERROR) << "Init reuturned: " << result << ", please check your camera."; 
    return;
  }
  // Why use query 2D scanner rather than 3D scanner?
  scanner_ = sense_manager_->Query3DScan();
  if (!scanner_) {
    LOG(ERROR) << "3D scanner is unavailable. "; 
    return;
  }

  // Configure the system according to the provided arguments.
  PXC3DScan::Configuration config = scanner_->QueryConfiguration();
  config.mode = PXC3DScan::FACE;
  config.options = config.options | PXC3DScan::SOLIDIFICATION;
  config.minFramesBeforeScanStart = 10;
  result = scanner_->SetConfiguration(config);
  if (result != PXC_STATUS_NO_ERROR) {
    LOG(ERROR) << "Set configuration failed: " << result; 
  }

  sense_manager_->QueryCaptureManager()->QueryDevice()
      ->SetColorAutoExposure(true);

  scenemanager_thread_.message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&Scan3DObject::OnRunPipeline,
                 base::Unretained(this)));

/*
  const base::FilePath path(FILE_PATH_LITERAL("filename.txt"));
  FILE* file = OpenFile(path, "wb");
  if (!file) {
    LOG(ERROR) << "Couldn't open '" << path.AsUTF8Unsafe()
                << "' for writing";
  }
*/
  scoped_ptr<base::ListValue> error(new base::ListValue());
  error->AppendString("noerror");
  info->PostResult(error.Pass());
}

void Scan3DObject::OnRunPipeline() {
  DCHECK_EQ(scenemanager_thread_.message_loop(), base::MessageLoop::current());
  if (!scanner_) {
    LOG(ERROR) << "3D scanner is unavailable. "; 
    return;
  }

  int first_scan_frame = 0;
  while (scanning_frames_) {
    if (sense_manager_->AcquireFrame(true) < PXC_STATUS_NO_ERROR)
      break;
    if (scanner_->IsScanning()) {
      scanning_frames_--;
      if (!first_scan_frame) {
        first_scan_frame++;
        sense_manager_->QueryCaptureManager()->QueryDevice()
            ->SetColorAutoExposure(false);
      }
    }
    PXCImage* image = scanner_->AcquirePreviewImage();
    sense_manager_->ReleaseFrame();
  }

  sense_manager_->QueryCaptureManager()->QueryDevice()
      ->SetColorAutoExposure(true);
  if (!scanner_->IsScanning())
    return;

  //pxcCHAR mesh_file_name[MAX_PATH];
  //swprintf_s(mesh_file_name, MAX_PATH, L"%s", "C:\\work\\3dscan.obj");
  //pxcStatus result = scanner_->Reconstruct(PXC3DScan::OBJ, mesh_file_name);

  pxcStatus result = scanner_->Reconstruct(PXC3DScan::OBJ, L"C:\\work\\3dscan.obj");
  PrintReconstructMessage(result);
/*
  scenemanager_thread_.message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&Scan3DObject::OnRunPipeline,
                 base::Unretained(this)));
*/
}

void Scan3DObject::OnStop(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  if (!scenemanager_thread_.IsRunning()) {
    scoped_ptr<base::ListValue> error(new base::ListValue());
    error->AppendString("scenemanager thread is not running");
    info->PostResult(error.Pass()); 
    return;  // Wrong state.
  }
  scenemanager_thread_.message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&Scan3DObject::OnStopAndDestroyPipeline,
                 base::Unretained(this),
                 base::Passed(&info)));
  scenemanager_thread_.Stop();
}

void Scan3DObject::OnStopAndDestroyPipeline(scoped_ptr<XWalkExtensionFunctionInfo> info) {
  DCHECK_EQ(scenemanager_thread_.message_loop(), base::MessageLoop::current());

  if (info.get()) {
    scoped_ptr<base::ListValue> error(new base::ListValue());
    error->AppendString("noerror");
    info->PostResult(error.Pass());
  }
}

void Scan3DObject::OnReset(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  scenemanager_thread_.message_loop()->PostTask(
    FROM_HERE,
    base::Bind(&Scan3DObject::OnResetScan3DObject,
               base::Unretained(this),
               base::Passed(&info)));
}

void Scan3DObject::OnResetScan3DObject(scoped_ptr<XWalkExtensionFunctionInfo> info) {
  DCHECK_EQ(scenemanager_thread_.message_loop(), base::MessageLoop::current());
  // TODO
}

void Scan3DObject::OnSetConfiguration(scoped_ptr<XWalkExtensionFunctionInfo> info) {
  // TODO
}

}  // namespace sysapps
}  // namespace xwalk
