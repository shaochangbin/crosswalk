// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/time/time.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/application_process_manager.h"
#include "xwalk/application/common/application.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/test/application_browsertest.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_registry.h"
#include "xwalk/runtime/browser/ui/native_app_window.h"
#include "xwalk/runtime/common/xwalk_switches.h"

namespace xwalk {
class NativeAppWindow;
}

using xwalk::application::Application;
using xwalk::application::ApplicationSystem;

class ApplicationBusyMainDocumentBrowserTest: public ApplicationBrowserTest {
 public:
  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE;
  void CheckMainDocumentState(ApplicationSystem* system,
                              bool suspending,
                              const base::Closure& runner_quit_task);
};

void ApplicationBusyMainDocumentBrowserTest::SetUpCommandLine(
    CommandLine* command_line) {
  ApplicationBrowserTest::SetUpCommandLine(command_line);
  GURL url = net::FilePathToFileURL(test_data_dir_.Append(
        FILE_PATH_LITERAL("busy_main_document")));
  command_line->AppendArg(url.spec());
}

// Verifies main document state when it is busying executing script.
IN_PROC_BROWSER_TEST_F(ApplicationBusyMainDocumentBrowserTest,
                       CloseBusyMainDocument) {
  content::RunAllPendingInMessageLoop();
  const xwalk::RuntimeList& runtimes =
      xwalk::RuntimeRegistry::Get()->runtimes();

  xwalk::RuntimeContext* runtime_context = runtimes[0]->runtime_context();
  WaitForRuntimes(2);

  xwalk::NativeAppWindow* win1 = runtimes[1]->window();
  ASSERT_TRUE(win1);
  win1->Close();

  scoped_refptr<content::MessageLoopRunner> runner =
      new content::MessageLoopRunner;

  base::MessageLoop::current()->PostDelayedTask(
    FROM_HERE,
    base::Bind(&ApplicationBusyMainDocumentBrowserTest::CheckMainDocumentState,
               base::Unretained(this),
               runtime_context->GetApplicationSystem(),
               false,
               runner->QuitClosure()),
    base::TimeDelta::FromMilliseconds(2200));
  runner->Run();

  scoped_refptr<content::MessageLoopRunner> runner2 =
      new content::MessageLoopRunner;
  base::MessageLoop::current()->PostDelayedTask(
    FROM_HERE,
    base::Bind(&ApplicationBusyMainDocumentBrowserTest::CheckMainDocumentState,
               base::Unretained(this),
               runtime_context->GetApplicationSystem(),
               true,
               runner2->QuitClosure()),
    base::TimeDelta::FromMilliseconds(2000));
  runner2->Run();
}

void ApplicationBusyMainDocumentBrowserTest::CheckMainDocumentState(
    ApplicationSystem* system,
    bool suspending,
    const base::Closure& runner_quit_task) {
  runner_quit_task.Run();
  ASSERT_EQ(system->process_manager()->IsMainDocumentSuspending(), suspending);
}

