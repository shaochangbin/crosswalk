// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_platform_util.h"

#include "base/logging.h"
#include "base/process/kill.h"
#include "base/process/launch.h"
#include "url/gurl.h"

namespace platform_util {

void OpenExternal(const GURL& url) {
  if (url.SchemeIsHTTPOrHTTPS()) {
    LOG(INFO) << "Open in MiniBrowser.";
    std::vector<std::string> argv;
    argv.push_back("MiniBrowser");
    argv.push_back(url.spec());
    base::ProcessHandle handle;

    if (base::LaunchProcess(argv, base::LaunchOptions(), &handle))
      base::EnsureProcessGetsReaped(handle);
  }
}

}  // namespace platform_util
