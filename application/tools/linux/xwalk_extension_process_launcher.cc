// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/at_exit.h"
#include "base/message_loop/message_loop.h"
#include "xwalk/application/tools/linux/xwalk_extension_process_launcher.h"
#include "xwalk/extensions/extension_process/xwalk_extension_process.h"

XWalkExtensionProcessLauncher::XWalkExtensionProcessLauncher()
    : base::Thread("LauncherExtensionService"),
      is_started_(false) {
  exit_manager_.reset(new base::AtExitManager);
  base::Thread::Options thread_options;
  thread_options.message_loop_type = base::MessageLoop::TYPE_DEFAULT;
  StartWithOptions(thread_options);
}

XWalkExtensionProcessLauncher::~XWalkExtensionProcessLauncher() {
  Stop();
}

void XWalkExtensionProcessLauncher::CleanUp() {
  extension_process_.reset();
}

void XWalkExtensionProcessLauncher::Launch() {
  is_started_ = true;
  message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&XWalkExtensionProcessLauncher::StartExtensionProcess,
                 base::Unretained(this)));
}

void XWalkExtensionProcessLauncher::StartExtensionProcess() {
  /*
  extension_process_.reset(new xwalk::extensions::XWalkExtensionProcess(
      IPC::ChannelHandle(channel_id, base::FileDescriptor(channel_fd, true))));
  */
#if defined(OS_LINUX)
    channel_id_ =
        IPC::Channel::GenerateVerifiedChannelID(std::string());
    IPC::Channel* channel = new IPC::Channel(
          channel_id_, IPC::Channel::MODE_CLIENT, NULL); // ?
    if (!channel->Connect())
      NOTREACHED();
    IPC::ChannelHandle channel_handle(channel_id_,
        base::FileDescriptor(channel->TakeClientFileDescriptor(), true));
    channel_fd_ = channel_handle.socket.fd; // ?
    extension_process_.reset(new xwalk::extensions::XWalkExtensionProcess(
      IPC::ChannelHandle(channel_id_, base::FileDescriptor(channel_fd_, true))));
#else
    NOTIMPLEMENTED();
#endif

}
