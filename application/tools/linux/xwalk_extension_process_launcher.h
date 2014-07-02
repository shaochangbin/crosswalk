// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TOOLS_LINUX_XWALK_EXTENSION_PROCESS_LAUNCHER_H_
#define XWALK_APPLICATION_TOOLS_LINUX_XWALK_EXTENSION_PROCESS_LAUNCHER_H_

#include <string>

#include "base/threading/thread.h"

namespace base {
class AtExitManager;
}

namespace xwalk {
namespace extensions {
class XWalkExtensionProcess;
}
}

class XWalkExtensionProcessLauncher: public base::Thread {
 public:
  XWalkExtensionProcessLauncher();
  ~XWalkExtensionProcessLauncher();

  // Implement base::Thread.
  virtual void CleanUp() OVERRIDE;

  // Will be called in launcher's main thread.
  void Launch();

  bool is_started() const { return is_started_; }

  std::string channel_id() { return channel_id_; }
  int channel_fd() { return channel_fd_; }

 private:
  void StartExtensionProcess();

  bool is_started_;
  scoped_ptr<base::AtExitManager> exit_manager_;
  scoped_ptr<xwalk::extensions::XWalkExtensionProcess> extension_process_;
  std::string channel_id_;
  int channel_fd_;
};

#endif  // XWALK_APPLICATION_TOOLS_LINUX_XWALK_EXTENSION_PROCESS_LAUNCHER_H_
