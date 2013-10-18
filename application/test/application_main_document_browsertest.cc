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

class ApplicationMainDocumentBrowserTest: public ApplicationBrowserTest {
 public:
  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE;
  void CheckMainDocumentState(ApplicationSystem* system,
                              bool suspending,
                              const base::Closure& runner_quit_task);
  void CreateNewRuntime(xwalk::RuntimeContext* context,
                        const GURL& url,
                        const base::Closure& runner_quit_task);
};

void ApplicationMainDocumentBrowserTest::SetUpCommandLine(
    CommandLine* command_line) {
  ApplicationBrowserTest::SetUpCommandLine(command_line);
  GURL url = net::FilePathToFileURL(test_data_dir_.Append(
        FILE_PATH_LITERAL("main_document")));
  command_line->AppendArg(url.spec());
}

// Verifies the runtime creation when main document is used.
IN_PROC_BROWSER_TEST_F(ApplicationMainDocumentBrowserTest, MainDocument) {
  content::RunAllPendingInMessageLoop();
  // At least the main document's runtime exist after launch.
  ASSERT_GE(GetRuntimeNumber(), 1);

  xwalk::Runtime* main_runtime = xwalk::RuntimeRegistry::Get()->runtimes()[0];
  xwalk::RuntimeContext* runtime_context = main_runtime->runtime_context();
  xwalk::application::ApplicationService* service =
    runtime_context->GetApplicationSystem()->application_service();
  const Application* app = service->GetRunningApplication();
  GURL generated_url =
    app->GetResourceURL(xwalk::application::kGeneratedMainDocumentFilename);
  // Check main document URL.
  ASSERT_EQ(main_runtime->web_contents()->GetURL(), generated_url);
  ASSERT_TRUE(!main_runtime->window());

  // There should exist 2 runtimes(one for generated main document, one for the
  // window created by main document).
  WaitForRuntimes(2);
}

// Verifies proper shutdown of the main document.
IN_PROC_BROWSER_TEST_F(ApplicationMainDocumentBrowserTest, CloseMainDocument) {
  content::RunAllPendingInMessageLoop();
  const xwalk::RuntimeList& runtimes =
      xwalk::RuntimeRegistry::Get()->runtimes();

  xwalk::RuntimeContext* runtime_context = runtimes[0]->runtime_context();
  WaitForRuntimes(2);

  xwalk::NativeAppWindow* win1 = runtimes[1]->window();
  ASSERT_TRUE(win1);
  win1->Close();

  // Now only the main document exists, therefore, its state should
  // switch to Suspending after main_document_idle_time_.
  scoped_refptr<content::MessageLoopRunner> runner =
      new content::MessageLoopRunner;
  base::MessageLoop::current()->PostDelayedTask(
    FROM_HERE,
    base::Bind(&ApplicationMainDocumentBrowserTest::CheckMainDocumentState,
               base::Unretained(this),
               runtime_context->GetApplicationSystem(),
               true,
               runner->QuitClosure()),
    base::TimeDelta::FromMilliseconds(2200));
  runner->Run();
}

// Verifies cancel shutdown of the main document.
IN_PROC_BROWSER_TEST_F(ApplicationMainDocumentBrowserTest,
                       CancelCloseMainDocument) {
  content::RunAllPendingInMessageLoop();
  const xwalk::RuntimeList& runtimes =
      xwalk::RuntimeRegistry::Get()->runtimes();

  xwalk::RuntimeContext* runtime_context = runtimes[0]->runtime_context();
  WaitForRuntimes(2);

  xwalk::NativeAppWindow* win1 = runtimes[1]->window();
  ASSERT_TRUE(win1);
  win1->Close();

  // Create a new Runtime object when the main document is suspending.
  scoped_refptr<content::MessageLoopRunner> runner =
      new content::MessageLoopRunner;
  GURL url(test_server()->GetURL("test.html"));
  base::MessageLoop::current()->PostDelayedTask(
    FROM_HERE,
    base::Bind(&ApplicationMainDocumentBrowserTest::CreateNewRuntime,
               base::Unretained(this),
               runtime_context,
               url,
               runner->QuitClosure()),
    base::TimeDelta::FromMilliseconds(2200));
  runner->Run();

  scoped_refptr<content::MessageLoopRunner> runner2 =
      new content::MessageLoopRunner;
  base::MessageLoop::current()->PostDelayedTask(
    FROM_HERE,
    base::Bind(&ApplicationMainDocumentBrowserTest::CheckMainDocumentState,
               base::Unretained(this),
               runtime_context->GetApplicationSystem(),
               false,
               runner2->QuitClosure()),
    base::TimeDelta::FromMilliseconds(200));
  runner2->Run();
}

void ApplicationMainDocumentBrowserTest::CheckMainDocumentState(
    ApplicationSystem* system,
    bool suspending,
    const base::Closure& runner_quit_task) {
  runner_quit_task.Run();
  ASSERT_EQ(system->process_manager()->IsMainDocumentSuspending(), suspending);
}

void ApplicationMainDocumentBrowserTest::CreateNewRuntime(
    xwalk::RuntimeContext* runtime_context,
    const GURL& url,
    const base::Closure& runner_quit_task) {
  xwalk::Runtime* new_runtime = xwalk::Runtime::CreateWithDefaultWindow(
      runtime_context, url);
  runner_quit_task.Run();
}
