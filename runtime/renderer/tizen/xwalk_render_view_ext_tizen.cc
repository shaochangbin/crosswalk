// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/tizen/xwalk_render_view_ext_tizen.h"

#include <string>

#include "base/bind.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_view.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "xwalk/runtime/common/xwalk_common_messages.h"

namespace xwalk {

XWalkRenderViewExtTizen::XWalkRenderViewExtTizen(content::RenderView* render_view)
    : content::RenderViewObserver(render_view) {
  LOG(INFO) << "+++" << __FUNCTION__ << "+++";
  render_view_ = render_view;
  DCHECK(render_view_);
}

XWalkRenderViewExtTizen::~XWalkRenderViewExtTizen() {
}

// static
void XWalkRenderViewExtTizen::RenderViewCreated(content::RenderView* render_view) {
  new XWalkRenderViewExtTizen(render_view);  // |render_view| takes ownership.
}

bool XWalkRenderViewExtTizen::OnMessageReceived(const IPC::Message& message) {
  LOG(INFO) << "+++" << __FUNCTION__ << "+++";
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkRenderViewExtTizen, message)
    IPC_MESSAGE_HANDLER(XWalkViewMsg_SuspendScheduledTasks,
                        OnSuspendScheduledTasks)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkRenderViewExtTizen::OnSuspendScheduledTasks() {
  LOG(INFO) << "+++" << __FUNCTION__ << "+++";
  content::RenderFrame* render_frame = render_view_->GetMainRenderFrame();
  blink::WebFrame* web_frame = render_frame->GetWebFrame();
  web_frame->document().suspendScheduledTasks();
}

}  // namespace xwalk
