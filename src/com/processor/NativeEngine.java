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
        final int bufferSize = 256 * 1024 * 1024; 
        final int alignment = 64; 
        
        ByteBuffer signalData = ByteBuffer.allocateDirect(bufferSize + alignment)
                                          .order(ByteOrder.nativeOrder());
        
        long address = ((sun.nio.ch.DirectBuffer)signalData).address();
        int offset = (int) (alignment - (address % alignment));
        signalData.position(offset);
        ByteBuffer alignedBuffer = signalData.slice().order(ByteOrder.nativeOrder());

        byte[] entropy = new byte[1024 * 1024];
        ThreadLocalRandom.current().nextBytes(entropy);

        IntStream.range(0, Runtime.getRuntime().availableProcessors()).parallel().forEach(core -> {
            int chunk = bufferSize / Runtime.getRuntime().availableProcessors();
            int start = core * chunk;
            alignedBuffer.duplicate().position(start).put(entropy, 0, Math.min(entropy.length, chunk));
        });

        System.out.printf("Memory: %d MB | Alignment: %d bits%n", bufferSize >> 20, alignment * 8);

        for (int warmup = 0; warmup < 5; warmup++) {
            applyGainFilterDirect(alignedBuffer, 1.1f, 1);
        }

        final long startTime = System.nanoTime();

        applyGainFilterDirect(alignedBuffer, 1.5f, 20);

        final long endTime = System.nanoTime();
        
        final double durationS = (endTime - startTime) / 1_000_000_000.0;
        final double throughputGBs = (bufferSize / (1024.0 * 1024.0 * 1024.0)) / durationS;

        System.out.printf("Latency: %.4f ms | Throughput: %.2f GB/s%n", durationS * 1000, throughputGBs);
    }

    public static void main(String[] args) {
        new NativeEngine().executeHighPerformanceCompute();
    }
}