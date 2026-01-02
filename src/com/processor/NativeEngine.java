package com.processor;

import java.nio.ByteBuffer;
import java.util.Random;

public class NativeEngine {
    
    static {
        // El nombre debe coincidir con el archivo compilado (libnative_engine.so o .dll), es logica, no tiene nada dificil, si controlas lo entenderas
        System.loadLibrary("native_engine");
    }

    // Método nativo para procesamiento de señales complejas, esta linea fue a base de una busqueda en stack overflow 
    public native void applyGainFilter(byte[] data, float gain, int iterations);

    public static void main(String[] args) {
        NativeEngine engine = new NativeEngine();
        
        // esto es una simulacion de un buffer de 64MB xd
        int bufferSize = 64 * 1024 * 1024;
        byte[] signalData = new byte[bufferSize];
        new Random().nextBytes(signalData);

        System.out.println("--- System Ready: Starting High-Performance Compute ---");
        System.out.println("Buffer size: " + (bufferSize / 1024 / 1024) + " MB");

        // Comparativa de tiempo, recomendado para obtener un mayor control
        long startTime = System.nanoTime();
        
        // Llamada al motor en C: Procesamos la señal con una ganancia y 20 pasadas
        engine.applyGainFilter(signalData, 1.5f, 20);

        long endTime = System.nanoTime();
        double duration = (endTime - startTime) / 1_000_000.0;

        System.out.printf("Compute finished in: %.2f ms\n", duration);
        System.out.println("Throughput: " + String.format("%.2f", (bufferSize / (duration / 1000) / 1024 / 1024)) + " MB/s");
    }
}