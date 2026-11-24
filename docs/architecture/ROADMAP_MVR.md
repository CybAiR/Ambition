# Roadmapa Sekcji Software: Strategia „Minimum Viable Rover”

## 1. Podział na 3 Fazy

- **Faza 1 — MVP Jeżdżąco-Wiercący (Priorytet P0):** zamrożony Docker ROS 2, naprawiony napęd z realnym E-Stopem i dead-manem, spójna odometria z jednego YAML oraz kontener wiertła/wagi na SocketCAN. Zapewnia bazę pod zadania punktowane na ERC: Surface/Deep Sampling (400 pkt), Exploration (300 pkt) i Probing (200 pkt).
- **Faza 2 — Percepcja ERC i Integracja Stacji (Priorytet P1):** detekcja markerów ArUco 5x5 (`150x150 mm`, ID `51–64`, środek na wysokości `417 mm` pod Traverse oraz tagów na panelu Maintenance), dekoder QR dla strefy Astro-Bio, narzędzie zapisu zdjęć przed/po pobraniu próbki, podpięcie realnego E-Stopu w `Ambition_GUI` i wydzielenie `Comms/rover` bez zależności SDL2/GLFW.
- **Faza 3 — Autonomia i Orkiestracja (Backlog P2 — otwierany dopiero po domknięciu Faz 1–2):** fuzja sensorów EKF (`robot_localization`), autonomiczny Traverse w `Nav2` (dla pełnych 100% punktów bez podglądu wideo i GNSS w stacji operatora) oraz warstwa misji (`BehaviorTree.CPP`).

## 2. Backlog zadań (GitHub Issues `T1–T7`)

| ID | Priorytet | Moduł | Zakres prac | Kryterium odbioru (DoD) |
| :--- | :---: | :--- | :--- | :--- |
| **T1** | `P0` | Wiertło i Waga (`software/drill`) | Zamrożenie `docs/icd/ICD_DRILL.md` (CAN 500 kbps, 11-bit, Little-Endian) + kontener `drill_driver_node` (SocketCAN, waga HX711, akcja z timeoutem, E-Stop, logi). Wymogi ERC: odwiert >= 300 mm, masa > 100 g. | Czysty `docker build` + 4 testy na `vcan0` (`DONE`, `ERROR`, timeout, E-Stop). |
| **T2** | `P0` | Napęd + Bezpieczeństwo (`ambition_control`) | Przeniesienie `motor_control_node` z `origin/devel`, naprawa instalacji YAML i nazwy węzła, usunięcie dereferencji null, ograniczone timeouty usług `candle_ros2`, dodanie `twist_mux` + E-Stop + `require_enable_button: true`. | Build w Dockerze; puszczenie dead-mana lub E-Stop zeruje komendy w < 200 ms. |
| **T3** | `P0` | Odometria (`joint_state_odometry`) | Wyciągnięcie pakietu z `origin/devel`, ujednolicenie geometrii i mapowania kół `509–512` z T2, walidacja długości wektora `velocity`, `header.stamp` i macierzy kowariancji. | Test na sztucznym `JointState`: 1 m naprzód i obrót 90° dają poprawny `/odom`; pusty wektor nie wywala węzła. |
| **T4** | `P1` | URDF + Symulacja (`ambition_description`) | Konsolidacja `ambition_description` / `ambition_simulation`, uzupełnienie `package.xml`, czytelne nazwy jointów kół (zamiast `8–11`), prosty świat Gazebo z `diff_drive` i kamerą. | Łazik jeździ w Gazebo po `/cmd_vel` i publikuje kompletne drzewo TF. |
| **T5** | `P1` | Markery ERC (`ambition_markers`) | Węzeł Python w Dockerze: detekcja ArUco `DICT_5X5_100` (ID `51–64`, `150x150 mm`) z estymacją pozycji 3D/TF + skaner kodów QR (Astro-Bio). | Wydrukowany marker przed kamerą zwraca odległość w RViz z błędem < 5 cm; QR publikuje treść na topik. |
| **T6** | `P1` | Narzędzia Misji (`ambition_tools`) | Serwis „Snapshot” zapisujący zdjęcia przed/po pobraniu próbki z timestampem + profil `ros2 bag record` wykluczający surowe strumienie wideo. | Wywołanie serwisu zapisuje plik w folderze sesji; `ros2 bag` działa w Dockerze. |
| **T7** | `P2` | Integracja GUI / Comms / Skały | Rozdzielenie builda `Comms/rover` od SDL2/GLFW, przeniesienie IP i `/dev/video*` do configu, podpięcie realnego E-Stopu w `Ambition_GUI`, naprawa `detekcja_kamienia` (synchronizacja, `cx`/`cy`, ścieżki wag). | Otwierane po domknięciu `T1–T3` i uzgodnieniu interfejsów z autorami GUI/Comms. |
