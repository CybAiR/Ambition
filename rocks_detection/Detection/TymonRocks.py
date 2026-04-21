import cv2
from ultralytics import YOLO

# --- Konfiguracja ---

# Załaduj swój wytrenowany model YOLOv8
# Upewnij się, że plik 'best.pt' znajduje się w tym samym folderze
# co skrypt lub podaj pełną ścieżkę do pliku.
model = YOLO('./depthai_model/best2.pt')

# Określ źródło wideo (0 to zazwyczaj domyślna kamera internetowa)
# Jeśli masz wiele kamer, spróbuj 1, 2 itd.
# Możesz też podać ścieżkę do pliku wideo, np. 'moj_film.mp4'
source = 0
cap = cv2.VideoCapture(source)

# Sprawdź, czy kamera została otwarta poprawnie
if not cap.isOpened():
    print(f"Błąd: Nie można otworzyć źródła wideo: {source}")
    exit()

# --- Główna pętla ---

while True:
    # Odczytaj klatkę z kamery
    ret, frame = cap.read()

    # 'ret' to flaga (True/False), czy odczyt się powiódł
    if not ret:
        print("Koniec strumienia wideo lub błąd odczytu.")
        break

    # Uruchom detekcję na klatce
    # Wynik 'results' to lista, zazwyczaj z jednym elementem dla pojedynczego obrazu
    results = model(frame)

    # Narysuj wyniki na klatce
    # Metoda .plot() automatycznie rysuje ramki (bounding boxes) i etykiety
    annotated_frame = results[0].plot()

    # Wyświetl klatkę z detekcją w oknie OpenCV
    cv2.imshow("YOLOv8 Detekcja na żywo", annotated_frame)

    # Czekaj na naciśnięcie klawisza 'q', aby zakończyć
    # cv2.waitKey(1) czeka 1ms na klawisz
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# --- Zakończenie ---

# Zwolnij zasoby kamery
cap.release()
# Zamknij wszystkie okna OpenCV
cv2.destroyAllWindows()