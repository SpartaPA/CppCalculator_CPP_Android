# CppCalculator_CPP_Android

Qt 6.11.1 + C++ + CMake로 만든 Android 계산기 프로젝트입니다.
VS Code와 명령줄 CMake를 기준으로 Android APK를 빌드하고 설치할 수 있습니다.

## 1. 개발 환경

- Qt 6.11.1
- Qt Android `android_arm64_v8a`
- Host Qt: `gcc_64`
- JDK 21
- Android SDK
- Android NDK 27.2.12479018
- CMake
- Ninja
- VS Code

주요 설치 경로 예시:

```text
~/Qt/6.11.1/gcc_64
~/Qt/6.11.1/android_arm64_v8a
~/Android/Sdk
~/Android/Sdk/ndk/27.2.12479018
```

## 2. 프로젝트 열기

```bash
cd ~/Downloads/CppCalculator_CPP_Android
code .
```

## 3. Android 빌드

기존 Android 빌드 디렉터리를 삭제하고 다시 구성할 때:

```bash
cd ~/Downloads/CppCalculator_CPP_Android
rm -rf build-android

~/Qt/6.11.1/android_arm64_v8a/bin/qt-cmake \
  -DANDROID_SDK_ROOT=$HOME/Android/Sdk \
  -DANDROID_NDK_ROOT=$HOME/Android/Sdk/ndk/27.2.12479018 \
  -DQT_HOST_PATH=$HOME/Qt/6.11.1/gcc_64 \
  -S . \
  -B build-android \
  -GNinja
```

빌드:

```bash
cmake --build build-android -j$(nproc)
```

APK 생성:

```bash
cmake --build build-android --target apk -j$(nproc)
```

## 4. 생성된 APK 위치

현재 빌드 결과는 다음 두 곳에 생성됩니다.

```text
build-android/android-build/CppCalculator.apk
build-android/android-build/build/outputs/apk/debug/android-build-debug.apk
```

일반적인 테스트 설치에는 다음 APK를 사용합니다.

```text
build-android/android-build/build/outputs/apk/debug/android-build-debug.apk
```

APK 파일을 프로젝트 루트에 복사해 보관하려면:

```bash
cp \
  build-android/android-build/build/outputs/apk/debug/android-build-debug.apk \
  CppCalculator-debug.apk
```

## 5. Android 기기 연결 확인

USB 디버깅이 켜진 실제 Android 휴대폰 또는 Android Emulator를 연결한 뒤:

```bash
adb devices
```

정상적으로 연결된 장치가 `device` 상태로 표시되면 설치할 수 있습니다. 여러 장치가 동시에 연결되어 있으면 `-s <serial>` 옵션으로 대상 장치를 지정합니다.

## 6. 설치된 앱의 패키지명 확인

패키지명을 확인하려면:

```bash
adb shell pm list packages | grep -i calculator
```

패키지명이 확인되면 다음과 같이 앱을 실행할 수 있습니다.

```bash
adb shell monkey -p <패키지명> 1
```

## 7. APK 재설치

소스 코드를 수정한 뒤 다시 APK를 빌드하고 설치할 때:

```bash
cmake --build build-android -j$(nproc)
cmake --build build-android --target apk -j$(nproc)

adb -s <device-serial> install -r \
  build-android/android-build/build/outputs/apk/debug/android-build-debug.apk
```

`-r` 옵션은 기존 앱을 유지하면서 APK를 업데이트할 때 사용합니다.

## 8. 문제 해결

### 여러 Android 기기가 연결되어 `adb install`이 실패하는 경우

반드시 대상 serial을 지정합니다.

```bash
adb devices
adb -s <device-serial> install -r <APK파일>
```

### ADB 연결이 안 되는 경우

```bash
adb kill-server
adb start-server
adb devices
```

휴대폰에 USB 디버깅 승인 창이 나타나면 허용합니다.

### APK 위치를 다시 찾는 경우

```bash
find . -type f -name "*.apk"
```

## 9. 권장 작업 순서

```text
소스 코드 수정
    ↓
CMake 빌드
    ↓
APK 생성
    ↓
adb devices
    ↓
adb -s <device> install -r <APK>
    ↓
Android 휴대폰 / Emulator에서 실행
```
