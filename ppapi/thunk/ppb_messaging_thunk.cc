// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// From ppb_messaging.idl modified Tue Aug 20 08:13:36 2013.

#include "ppapi/c/pp_errors.h"
#include "ppapi/c/ppb_messaging.h"
#include "ppapi/shared_impl/tracked_callback.h"
#include "ppapi/thunk/enter.h"
#include "ppapi/thunk/ppapi_thunk_export.h"

namespace ppapi {
namespace thunk {

namespace {

void PostMessage(PP_Instance instance, struct PP_Var message) {
  VLOG(4) << "PPB_Messaging::PostMessage()";
  EnterInstance enter(instance);
  if (enter.failed())
    return;
  enter.functions()->PostMessage(instance, message);
}

const PPB_Messaging_1_0 g_ppb_messaging_thunk_1_0 = {
  &PostMessage
};

}  // namespace

PPAPI_THUNK_EXPORT const PPB_Messaging_1_0* GetPPB_Messaging_1_0_Thunk() {
  return &g_ppb_messaging_thunk_1_0;
}

}  // namespace thunk
}  // namespace ppapi
