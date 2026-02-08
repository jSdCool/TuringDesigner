# Turing Designer
Easily Design turing machines in a graphical environment

---

## Controls
Arrow keys - Move machine  
Brackets /mouse forward and back buttons [windows only] - Change mode

## Modes
Each mode is represented by the buttons across the top of the screen  
The following modes exist:
- PAN
  - Click and drag with the mouse to move around
- NEW STATE
  - Click to place new states
- NEW TRANSITION
  - Click on a state to start creating a transition
  - Click on empty space to set a control point
  - (optionally) Click on another empty space to set a second control point
  - Click on another state to set the end of the transition
  - Enter the transition match string
  - Enter the transition wright letter
  - Select Move right or left
- MOVE STATE
  - Click and drag states to move them
- MOVE TRANSITION
  - Click on the text of a transition to select it
  - Click and drag the visible control points to move them around
- EDIT TRANSITION
  - Click on the text of a transition to edit it
- DELETE STATE
  - Click on a state to delete it as well as any transitions going to or from that state
- DELETE TRANSITION
  - Click on the text of a transition to delete the transition 
- SET START STATE
  - Click on a state to set it as the start state
- NEW HALT TRANSITION
  - Click on a state to start a new halt transition
  - Click on empty space to set the control point for the halt transition
  - Fill in the transtion details

## Compile details
Setup with cmake like you would any project for your platform 

### Linux specific: you will need the following packages installed: 
Ubuntu/debien
```shell
sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
```

Fedora
```shell
sudo dnf install alsa-lib-devel mesa-libGL-devel libX11-devel libXrandr-devel libXi-devel libXcursor-devel libXinerama-devel libatomic
```

Asahi Remix (Apple Silicon)
```shell
sudo dnf install libX11-devel libXrandr-devel libXi-devel libXcursor-devel mesa-libGL-devel pulseaudio-libs-devel libdrm-devel libXinerama-devel
```

Arch Linux
```shell
sudo pacman -S alsa-lib mesa libx11 libxrandr libxi libxcursor libxinerama
```

SteamOS (Native - Arch Linux)  
Add the correct mirror first  
`sudo nano /etc/pacman.d/mirrorlist`  
`Server = htpps://steamdeck-packages.steamos.cloud/archlinux-mirror/$repo/os/$arch`
```shell
sudo pacman -S base-devel glibc linux-api-header alsa-lib mesa xorgproto libx11 libxrandr libxi libxcursor libxinerama libxext libxrender libxfixes
```

Void Linux
```shell
sudo xbps-install make alsa-lib-devel libglvnd-devel libX11-devel libXrandr-devel libXi-devel libXcursor-devel libXinerama-devel mesa MesaLib-devel
```
And with intel drivers:
```shell
sudo xbps-install mesa-dri mesa-intel-dri
```