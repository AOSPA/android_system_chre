/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.android.utils.chre;

import static com.google.common.truth.Truth.assertWithMessage;

import android.app.Instrumentation;
import android.content.Context;
import android.hardware.location.ContextHubInfo;
import android.hardware.location.ContextHubManager;
import android.hardware.location.ContextHubTransaction;
import android.hardware.location.NanoAppBinary;
import android.hardware.location.NanoAppState;
import android.os.ParcelFileDescriptor;

import androidx.test.InstrumentationRegistry;

import org.junit.Assert;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

/**
 * A set of helper functions for PTS CHRE tests.
 */
public class ChreTestUtil {
    // Various timeouts for Context Hub operations.
    private static final long TIMEOUT_LOAD_NANOAPP_SECONDS = 5;
    private static final long TIMEOUT_UNLOAD_NANOAPP_SECONDS = 5;
    private static final long QUERY_NANOAPPS_TIMEOUT_SECONDS = 5;

    /**
     * Read the nanoapp to an InputStream object.
     *
     * @param context  the Context to find the asset resources
     * @param fileName the fileName of the nanoapp
     * @return the InputStream of the nanoapp
     */
    public static InputStream getNanoAppInputStream(Context context, String fileName) {
        InputStream inputStream = null;
        try {
            inputStream = context.getAssets().open(fileName);
        } catch (IOException e) {
            Assert.fail("Could not find asset " + fileName + ": " + e.toString());
        }
        return inputStream;
    }

    /**
     * Creates a NanoAppBinary object from the nanoapp fileName.
     *
     * @param fileName the fileName of the nanoapp
     * @return the NanoAppBinary object
     */
    public static NanoAppBinary createNanoAppBinary(String fileName) {
        Context context = InstrumentationRegistry.getTargetContext();

        InputStream stream = getNanoAppInputStream(context, fileName);
        byte[] binary = null;
        try {
            binary = new byte[stream.available()];
            stream.read(binary);
        } catch (IOException e) {
            Assert.fail("IOException while reading binary for " + fileName + ": " + e.getMessage());
        }

        return new NanoAppBinary(binary);
    }

    /**
     * Loads a nanoapp.
     *
     * @param manager       The ContextHubManager to use to load the nanoapp.
     * @param info          The ContextHubInfo describing the Context Hub to load the nanoapp to.
     * @param nanoAppBinary The nanoapp binary to load.
     * @return true if the load succeeded.
     */
    public static boolean loadNanoApp(
            ContextHubManager manager, ContextHubInfo info, NanoAppBinary nanoAppBinary) {
        ContextHubTransaction<Void> txn = manager.loadNanoApp(info, nanoAppBinary);
        ContextHubTransaction.Response<Void> resp = null;
        try {
            resp = txn.waitForResponse(TIMEOUT_LOAD_NANOAPP_SECONDS, TimeUnit.SECONDS);
        } catch (TimeoutException | InterruptedException e) {
            Assert.fail(e.getMessage());
        }

        return resp != null && resp.getResult() == ContextHubTransaction.RESULT_SUCCESS;
    }

    /**
     * Same as loadNanoApp(), but asserts that it succeeds.
     */
    public static void loadNanoAppAssertSuccess(
            ContextHubManager manager, ContextHubInfo info, NanoAppBinary nanoAppBinary) {
        if (!loadNanoApp(manager, info, nanoAppBinary)) {
            Assert.fail("Failed to load nanoapp");
        }
    }

    /**
     * Unloads a nanoapp.
     *
     * @param manager   The ContextHubManager to use to unload the nanoapp.
     * @param info      The ContextHubInfo describing the Context Hub to unload the nanoapp from.
     * @param nanoAppId The 64-bit ID of the nanoapp to unload.
     * @return true if the unload succeeded.
     */
    public static boolean unloadNanoApp(
            ContextHubManager manager, ContextHubInfo info, long nanoAppId) {
        ContextHubTransaction<Void> txn = manager.unloadNanoApp(info, nanoAppId);
        ContextHubTransaction.Response<Void> resp = null;
        try {
            resp = txn.waitForResponse(TIMEOUT_UNLOAD_NANOAPP_SECONDS, TimeUnit.SECONDS);
        } catch (TimeoutException | InterruptedException e) {
            Assert.fail(e.getMessage());
        }

        return resp != null && resp.getResult() == ContextHubTransaction.RESULT_SUCCESS;
    }
    /**
     * Same as unloadNanoApp(), but asserts that it succeeds.
     */
    public static void unloadNanoAppAssertSuccess(
            ContextHubManager manager, ContextHubInfo info, long nanoAppId) {
        if (!unloadNanoApp(manager, info, nanoAppId)) {
            Assert.fail("Failed to unload nanoapp");
        }
    }

