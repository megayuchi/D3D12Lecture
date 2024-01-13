## Requirements
Visual Studio 2022 , Windows 10/11

## How to build
1. Open .sln in Visual Studio.
2. update d3d12 core binaries.
3. build and run.
      
## Update D3D12 Agility SDK
This project uses the D3D12 Agility SDK. (https://devblogs.microsoft.com/directx/directx12agility/)
Agility SDK is updated as a NuGet package.
This project does not directly contain the package's binaries. Users must update themselves

1. Right-click on top solution in solution explorer.
2. Click ‘Restore NuGet Packages’.
3. Copy the arm, arm64, win32, x64 folders from '[Project Folder]\packages\Microsoft.Direct3D.D3D12.xxxxx\build\native\bin' to the D3D12 folder in the project folder.
   
