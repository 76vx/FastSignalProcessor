package com.processor;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.concurrent.ThreadLocalRandom;
import java.util.stream.IntStream;

public final class NativeEngine {

    static {
        System.loadLibrary("native_engine");
    }

    public native void applyGainFilterDirect(ByteBuffer data, float gain, int iterations);

    public void executeHighPerformanceCompute() {
        final int bufferSizeMB = 256;
        final int bufferSize = bufferSizeMB * 1024 * 1024;
        final int alignmentBytes = 64;

        System.out.println("Allocating buffer of size: " + bufferSizeMB + " MB with alignment: " + alignmentBytes + " bytes");

        ByteBuffer rawBuffer = ByteBuffer.allocateDirect(bufferSize + alignmentBytes)
                                         .order(ByteOrder.nativeOrder());

        long rawAddress = ((sun.nio.ch.DirectBuffer) rawBuffer).address();
        int offset = (int) (alignmentBytes - (rawAddress % alignmentBytes));
        rawBuffer.position(offset);
        ByteBuffer alignedBuffer = rawBuffer.slice().order(ByteOrder.nativeOrder());

        System.out.println("Buffer aligned at offset: " + offset);

        byte[] entropy = new byte[1024 * 1024];
        ThreadLocalRandom.current().nextBytes(entropy);
        System.out.println("Entropy generated: " + entropy.length + " bytes");

        int cores = Runtime.getRuntime().availableProcessors();
        System.out.println("Available cores: " + cores);

        IntStream.range(0, cores).parallel().forEach(core -> {
            int chunkSize = bufferSize / cores;
            int start = core * chunkSize;
            ByteBuffer slice = alignedBuffer.duplicate();
            slice.position(start);
            slice.put(entropy, 0, Math.min(entropy.length, chunkSize));
        });

        System.out.println("Buffer populated with random data");

        for (int i = 0; i < 5; i++) {
            applyGainFilterDirect(alignedBuffer, 1.1f, 1);
        }

        long startTime = System.nanoTime();
        applyGainFilterDirect(alignedBuffer, 1.5f, 20);
        long endTime = System.nanoTime();

        double durationS = (endTime - startTime) / 1_000_000_000.0;
        double throughputGBs = (bufferSize / (1024.0 * 1024.0 * 1024.0)) / durationS;

        System.out.printf("Execution finished: %.4f s | Throughput: %.2f GB/s%n", durationS, throughputGBs);
    }

    public static void main(String[] args) {
        System.out.println("Starting NativeEngine benchmark...");
        NativeEngine engine = new NativeEngine();
        engine.executeHighPerformanceCompute();
        System.out.println("NativeEngine benchmark completed successfully.");
    }
}
