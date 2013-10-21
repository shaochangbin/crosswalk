// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_PROCESS_MANAGER_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_PROCESS_MANAGER_H_

#include <map>
#include <set>
#include <string>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "xwalk/application/common/application.h"
#include "xwalk/runtime/browser/runtime_registry.h"

class GURL;

namespace xwalk {
class Runtime;
class RuntimeContext;
}

namespace xwalk {
namespace application {

class ApplicationHost;
class Manifest;

// This manages dynamic state of running applications. By now, it only launches
// one application, later it will manages all event pages' lifecycle.
class ApplicationProcessManager : public RuntimeRegistryObserver {
 public:
  explicit ApplicationProcessManager(xwalk::RuntimeContext* runtime_context);
  ~ApplicationProcessManager();

  bool LaunchApplication(xwalk::RuntimeContext* runtime_context,
                         const Application* application);

  Runtime* GetMainDocumentRuntime() const { return main_runtime_; }

  // RuntimeRegistryObserver implementation.
  virtual void OnRuntimeAdded(Runtime* runtime) OVERRIDE;
  virtual void OnRuntimeRemoved(Runtime* runtime) OVERRIDE;
  virtual void OnRuntimeAppIconChanged(Runtime* runtime) OVERRIDE {}

  // Handles a response to the ShouldSuspend message.
  void OnShouldSuspendAck(int sequence_id);
  // Same as above, for the Suspend message.
  void OnSuspendAck();
  void CancelSuspend();
  bool IsMainDocumentSuspending() const {
    return main_document_data_.is_closing;
  }

 private:
  bool RunMainDocument(const Application* application);
  bool RunFromLocalPath(const Application* application);
  void CloseMainDocument(int sequence_id);
  void OnMainDocumentActive();
  void OnMainDocumentIdle(int sequence_id);

  struct MainDocumentData {
    // This is used with the ShouldSuspend message, to ensure that the main
    // document remained idle between sending the message and receiving the ack.
    int close_sequence_id;

    // True if the main document responded to the ShouldSuspend message and is
    // about to close.
    bool is_closing;
  };

  xwalk::RuntimeContext* runtime_context_;
  xwalk::Runtime* main_runtime_;
  base::WeakPtrFactory<ApplicationProcessManager> weak_ptr_factory_;

  std::set<Runtime*> runtimes_;
  MainDocumentData main_document_data_;
  base::TimeDelta main_document_idle_time_;
  base::TimeDelta main_document_suspending_time_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationProcessManager);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_PROCESS_MANAGER_H_
