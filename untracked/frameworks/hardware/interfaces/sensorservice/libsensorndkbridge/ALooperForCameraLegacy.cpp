/*
 * Copyright (C) 2026 LineageOS
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

// ALooper_*_forCamera() uyumluluk katmani.
//
// Neden gerekli?
//   Oreo (API 27) donemi kamera sensor dongusu bu uc C API'sini kullaniyordu:
//     vendor/lib/libsensorlistener.so (Samsung, Oreo)
//       -> ALooper_forCamera / ALooper_pollOnce_camera / ALooper_release_forCamera
//   Bu semboller AOSP'nin "kamera-ozel ALooper" API'siydi ve Oreo'daki
//   libsystem_core icinde bulunuyordu. Android 10'da API tamamen kaldirildi;
//   dolayisiyla agacin hicbir kutuphanesi (libcutils dahil) bu sembolleri
//   vermiyor. Sonuc olarak Oreo kamera HAL'i vendor'da yuklenemiyordu:
//
//     E/vndksupport: Could not load
//       /vendor/lib/hw/camera.universal7870_go.so from default namespace:
//       dlopen failed: cannot locate symbol "ALooper_forCamera"
//       referenced by "/vendor/lib/libsensorlistener.so"
//     E/CamPrvdr@2.4-legacy: Could not load camera HAL module: -22
//
//   Bu dosya o uc fonksiyonu, Android 10'un herkese acik ALooper API'si
//   uzerinden yeniden tanimlar. Boylece libsensorlistener.so'nun DT_NEEDED
//   zinciri cozulur ve kamera HAL'i vendor namespace'inde yuklenebilir.
//
// Imzalar cagridan (libsensorlistener.so disassembly'i) ve stok Oreo
// libsensorndkbridge.so govdesinden birebir tespit edildi:
//   ALooper* ALooper_forCamera(ALooper* /*looper*/)              -> r0 verilir, sonuc saklanir
//   int      ALooper_pollOnce_camera(ALooper*, int timeout,      -> r0=looper, r1=timeout(300),
//                                    int* fd, int* ev, void** d)      r2=fd, r3=events, [sp]=data
//   void     ALooper_release_forCamera(ALooper*)                -> r0=looper
//
// NOT: Oreo'daki ALooper::pollOnce() uye fonksiyondu (this + 4 arg).
// Android 10'da static'tir; bu yuzden ALooper_pollOnce() serbest
// fonksiyonuna cagriliyor.

#include <android/looper.h>

extern "C" {

// Oreo: mutex al, her cagrida yeni bir ALooper olustur.
// A10'da ALoper opak bir tip; ALooper_prepare(0) cagirilan thread'e ait
// looperi dondurur, yoksa olusturur. Oreo'daki "her seferinde yeni"
// davranisi zaten anlamsizdi, cunku ALooper::pollOnce() daima thread'in
// kendi looperini kullanir.
ALooper* ALooper_forCamera(ALooper* /*looper*/) {
    return ALooper_prepare(0);
}

// Oreo: looper->pollOnce(timeoutMillis, outFd, outEvents, outData)
int ALooper_pollOnce_camera(ALooper* /*looper*/, int timeoutMillis,
                            int* outFd, int* outEvents, void** outData) {
    return ALooper_pollOnce(timeoutMillis, outFd, outEvents, outData);
}

// Oreo: ALooper'i mutex altinda yikip delete ediyordu.
// Buradaki ALooper_prepare() cagrisi thread'e ait olmayan yeni bir nesne
// degil, thread'in kendi looperi; dolayisiyla delete etmek yanlis olur.
// Genel API'de yikma islemi yok (yalnizca ALooper_acquire/release var ve
// release, bizim almadigimiz bir referansi dusurur). Bu yuzden bilerek
// no-op: SensorListener dongusu baslayip bir kez kapanir, sizinti ihmal
// edilebilir ve yanlis bir delete ile kullanimda-kullanilmayan-heap
// hatasi riskine girilmez.
void ALooper_release_forCamera(ALooper* /*looper*/) {}

} // extern "C"