// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var internal = requireNative('internal');
internal.setupInternalExtension(extension);

var v8tools = requireNative('v8tools');
var common = requireNative('sysapps_common');
common.setupSysAppsCommon(internal, v8tools);

var Promise = requireNative('sysapps_promise').Promise;

var RealSense = function() {
  common.BindingObject.call(this, common.getUniqueId());
  common.EventTarget.call(this);

  internal.postMessage("realsenseConstructor", [this._id]);

  this._addMethodWithPromise("getVersion", Promise);
};

RealSense.prototype = new common.EventTargetPrototype();
RealSense.prototype.constructor = RealSense;

var ScenePerception = function() {
  common.BindingObject.call(this, common.getUniqueId());
  common.EventTarget.call(this);

  internal.postMessage("sceneperceptionConstructor", [this._id]);
  
  this._addMethodWithPromise("start", Promise);
  this._addMethodWithPromise("stop", Promise);
  this._addMethodWithPromise("reset", Promise);
  
  this._addEvent("error");
  this._addEvent("tracking");
};

ScenePerception.prototype = new common.EventTargetPrototype();
ScenePerception.prototype.constructor = ScenePerception;

exports = new RealSense();
exports.ScenePerception = ScenePerception;