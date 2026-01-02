# FastSignalProcessor

High-performance DSP (Digital Signal Processing) engine. This project implements a hybrid architecture using **Java** for orchestration and **C (JNI)** for low-level memory manipulation.

## Technical Highlights
* **Critical Array Access:** Uses JNI's `GetPrimitiveArrayCritical` to minimize JVM overhead.
* **Architecture:** Decoupled logic with native C shared libraries.
* **Performance:** Optimized for high-throughput data filtering.

## Build Requirements
* JDK 17+
* GCC / Clang

## By Astra