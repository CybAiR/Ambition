# ICD: Moduł Wiertła i Ładunku (STM32 <-> ROS 2)

- **Status:** `DRAFT v0.1` (wymaga akceptacji i zamrożenia z Sekcją Wiertła/Elektroniki przed startem implementacji finalnej)
- **Właściciel (Software):** Wookie
- **Interfejs sprzętowy (Hardware):** Sekcja Wiertła / Elektronika (STM32)
- **Kontekst ERC:** Deep/Surface Sampling — wymóg odwiertu >= 300 mm oraz pokładowego pomiaru masy próbki (> 100 g).

---

## 1. Parametry globalne magistrali CAN (Warstwa fizyczna i łącza)

| Parametr | Wymagana wartość | Uwagi |
| :--- | :--- | :--- |
| Standard | CAN 2.0A (Classic CAN) | Max 8 bajtów payloadu na ramkę (DLC 0–8) |
| Prędkość (Bitrate) | **500 kbps** | Standard dla całego łazika |
| Format ID | **11-bit (Standard ID)** | Niższe ID = wyższy priorytet arbitrażu |
| Kolejność bajtów | **Little-Endian** | Natywne dla STM32 i PC (x86/ARM) |
| Terminacja | 120 Ω na obu końcach magistrali | Po stronie hardware |
| Watchdog (MCU) | **500 ms** | Brak ramki sterującej / heartbeat z PC przez 500 ms -> natychmiastowy STOP napędów wiertła |
| Interfejs na PC | Natywny **SocketCAN** (`can0` / `vcan0`) | Adapter USB-CAN z `gs_usb` (np. CANable / PCAN-USB); bez konwersji UART-CAN po stronie PC |

### Rezerwacja puli CAN ID dla całego łazika
- `0x000–0x0FF` — System globalny, E-Stop, Heartbeat
- `0x100–0x1FF` — Jazda / Napęd
- **`0x200–0x2FF` — Wiertło i Ładunek (ten dokument)**
- `0x300–0x3FF` — Ramię robotyczne

---

## 2. Tabela ramek CAN (Kontrakt do uzgodnienia z Hardware)

*Uwaga: konkretne ID i mapowanie bajtów z zakresu `0x200–0x2FF` należy uzgodnić z sekcją Hardware i zaktualizować w tym pliku przed zmianą statusu na `FROZEN`.*

| CAN ID (hex) | Priorytet | Kierunek | Nazwa ramki | DLC | Układ bajtów (Payload) |
| :--- | :---: | :--- | :--- | :---: | :--- |
| `0x001` | Krytyczny | PC -> ALL | `GLOBAL_ESTOP` | 1 | `B0`: `0x01` = natychmiastowe zatrzymanie awaryjne |
| `0x201` | Wysoki | PC -> STM32 | `DRILL_CMD` | 6 | `B0`: komenda/akcja (`0`=IDLE, `1`=MANUAL, `2`=AUTO_DRILL, `3`=RETRACT, `4`=TARE_SCALE)<br>`B1`: ID żądania (`uint8`, `seq_id` do parowania z potwierdzeniem)<br>`B2–B3`: zadana prędkość obrotowa (`int16`, RPM)<br>`B4–B5`: zadany posuw liniowy (`int16`, mm/s) |
| `0x210` | Średni | STM32 -> PC | `DRILL_STATUS` | 8 | `B0`: stan wykonania (`0`=IDLE, `1`=IN_PROGRESS, `2`=DONE, `3`=ERROR)<br>`B1`: `seq_id` ostatniej odebranej komendy (`uint8`)<br>`B2`: kod błędu (`0`=OK, `1`=TIMEOUT, `2`=OVERCURRENT, `3`=LIMIT_HIT, `4`=ESTOP)<br>`B3`: krańcówki (`bit0`=góra, `bit1`=dół)<br>`B4–B5`: aktualna głębokość / pozycja (`int16`, mm)<br>`B6–B7`: prąd silnika (`uint16`, mA) |
| `0x211` | Niski | STM32 -> PC | `PAYLOAD_SENSORS` | 6 | `B0–B3`: masa próbki z HX711 (`float32`, g)<br>`B4–B5`: temperatura silnika (`int16`, 0.1 °C) |

### Wymóg potwierdzenia wykonania (Handshake)
1. Każda akcja z PC posiada licznik `seq_id`.
2. Podczas wykonywania akcji STM32 okresowo wysyła `DRILL_STATUS` ze stanem `IN_PROGRESS`.
3. Po zakończeniu STM32 wysyła `DONE` **albo** `ERROR` (z niezerowym kodem błędu).
4. PC **nie może** wisieć w nieskończoność w oczekiwaniu na `DONE` — obowiązuje konfigurowalny timeout po stronie ROS 2.

---

## 3. Kontrakt po stronie ROS 2 (Architektura Black-Box)

- **Docelowy katalog w repo:** `software/drill/`
- **Nazwa węzła:** `drill_driver_node`
- **Parametry konfiguracyjne (YAML / env — zakaz wpisywania na sztywno w kodzie):**
  - `can_interface` (domyślnie `can0`, do testów `vcan0`)
  - `action_timeout_sec`
  - `log_file_path`

### Interfejs ROS 2
- **Publikowane topiki (do użycia przez automat misji i GUI):**
  - `/drill/state` — aktualny stan, kod błędu, krańcówki, prąd
  - `/drill/depth` — aktualna głębokość odwiertu [m lub mm — spójnie w całym pakiecie]
  - `/drill/weight` — odczyt wagi próbki z tensometru [g]
- **Sterowanie:**
  - Subskrypcja `/estop` (natychmiastowa wysyłka `0x001` na CAN i przerwanie aktywnej akcji).
  - **ROS 2 Action** `/drill/execute` (lub serwis + stan dla prostych komend) obsługujący sekwencje z feedbackiem (`IN_PROGRESS`), wynikiem końcowym (`DONE`/`ERROR`) oraz bezpiecznym przerwaniem (`CANCEL`).
- **Logowanie na PC:**
  - Zapis wszystkich komend, zmian stanu, błędów i timeoutów do pliku w formacie: `[YYYY-MM-DD HH:MM:SS.mmm] [EVENT] <szczegóły>`.

---

## 4. Wymagania wdrożeniowe i kryteria odbioru (DoD)

1. **Konteneryzacja:** Moduł musi działać jako **osobny kontener Dockerowy** (własny `Dockerfile` w `software/drill/`, uruchamiany z `network_mode: host`), bez instalowania czegokolwiek bezpośrednio na hoście.
2. **Testowalność bez sprzętu:** Dołączony skrypt/instrukcja testu na wirtualnym interfejsie `vcan0` (`python-can` lub `can-utils`) symulujący:
   - poprawne wykonanie akcji (`IN_PROGRESS` -> `DONE`),
   - zgłoszenie błędu przez STM32 (`ERROR`),
   - brak odpowiedzi z STM32 (zadziałanie timeoutu po stronie PC),
   - wywołanie `E-Stop`.
3. **Jakość kodu:** Kod i komentarze spójne, brak zahardkodowanych ścieżek lokalnych, czytelny `README.md` z dokładną komendą `docker build` i `docker run`.