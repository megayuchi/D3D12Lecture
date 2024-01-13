## 요구사항
Visual Studio 2022 , Windows 10/11

## 빌드 방법
1. .sln파일 폴더를 엽니다.
2. d3d12 core 바이너리 파일들을 업데이트 합니다. 아래 d3d12 업데이트 방법을 참고하세요.
3. 빌드하고 실행합니다.

## D3D12 Agility SDK 업데이트
이 프로젝트는 D3D12 Agility SDK를 사용합니다.  (https://devblogs.microsoft.com/directx/directx12agility/) 

Agility SDK는 NuGet패키지로서 업데이트 됩니다.
이 프로젝트는 직접적으로 D3D12 Core 바이너리를 포함하지 않습니다. 사용자가 직접 업데이트해야 합니다

1. solution explorer의 solution에서 오른쪽 버튼을 누릅니다.
2. 'Restore NuGet Packages'을 클릭합니다.
3. 해당 프로젝트 폴더의 packages\Microsoft.Direct3D.D3D12.xxxxx\build\native\bin의 arm,arm64,win32,x64 폴더들을 프로젝트 폴더의 D3D12폴더로 카피합니다.


## Requirements
Visual Studio 2022 , Windows 10/11

## How to build
1. Open .sln in Visual Studio.
2. update d3d12 core binaries. Please refer to the d3d12 sdk update method below.
3. build and run.
      
## Update D3D12 Agility SDK
This project uses the D3D12 Agility SDK. (https://devblogs.microsoft.com/directx/directx12agility/)

Agility SDK is updated as a NuGet package.
This project does not directly contain the package's binaries. Users must update themselves

1. Right-click on top solution in solution explorer.
2. Click ‘Restore NuGet Packages’.
3. Copy the arm, arm64, win32, x64 folders from '[Project Folder]\packages\Microsoft.Direct3D.D3D12.xxxxx\build\native\bin' to the D3D12 folder in the project folder.
   
