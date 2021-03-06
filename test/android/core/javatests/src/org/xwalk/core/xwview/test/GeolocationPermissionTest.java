// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.graphics.Bitmap;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;

import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkClient;
import org.xwalk.core.XWalkGeolocationPermissions;
import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkWebChromeClient;

/**
 * Test suite for onGeolocationPermissionsShowPrompt() and
 *                onGeolocationPermissionsHidePrompt().
 */
public class GeolocationPermissionTest extends XWalkViewTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();

        setXWalkClient(new XWalkViewTestBase.TestXWalkClient());
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().getSettings().setJavaScriptEnabled(true);
                getXWalkView().getSettings().setGeolocationEnabled(true);
            }
        });
    }

    @SmallTest
    @Feature({"GeolocationPermission"})
    public void testGeolocationPermissionShowPrompt() throws Throwable {
        class TestWebChromeClient extends XWalkWebChromeClient {
            public TestWebChromeClient() {
                super(getXWalkView().getContext(), getXWalkView());
            }

            private int mCalledCount = 0;
            @Override
            public void onGeolocationPermissionsShowPrompt(String origin,
                    XWalkGeolocationPermissions.Callback callback) {
                // The origin is empty for data stream.
                assertTrue(origin.isEmpty());
                callback.invoke(origin, true, true);
                mCalledCount++;
            }

            public int getCalledCount() {
                return mCalledCount;
            }
        }
        final TestWebChromeClient testWebChromeClient = new TestWebChromeClient();
        setXWalkWebChromeClient(testWebChromeClient);
        loadAssetFile("geolocation.html");
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                assertEquals(1, testWebChromeClient.getCalledCount());
            }
        });
    }

    // This is not used now. Need a TODO to follow up this.
    // TODO(hengzhi): how to verify it automaticly.
    // @SmallTest
    // @Feature({"GeolocationPermission"})
    @DisabledTest
    public void testGeolocationPermissionHidePrompt() throws Throwable {
        class TestWebChromeClient extends XWalkWebChromeClient {
            public TestWebChromeClient() {
                super(getXWalkView().getContext(), getXWalkView());
            }

            @Override
            public void onGeolocationPermissionsHidePrompt() {
                // Do something.
            }
        }
        setXWalkWebChromeClient(new TestWebChromeClient());
        loadUrlSync("http://html5demos.com/geo");
    }
}
