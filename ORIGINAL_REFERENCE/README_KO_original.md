# Cpp Calculator — C# Android

원본 HTML의 계산기 기능을 C#/.NET MAUI Android 앱으로 옮긴 프로젝트입니다.

## Windows + VS Code에서 APK 만들기

1. .NET 9 SDK 설치
2. Android SDK / JDK 준비
3. VS Code에서 이 폴더 열기
4. 터미널:
   dotnet workload install maui-android
   dotnet restore
   dotnet build -f net9.0-android
5. APK 생성:
   dotnet publish -f net9.0-android -c Release

빌드 후 bin/Release/net9.0-android/publish/ 아래 APK를 휴대폰으로 복사해 설치합니다.

## 잘못 설치된 기존 앱 제거

휴대폰에서:
설정 → 애플리케이션 → 앱 목록 → 잘못 설치된 계산기 앱 → 제거

또는 PC에서 ADB가 연결되어 있다면:
adb shell pm list packages | findstr /i calculator
adb uninstall <확인한 패키지명>

주의: 패키지명을 확인하기 전에는 adb uninstall을 실행하지 마세요.
