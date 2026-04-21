import cv2
import numpy as np
import os


def segment_sky_ground(img, window_size=11, var_thresh=25.0):
    img_f = img.astype(np.float32)
    blur = cv2.blur(img_f, (window_size, window_size))
    blur_sq = blur ** 2
    sq_blur = cv2.blur(img_f ** 2, (window_size, window_size))
    variance = sq_blur - blur_sq

    h, w = img.shape
    high_var = variance >= var_thresh

    first_high_var_idx = np.argmax(high_var, axis=0)
    first_high_var_idx[~high_var.any(axis=0)] = h

    Y, X = np.indices((h, w))
    sky_mask = (Y < first_high_var_idx[X]).astype(np.uint8) * 255

    kernel = np.ones((5, 5), np.uint8)
    sky_mask = cv2.morphologyEx(sky_mask, cv2.MORPH_OPEN, kernel)
    sky_mask = cv2.morphologyEx(sky_mask, cv2.MORPH_CLOSE, kernel)
    return sky_mask


def find_endpoints(skel):
    skel_bin = (skel > 0).astype(np.uint8)
    kernel = np.array([[1, 1, 1], [1, 10, 1], [1, 1, 1]], dtype=np.uint8)
    filtered = cv2.filter2D(skel_bin, -1, kernel)
    y, x = np.where(filtered == 11)
    return np.column_stack((x, y))


def fill_gaps(edges, gap_threshold=15):
    linked_edges = edges.copy()
    endpoints = find_endpoints(edges)

    if len(endpoints) == 0:
        return linked_edges

    diff = endpoints[:, np.newaxis, :] - endpoints[np.newaxis, :, :]
    dist_sq = np.sum(diff ** 2, axis=-1).astype(float)
    np.fill_diagonal(dist_sq, np.inf)

    min_dist_idx = np.argmin(dist_sq, axis=1)
    min_dist_sq = np.min(dist_sq, axis=1)

    valid_pairs = min_dist_sq <= (gap_threshold ** 2)

    for i in range(len(endpoints)):
        if valid_pairs[i]:
            pt1 = tuple(endpoints[i])
            pt2 = tuple(endpoints[min_dist_idx[i]])
            cv2.line(linked_edges, pt1, pt2, 255, 1)

    return linked_edges


def debug_mars_image(image_path, sigma=3.0, low_thresh=30, high_thresh=80, gap_size=20, var_thresh=25.0):
    img = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)
    if img is None:
        return

    sky_mask = segment_sky_ground(img, window_size=11, var_thresh=var_thresh)

    clahe = cv2.createCLAHE(clipLimit=1.5, tileGridSize=(8, 8))
    img_clahe = clahe.apply(img)

    blurred = cv2.GaussianBlur(img_clahe, (0, 0), sigmaX=sigma, sigmaY=sigma)
    edges = cv2.Canny(blurred, low_thresh, high_thresh)
    edges[sky_mask == 255] = 0

    linked_edges = fill_gaps(edges, gap_threshold=gap_size)

    h, w = linked_edges.shape
    padded_edges = cv2.copyMakeBorder(linked_edges, 1, 1, 1, 1, cv2.BORDER_CONSTANT, value=0)
    flood_mask = np.zeros((h + 4, w + 4), np.uint8)

    flooded = padded_edges.copy()
    cv2.floodFill(flooded, flood_mask, (0, 0), 255)

    flooded_inv = cv2.bitwise_not(flooded)

    filled_rocks_padded = padded_edges | flooded_inv
    filled_rocks = filled_rocks_padded[1:h + 1, 1:w + 1]

    kernel_open = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (5, 5))
    cleaned_rocks = cv2.morphologyEx(filled_rocks, cv2.MORPH_OPEN, kernel_open)

    contours, _ = cv2.findContours(cleaned_rocks, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)

    result_img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)

    valid_targets = []
    for c in contours:
        if cv2.contourArea(c) > 300:
            x, y, w, h = cv2.boundingRect(c)
            valid_targets.append(((x, y, w, h), cv2.contourArea(c)))

    valid_targets.sort(key=lambda item: item[1], reverse=True)

    for i, target in enumerate(valid_targets):
        x, y, w, h = target[0]
        cv2.rectangle(result_img, (x, y), (x + w, y + h), (0, 255, 0), 2)

    os.makedirs('./results', exist_ok=True)
    img_bgr = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
    combined_img = np.hstack((img_bgr, result_img))
    cv2.imwrite('./results/resultsOpp.jpg', combined_img)

    s_img_clahe = cv2.resize(img_clahe, None, fx=0.5, fy=0.5, interpolation=cv2.INTER_AREA)
    s_edges = cv2.resize(edges, None, fx=0.5, fy=0.5, interpolation=cv2.INTER_AREA)
    s_closed_edges = cv2.resize(linked_edges, None, fx=0.5, fy=0.5, interpolation=cv2.INTER_AREA)
    s_result = cv2.resize(result_img, None, fx=0.5, fy=0.5, interpolation=cv2.INTER_AREA)

    cv2.imshow('1. Obraz po CLAHE', s_img_clahe)
    cv2.imshow('2. Krawedzie', s_edges)
    cv2.imshow('3. Domykanie luk', s_closed_edges)
    cv2.imshow('4. Final', s_result)

    cv2.waitKey(0)
    cv2.destroyAllWindows()


debug_mars_image('./img/02rock.png', sigma=1.5, low_thresh=30, high_thresh=60, gap_size=40, var_thresh=25.0)