    /**
     * Executes a given shell command.
     *
     * @param instrumentation The instrumentation to use.
     * @param command         The shell command to execute.
     * @return The string output.
     */
    public static String executeShellCommand(Instrumentation instrumentation, String command) {
        final ParcelFileDescriptor pfd = instrumentation.getUiAutomation()
                .executeShellCommand(command);
        try (InputStream in = new FileInputStream(pfd.getFileDescriptor())) {
            return readFromInputStream(in);
        } catch (Exception e) {
            Assert.fail(e.getMessage());
        } finally {
            closeOrAssert(pfd);
        }
        return null;
    }

    /**
     * Executes a given shell command using the app context rather than the shells so that the app's
     * permissions are used.
     *
     * @param command         The shell command to execute.
     * @return The string output.
     */
    public static String executeShellCommandWithAppPerms(String command) throws Exception {
        final Process process = Runtime.getRuntime().exec(command);
        process.waitFor();
        return readFromInputStream(process.getInputStream());
    }

    /**
     * @param input The string input of an integer.
     * @return The converted integer.
     */
    public static int convertToIntegerOrFail(String input) {
        try {
            return Integer.parseInt(input);
        } catch (NumberFormatException e) {
            Assert.fail(e.getMessage());
        }

        return -1;
    }

    /**
     * @param input The string input of an integer.
     * @return The converted integer.
     */
    public static int convertToIntegerOrReturnZero(String input) {
        try {
            return Integer.parseInt(input);
        } catch (NumberFormatException e) {
            return 0;
        }
    }

    /**
     * Get all the nanoapps currently loaded on device.
     *
     * @return The nanoapps loaded currently.
     */
    public static List<NanoAppState> queryNanoAppsAssertSuccess(
            ContextHubManager contextHubManager, ContextHubInfo contextHubInfo) {
        ContextHubTransaction<List<NanoAppState>> transaction =
                contextHubManager.queryNanoApps(contextHubInfo);
        assertTransactionSuccessSync(transaction, QUERY_NANOAPPS_TIMEOUT_SECONDS);
        ContextHubTransaction.Response<List<NanoAppState>> response = null;
        try {
            response = transaction.waitForResponse(QUERY_NANOAPPS_TIMEOUT_SECONDS,
                    TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            Assert.fail("InterruptedException while getting query response");
        } catch (TimeoutException e) {
            Assert.fail("TimeoutException while getting query response");
        }
        return response.getContents();
    }

    /**
     * Queries for the nanoapp version.
     *
     * @param contextHubManager The ContextHubManager to use.
     * @param contextHubInfo    The ContextHubInfo describing the Context Hub to query.
     * @param nanoAppId         The ID of the nanoapp to get the version for.
     * @return The nanoapp version.
     */
    public static int getNanoAppVersion(ContextHubManager contextHubManager,
            ContextHubInfo contextHubInfo, long nanoAppId) {
        List<NanoAppState> stateList = queryNanoAppsAssertSuccess(contextHubManager,
                contextHubInfo);
        for (NanoAppState state : stateList) {
            if (state.getNanoAppId() == nanoAppId) {
                return (int) state.getNanoAppVersion();
            }
        }

        Assert.fail("Could not query for nanoapp with ID 0x" + Long.toHexString(nanoAppId));
        return -1;
    }

    /**
     * @param closeable The object to close.
     */
    private static void closeOrAssert(AutoCloseable closeable) {
        try {
            closeable.close();
        } catch (Exception e) {
            Assert.fail(e.getMessage());
        }
    }

    private static String readFromInputStream(InputStream in) throws Exception {
        StringBuilder out = new StringBuilder();
        BufferedReader br = new BufferedReader(new InputStreamReader(in,
                StandardCharsets.UTF_8));
        String str = null;
        while ((str = br.readLine()) != null) {
            out.append(str);
        }

        closeOrAssert(br);
        return out.toString();
    }

    /**
     * Assert that the context hub transaction gets a successful response.
     *
     * @param transaction      The context hub transaction
     * @param timeoutInSeconds The timeout while waiting for the transaction response in seconds
     */
    private static void assertTransactionSuccessSync(
            ContextHubTransaction<?> transaction, long timeoutInSeconds) throws AssertionError {
        if (transaction == null) {
            Assert.fail("ContextHubTransaction cannot be null");
        }

        String type = ContextHubTransaction.typeToString(transaction.getType(),
                true /* upperCase */);
        ContextHubTransaction.Response<?> response = null;
        try {
            response = transaction.waitForResponse(timeoutInSeconds, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            Assert.fail("InterruptedException while waiting for " + type + " transaction");
        } catch (TimeoutException e) {
            Assert.fail("TimeoutException while waiting for " + type + " transaction");
        }

        Assert.assertTrue(type + " transaction failed with error code " + response.getResult(),
                response.getResult() == ContextHubTransaction.RESULT_SUCCESS);
    }

    public static void assertLatchCountedDown(CountDownLatch latch, long timeoutThreshold)
            throws InterruptedException {
        boolean isCountedDown = latch.await(timeoutThreshold, TimeUnit.SECONDS);
        assertWithMessage(
                        "Waiting for latch to count down timeout after %s seconds",
                        timeoutThreshold)
                .that(isCountedDown)
                .isTrue();
    }
}
