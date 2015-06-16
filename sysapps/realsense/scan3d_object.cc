// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/realsense/scan3d_object.h"

#include "base/bind.h"
#include "base/logging.h"
#include "xwalk/sysapps/realsense/scan3d.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"

#include <algorithm>
#include <sstream>
#include <string>

namespace xwalk {
namespace sysapps {

using namespace jsapi::scan3d; // NOLINT

const int kDefaultScanningFrames = 200;
const int kDefaultFramesBeforeScanStart = 100;

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

PXC3DScan::FileFormat ToPxc3DFileFormat(const std::string& str) {
  const struct {
    const std::string str;
    PXC3DScan::FileFormat format;
  } kFileFormats [] = {
    { "obj", PXC3DScan::OBJ },
    { "ply", PXC3DScan::PLY },
    { "stl", PXC3DScan::STL }
  };
  for (const auto& iter : kFileFormats) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == iter.str)
      return iter.format;
  }
  return PXC3DScan::OBJ;
}

PXC3DScan::ScanningMode ToPxc3DScanningMode(const std::string& str) {
  const struct {
    const std::string str;
    PXC3DScan::ScanningMode mode;
  } kModes [] = {
    { "variable", PXC3DScan::VARIABLE },
    { "object_on_planar_surface_detection",
      PXC3DScan::OBJECT_ON_PLANAR_SURFACE_DETECTION },
    { "face", PXC3DScan::FACE },
    { "head", PXC3DScan::HEAD },
    { "body", PXC3DScan::BODY },
  };
  for (const auto& iter : kModes) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == iter.str)
      return iter.mode;
  }
  return PXC3DScan::FACE;
}

PXC3DScan::ReconstructionOption 
ToPxc3DReconstructionOptions(const std::string& str) {
  const struct {
    const std::string str;
    PXC3DScan::ReconstructionOption option;
  } kOptions [] = {
    { "none", PXC3DScan::NONE },
    { "solidification", PXC3DScan::SOLIDIFICATION },
    { "texture", PXC3DScan::TEXTURE }
  };
  for (const auto& iter : kOptions) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == iter.str)
      return iter.option;
  }
  return PXC3DScan::NONE;
}

}  // namespace

Scan3DObject::Scan3DObject() :
    sense_manager_(PXCSenseManager::CreateInstance()),
    scenemanager_thread_("SceneManagerThread") {
  SetDefaultConfiguration();
 
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
  scanner_ = sense_manager_->Query3DScan();
  if (!scanner_) {
    LOG(ERROR) << "3D scanner is unavailable. "; 
    return;
  }

  // Configure the system according to the provided arguments.
  PXC3DScan::Configuration config = scanner_->QueryConfiguration();
  // Must set the parameters separately.
  config.mode = configuration_.config.mode;
  config.options = configuration_.config.options;
  config.minFramesBeforeScanStart = configuration_.config.minFramesBeforeScanStart;
  result = scanner_->SetConfiguration(config);
  if (result != PXC_STATUS_NO_ERROR) {
    LOG(ERROR) << "Set configuration failed: " << result; 
    return;
  }

  sense_manager_->QueryCaptureManager()->QueryDevice()
      ->SetColorAutoExposure(true);

  scenemanager_thread_.message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&Scan3DObject::OnRunPipeline,
                 base::Unretained(this)));
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
  while (configuration_.scan_frames) {
    if (sense_manager_->AcquireFrame(true) < PXC_STATUS_NO_ERROR)
      break;
    if (scanner_->IsScanning()) {
      configuration_.scan_frames--;
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

  SetDefaultConfiguration();

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
  SetDefaultConfiguration();
}

void Scan3DObject::OnSetConfiguration(scoped_ptr<XWalkExtensionFunctionInfo> info) {
  scoped_ptr<SetConfiguration::Params> 
        params(SetConfiguration::Params::Create(*info->arguments()));
  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    return;
  }
  LOG(INFO) << "set fileFormat:" << params->configuration.file_format;
  LOG(INFO) << "set scanningMode:" << params->configuration.scanning_mode;
  LOG(INFO) << "set reconstructionOption:" << params->configuration.reconstruction_options;
  LOG(INFO) << "set minFramesBeforeScanStart:" << params->configuration.min_frames_before_scan_start;

  configuration_.file_format = ToPxc3DFileFormat(params->configuration.file_format);
  configuration_.config.mode = ToPxc3DScanningMode(params->configuration.scanning_mode);
  configuration_.config.options = ToPxc3DReconstructionOptions(params->configuration.reconstruction_options);
  configuration_.config.minFramesBeforeScanStart = params->configuration.min_frames_before_scan_start;
}

void Scan3DObject::SetDefaultConfiguration() {
  configuration_.file_format = PXC3DScan::OBJ;
  configuration_.scan_frames = kDefaultScanningFrames;
  configuration_.config.mode = PXC3DScan::FACE;
  //configuration_.config.options = PXC3DScan::NONE | PXC3DScan::SOLIDIFICATION;
  configuration_.config.options = PXC3DScan::NONE;
  configuration_.config.minFramesBeforeScanStart = kDefaultFramesBeforeScanStart;
}

}  // namespace sysapps
}  // namespace xwalk
