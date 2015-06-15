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

Scan3DObject::Scan3DObject() :
    scenemanager_thread_("SceneManagerThread") {
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

  LOG(INFO) << "-----" << __FUNCTION__;
  const base::FilePath path(FILE_PATH_LITERAL("filename.txt"));
  FILE* file = OpenFile(path, "wb");
  if (!file) {
    LOG(ERROR) << "Couldn't open '" << path.AsUTF8Unsafe()
                << "' for writing";
  }

 scoped_ptr<base::ListValue> error(new base::ListValue());
  error->AppendString("noerror");
  info->PostResult(error.Pass());
}

void Scan3DObject::OnRunPipeline() {
  DCHECK_EQ(scenemanager_thread_.message_loop(), base::MessageLoop::current());
 
  scenemanager_thread_.message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&Scan3DObject::OnRunPipeline,
                 base::Unretained(this)));
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
