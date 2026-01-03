package com.processor;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.security.SecureRandom;

public final class NativeEngine {

    static {
        try {
            System.loadLibrary("native_engine");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Critical: Native library 'libnative_engine' not found in java.library.path");
            System.exit(1);
        }
    }

    public native void applyGainFilterDirect(ByteBuffer data, float gain, int iterations);

    public void executeHighPerformanceCompute() {
        final int bufferSize = 64 * 1024 * 1024;
        
        ByteBuffer signalData = ByteBuffer.allocateDirect(bufferSize).order(ByteOrder.nativeOrder());
        
        byte[] entropy = new byte[1024];
        new SecureRandom().nextBytes(entropy);
        for (int i = 0; i < bufferSize; i++) {
            signalData.put(i, entropy[i % entropy.length]);
        }

        System.out.println("Status: Buffer Allocated (" + (bufferSize >> 20) + " MB)");

        final long startTime = System.nanoTime();

        applyGainFilterDirect(signalData, 1.5f, 20);

        final long endTime = System.nanoTime();
        final double durationMs = (endTime - startTime) / 1_000_000.0;
        final double throughput = (bufferSize / (1024.0 * 1024.0)) / (durationMs / 1000.0);

        System.out.printf("Execution: %.2f ms | Throughput: %.2f MB/s%n", durationMs, throughput);
    }

    public static void main(String[] args) {
        new NativeEngine().executeHighPerformanceCompute();
    }
}
