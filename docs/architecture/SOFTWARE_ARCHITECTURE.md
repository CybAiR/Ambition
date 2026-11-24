# Architektura Oprogramowania Łazika Ambition (ROS 2 + Docker)

## 1. Koncepcja nadrzędna (Black-Box Architecture)

- System oprogramowania pokładowego opiera się na **ROS 2** oraz pełnej konteneryzacji w **Dockerze** (`1 podsystem = 1 niezależny kontener`).
- Wszystkie kontenery na komputerze pokładowym uruchamiane są z `network_mode: host`, co zapewnia natywną komunikację DDS między kontenerami oraz dostęp do interfejsu SocketCAN (`can0`) bez użycia flagi `--privileged`.
- Interfejs `can0` (500 kbps) podnoszony jest wyłącznie po stronie systemu operacyjnego hosta (usługa systemowa przy starcie).
- **Decyzja bazowa (do zamrożenia przed startem T2):** docelowa dystrybucja ROS 2 (`Jazzy` **XOR** `Humble`) zostanie przypięta w bazowym `Dockerfile` na podstawie wyniku kompilacji przypiętego sterownika napędów `candle_ros2` (`08835c6f15944f64b6f7efabc7a87833d09be6bc` z `origin/devel`).

## 2. Schemat warstwowy systemu

```text
STACJA OPERATORA (osobny host / compose)
  [ Ambition_GUI (SDL2/ImGui) ] <---(ROS 2 Adapter)---> [ E-Stop / Kontroler ]
  [ Comms Station (Wideo RTP) ] <====(UDP / 5 GHz)====+
                                                      |
============================= ŁĄCZE RADIOWE ==========|======================
                                                      v
KOMPUTER POKŁADOWY ŁAZIKA (Linux + Docker, network_mode: host)
  WARSTWA 3 (Faza 3): [ ctr_mission: Nav2 / BT.CPP ]  [ ctr_logger: ros2 bag -> VPS-2 ]
                                | (Actions: /navigate, /drill/execute)
  WARSTWA 2 (Faza 2): [ ctr_localization: EKF ]       [ ctr_vision: ArUco 5x5 + QR + Skały ]
                                | (/cmd_vel_nav, /aruco_poses, /odom)
  WARSTWA 1 (Faza 1): [ ctr_core: URDF/TF + twist_mux + E-Stop ]
                      [ ctr_drive: candle_ros2 + motor_control + odometry ]
                      [ ctr_drill: drill_driver_node (SocketCAN -> /drill/*, /weight) ]
                      [ ctr_comms_rover: V4L2 -> H.264/RTP (bez zależności GUI) ]
                      [ ctr_arm_interface: kontrakt ROS 2 z sekcją Robotic Arm ]
```

## 3. Cztery twarde zasady architektoniczne

1. **Jeden pakiet kontraktów (`ambition_interfaces`):** wszystkie własne definicje `.msg`, `.srv` i `.action` znajdują się w jednym pakiecie; wiadomości percepcji muszą posiadać `std_msgs/Header` (znacznik czasu i ramkę odniesienia).
2. **Jeden arbiter ruchu (`twist_mux`):** wyłączny dostęp do komend napędu z twardą hierarchią priorytetów: `1. /estop_lock` > `2. /cmd_vel_teleop` (z obowiązkowym przyciskiem *dead-man*) > `3. /cmd_vel_nav`.
3. **Jedno źródło prawdy o geometrii:** promień koła, rozstaw kół oraz mapowanie ID napędów MD80 (`509–512`) zdefiniowane w jednym pliku YAML współdzielonym przez napęd i odometrię (eliminacja rozbieżności parametrów i znaków między `ambition_control` a `joint_state_odometry`).
4. **Higiena repozytorium i testy bez sprzętu:** zakaz commitowania artefaktów builda (`Debug/`), cache'ów, crashdumpów, haseł i ścieżek absolutnych `/home/...`; każdy moduł musi posiadać procedurę testu bez fizycznego łazika (`vcan0`, mocki, symulacja lub `ros2 bag`).

## 4. Granice odpowiedzialności sekcji Software

- **W zakresie:** architektura ROS 2/Docker, sterowniki wysokopoziomowe, bezpieczeństwo jazdy (`twist_mux`, watchdog, E-Stop), lokalizacja/odometria, percepcja (ArUco, QR, kamery), adapter ROS 2 dla GUI/Comms, rejestracja danych.
- **Poza zakresem:** firmware niskopoziomowy STM32 (dostarczamy wyłącznie wspólny kontrakt CAN/ICD), kinematyka i mechanika ramienia (dostarczamy kontener bazowy i kontrakt ROS 2), dron (wymóg regulaminowy ERC: MATLAB/Simulink pod opieką ONT).
